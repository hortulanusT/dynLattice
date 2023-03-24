
#include "modules/ExplicitModule.h"
#include "utils/testing.h"

//=======================================================================
//   class ExplicitModule
//=======================================================================

JEM_DEFINE_CLASS(ExplicitModule);

const char *ExplicitModule::TYPE_NAME = "Explicit";
const char *ExplicitModule::STEP_COUNT = "stepCount";
const char *ExplicitModule::SO3_DOFS = "dofs_SO3";
const char *ExplicitModule::REPORT_ENERGY = "reportEnergy";
const char *ExplicitModule::LEN_SCALE = "lenScale";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

ExplicitModule::ExplicitModule

  (const String& name)
  : Super(name)

{
  dtime_ = 1.0;
  valid_ = false;
  report_energy_ = false;
  lenScale_ = 1e-3;
  SO3_dofs_.resize(0);
  rdofs_.resize(0, 0);
  prec_ = jive::solver::Solver::PRECISION;
  saftey_ = 0.9;
  decrFact_ = 0.8;
  incrFact_ = 1.2;
  order_ = 0;
}

ExplicitModule::~ExplicitModule()
{
}

//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------

Module::Status ExplicitModule::init

    (const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  using jem::util::connect;
  using jive::implict::PropNames;

  Properties myConf = conf.makeProps(myName_);
  Properties myProps = props.findProps(myName_);
  StringVector SO3_dof_names;
  Properties params;
  Properties sparams;
  Ref<AbstractMatrix> inertia;
  Ref<DiagMatrixObject> diagInertia;

  // whether to report the energy
  myProps.find(report_energy_, REPORT_ENERGY);
  myConf.set(REPORT_ENERGY, report_energy_);

  // get the length scale
  myProps.find(lenScale_, LEN_SCALE);
  myProps.set(LEN_SCALE, lenScale_);

  // returning an address of an object pointing to the Model

  model_ = Model::get(globdat, getContext());

  // returning an address of an object pointing to class DofSpace

  dofs_ = DofSpace::get(globdat, getContext());
  cons_ = Constraints::find(dofs_, globdat);

  // Invalidate the state of this module when the DofSpace changes.

  connect(dofs_->newSizeEvent, this, &Self::invalidate);
  connect(dofs_->newOrderEvent, this, &Self::invalidate);

  // get the SO3 dof names

  if (myProps.find(SO3_dof_names, SO3_DOFS))
  {
    JEM_PRECHECK(SO3_dof_names.size() == 3);
    SO3_dofs_.resize(SO3_dof_names.size());
    rdofs_.resize(SO3_dof_names.size(), dofs_->dofCount() / dofs_->typeCount());
    for (idx_t i = 0; i < SO3_dof_names.size(); i++)
    {
      IdxVector nodes;
      SO3_dofs_[i] = dofs_->getTypeIndex(SO3_dof_names[i]);
      dofs_->getDofsForType(IdxVector(rdofs_(i, ALL)), nodes, SO3_dofs_[i]);
    }
    myConf.set(SO3_DOFS, SO3_dof_names);

    String modelType;
    props.get(modelType, "model.type");
    if (modelType == "Matrix") {
      props.set("model.matrix2.type", FlexMatrixBuilder::getType()->getName());
      props.set("model.matrix2.constant", false);
    } else
      ERR("Couldn't find matrix model, matrix2 will not be updated!")
  }

  // Initialize update condition

  if (myProps.contains(PropNames::UPDATE_COND))
    FuncUtils::configCond(updCond_, PropNames::UPDATE_COND, myProps,
                          globdat);
  else
    updCond_ = FuncUtils::newCond(false);
  FuncUtils::getConfig(myConf, updCond_, PropNames::UPDATE_COND);

  // time stepping setttings
  myProps.find(prec_, PropNames::PRECISION);
  myConf.set(PropNames::PRECISION, prec_);
  myProps.find(dtime_, PropNames::DELTA_TIME, 0., NAN);
  myConf.set(PropNames::DELTA_TIME, dtime_);

  Globdat::getVariables(globdat).set(PropNames::DELTA_TIME, dtime_);

  minDtime_ = dtime_ / 100.;
  myProps.find(minDtime_, PropNames::MIN_DTIME, 0., dtime_);
  myConf.set(PropNames::MIN_DTIME, minDtime_);
  maxDtime_ = dtime_ * 100.;
  myProps.find(maxDtime_, PropNames::MAX_DTIME, dtime_, NAN);
  myConf.set(PropNames::MAX_DTIME, maxDtime_);

  myProps.find(saftey_, "stepSaftey", 0.5, 1.);
  myConf.set("stepSaftey", saftey_);
  myProps.find(incrFact_, "increaseFactor", 1., 2.);
  myConf.set("increaseFactor", incrFact_);
  myProps.find(decrFact_, "decreaseFactor", 1., 2.);
  myConf.set("decreaseFactor", decrFact_);

  // Initialize solver
  jive::solver::declareSolvers();

  params.set(ActionParams::CONSTRAINTS, cons_);
  model_->takeAction(Actions::NEW_MATRIX2, params, globdat);
  params.get(inertia, ActionParams::MATRIX2);
  params.clear();

  diagInertia = jem::dynamicCast<DiagMatrixObject>(inertia);

  if (diagInertia)
    mode_ = LUMPED;
  else
    mode_ = CONSISTENT;

  // prepare solver for modes
  if (mode_ == LUMPED)
  {
    massInv_.resize(dofs_->dofCount());
    Globdat::storeFor("LumpedMass", diagInertia, this, globdat);

    myConf.set("mode", "lumped");
  }

  if (mode_ == CONSISTENT)
  {
    sparams = newSolverParams(globdat, inertia, nullptr, dofs_);
    model_->takeAction(Actions::GET_SOLVER_PARAMS, sparams, globdat);
    solver_ = newSolver("explicitSolver", myConf, myProps, sparams, globdat);
    solver_->setMode(Solver::LENIENT_MODE);
    solver_->setPrecision(prec_ / 2);
    solver_->configure(myProps);
    solver_->getConfig(myConf);

    myConf.set("mode", "consistent");
  }

  // Initialize the global simulation time and the time step number as
  // well as the constraints
  params.clear();
  Globdat::initTime(globdat);
  Globdat::initStep(globdat);
  model_->takeAction(Actions::INIT, params, globdat);
  updMass(globdat);

  return OK;
}

//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------

void ExplicitModule::shutdown(const Properties &globdat)
{
  model_ = nullptr;
  solver_ = nullptr;
  dofs_ = nullptr;
  updCond_ = nullptr;
}

//-----------------------------------------------------------------------
//   configure
//-----------------------------------------------------------------------

void ExplicitModule::configure

    (const Properties &props, const Properties &globdat)

{
  using jive::implict::PropNames;
  Properties myProps = props.findProps(myName_);

  myProps.find(prec_, PropNames::PRECISION);
  myProps.find(dtime_, PropNames::DELTA_TIME, 0., NAN);

  minDtime_ = dtime_ / 1000.;
  myProps.find(minDtime_, PropNames::MIN_DTIME, 0., dtime_);

  maxDtime_ = dtime_ * 1000.;
  myProps.find(maxDtime_, PropNames::MAX_DTIME, dtime_, NAN);
}

//-----------------------------------------------------------------------
//   getConfig
//-----------------------------------------------------------------------

void ExplicitModule::getConfig

    (const Properties &conf, const Properties &globdat) const

{
  using jive::implict::PropNames;
  Properties myConf = conf.makeProps(myName_);

  myConf.set(PropNames::PRECISION, prec_);
  myConf.set(PropNames::DELTA_TIME, dtime_);
  myConf.set(PropNames::MIN_DTIME, minDtime_);
  myConf.set(PropNames::MAX_DTIME, maxDtime_);
}

//-----------------------------------------------------------------------
//   advance
//-----------------------------------------------------------------------
void
ExplicitModule::advance(const Properties& globdat)
{
  Properties params;

  // check if mass needs to be updated
  valid_ &= !FuncUtils::evalCond(*updCond_, globdat);
  if (!valid_)
    updMass(globdat);

  // update time in models and boundary conditions
  Globdat::advanceTime(dtime_, globdat);
  Globdat::advanceStep(globdat);
  model_->takeAction(Actions::ADVANCE, params, globdat);
}

//-----------------------------------------------------------------------
//   cancel
//-----------------------------------------------------------------------
void
ExplicitModule::cancel(const Properties& globdat)
{
  Properties params;

  // update time in models and boundary conditions
  Globdat::restoreTime(globdat);
  Globdat::restoreStep(globdat);
  StateVector::restoreNew(dofs_, globdat);
  model_->takeAction(Actions::CANCEL, params, globdat);
}

//-----------------------------------------------------------------------
//   commit
//-----------------------------------------------------------------------
bool
ExplicitModule::commit(const Properties& globdat)
{
  double error;
  Properties params;
  bool accept;

  SolverInfo::get(globdat).get(error, SolverInfo::RESIDUAL);

  double dtime_opt = dtime_ * pow(prec_ / error, 1. / ((double)order_ + 1.));

  if (model_->takeAction(Actions::CHECK_COMMIT, params, globdat))
    params.get(accept, ActionParams::ACCEPT);
  else // if the model doesnt care accept the solution
    accept = true;
  accept &= error <= prec_ || dtime_ == minDtime_;

  if (accept) {
    dtime_ =
        jem::max(jem::min(saftey_ * dtime_opt, incrFact_ * dtime_, maxDtime_),
                 decrFact_ * dtime_,
                 minDtime_);
    params.clear();
    model_->takeAction(Actions::COMMIT, params, globdat);
    Globdat::commitStep(globdat);
    Globdat::commitTime(globdat);
    StateVector::updateOld(dofs_, globdat);
    if (report_energy_)
      store_energy(globdat);
  }
  else
  {
    dtime_ = jem::max(saftey_ * dtime_opt, decrFact_ * dtime_, minDtime_);
  }

  jem::System::info(myName_) << " ...Adapting time step size to " << dtime_ << "\n";
  if (dtime_ == maxDtime_ && dtime_ > minDtime_)
    jem::System::info(myName_) << " !!! Largest allowed time step !!!\n";
  if (dtime_ == minDtime_ && dtime_ < maxDtime_)
    jem::System::info(myName_) << " !!! Smallest allowed time step !!!\n";
  Globdat::getVariables(globdat).set(jive::implict::PropNames::DELTA_TIME,
                                     dtime_);

  return accept;
}

//-----------------------------------------------------------------------
//   getAcce
//-----------------------------------------------------------------------
void
ExplicitModule::getAcce(const Vector& a,
                        const Ref<Constraints>& cons,
                        const Vector& fres,
                        const Properties& globdat)
{
  Properties params;

  // Compute acceleration
  if (mode_ == CONSISTENT) {
    solver_->solve(a, fres);
  }

  if (mode_ == LUMPED) {
    a = massInv_ * fres;
    jive::util::setSlaveDofs(a, *cons);
  }
}

//-----------------------------------------------------------------------
//   updateVec
//-----------------------------------------------------------------------
void
ExplicitModule::updateVec(const Vector& y_new,
                          const Vector& y_old,
                          const Vector& delta_y,
                          const bool rot)
{
  y_new = y_old + delta_y;

  if (rot)
  {
    const idx_t rotCount = SO3_dofs_.size();
    Vector r_node(rotCount);
    Vector d_r(rotCount);
    Matrix R_old(rotCount, rotCount);
    Matrix R_new(rotCount, rotCount);
    Matrix V_upd(rotCount, rotCount);

    for (idx_t inode = 0; inode < rdofs_.size(1); inode++)
    {
      r_node = y_old[rdofs_[inode]];
      d_r = delta_y[rdofs_[inode]];

      expVec(R_old, r_node);
      expVec(V_upd, d_r);
      matmul(R_new, V_upd, R_old);

      logMat(r_node, R_new);

      y_new[rdofs_[inode]] = r_node;
    }
  }
}

//-----------------------------------------------------------------------
//   getForce
//-----------------------------------------------------------------------
Vector
ExplicitModule::getForce(const Vector& fint,
                         const Vector& fext,
                         const Properties& globdat)
{
  Properties params;

  // Get the internal and external force vectors for this configuration
  fext = 0.0;
  fint = 0.0;

  params.set(ActionParams::EXT_VECTOR, fext);
  params.set(ActionParams::INT_VECTOR, fint);
  params.set(ActionParams::CONSTRAINTS, cons_);
  if (mode_ == CONSISTENT)
    params.set(ActionParams::MATRIX2, solver_->getMatrix());

  model_->takeAction(Actions::GET_CONSTRAINTS, params, globdat);
  model_->takeAction(Actions::GET_EXT_VECTOR, params, globdat);
  model_->takeAction(Actions::GET_INT_VECTOR, params, globdat);

  return Vector(fext - fint);
}

//-----------------------------------------------------------------------
//   store_energy
//-----------------------------------------------------------------------

void
ExplicitModule::store_energy(const Properties& globdat)
{
  double E_pot, E_kin, ell;
  Vector velo;
  StateVector::get(velo, jive::model::STATE1, dofs_, globdat);
  Vector temp(velo.size());

  ElementSet allElemes = ElementSet::get(globdat, getContext());
  ElementGroup beamElemes =
      ElementGroup::get("beams", allElemes, globdat, getContext());
  IdxVector inodes(allElemes.maxElemNodeCount());
  Matrix coords(allElemes.maxElemNodeCount(),
                allElemes.getNodes().rank());

  Properties params;
  Ref<ItemSet> pointSet = allElemes.getData();
  Ref<XTable> strainTable = newInstance<DenseTable>("strain", pointSet);
  Ref<XTable> stressTable = newInstance<DenseTable>("stress", pointSet);
  Vector strainWeights(pointSet->size());
  Vector stressWeights(pointSet->size());

  strainWeights = 0.;
  stressWeights = 0.;

  params.set(ActionParams::TABLE_NAME, "mat_stress");
  params.set(ActionParams::TABLE_WEIGHTS, stressWeights);
  params.set(ActionParams::TABLE, stressTable);
  model_->takeAction(Actions::GET_TABLE, params, globdat);
  params.set(ActionParams::TABLE_NAME, "mat_strain");
  params.set(ActionParams::TABLE_WEIGHTS, strainWeights);
  params.set(ActionParams::TABLE, strainTable);
  model_->takeAction(Actions::GET_TABLE, params, globdat);
  params.clear();

  IdxVector elemIDs = beamElemes.getIDs();

  stressTable->scaleRows(stressWeights);
  strainTable->scaleRows(strainWeights);

  E_pot = 0.;
  for (idx_t i : elemIDs)
  {
    allElemes.getElemNodes(inodes, i);
    allElemes.getNodes().getSomeCoords(coords.transpose(), inodes);
    ell = 0.;
    for (idx_t k = 0; k < coords.size(1); k++)
      ell += pow(coords(0, k) - coords(coords.size(0) - 1, k),
                 2.); // LATER assumes straight beam!
    ell = sqrt(ell);

    for (idx_t j = 0; j < stressTable->columnCount(); j++)
      E_pot += 0.5 * stressTable->getValue(i, j) *
               strainTable->getValue(i, j) * ell;
  }

  if (mode_ == CONSISTENT)
    solver_->getMatrix()->matmul(temp, velo);
  if (mode_ == LUMPED)
    temp = velo / massInv_;
  E_kin = 0.5 * dotProduct(temp, velo);

  Properties variables = Globdat::getVariables(globdat);
  variables.set("PotentialEnergy", E_pot);
  variables.set("KineticEnergy", E_kin);
  variables.set("TotalEnergy", E_pot + E_kin);
}

//-----------------------------------------------------------------------
//   updMass
//-----------------------------------------------------------------------

void
ExplicitModule::updMass(const Properties& globdat)
{
  Properties params;

  jem::System::info(myName_)
      << " ...Updating mass information for explicit solver\n";
  model_->takeAction(Actions::UPD_MATRIX2, params, globdat);

  if (SO3_dofs_.size())
  {
    IdxVector iitems(dofs_->getItems()->size());
    iitems = jem::iarray(dofs_->getItems()->size());

    jem::System::info(myName_)
        << " ...Updating SO(3) dof inormation for explicit solver\n";

    rdofs_.resize(SO3_dofs_.size(), dofs_->getItems()->size());
    for (idx_t idof = 0; idof < SO3_dofs_.size(); idof++)
      dofs_->getDofsForType(rdofs_(idof, ALL), iitems, SO3_dofs_[idof]);
  }

  if (mode_ == LUMPED)
  {
    Ref<DiagMatrixObject> inertia;
    Globdat::findFor(inertia, "LumpedMass", this, globdat);

    massInv_ = inertia->getValues();
    if (jem::testany(massInv_ <= jem::Limits<double>::TINY_VALUE))
      throw jem::ArithmeticException(
          "Zero (or negative) masses cannot be inversed!");
    massInv_ = 1 / massInv_;
  }

  valid_ = true;
}

//-----------------------------------------------------------------------
//   invalidate
//-----------------------------------------------------------------------

void
ExplicitModule::invalidate()
{
  valid_ = false;
}

//-----------------------------------------------------------------------
//   setPrecision
//-----------------------------------------------------------------------

void
ExplicitModule::setPrecision(double eps)
{
  prec_ = eps;
}

//-----------------------------------------------------------------------
//   getPrecision
//-----------------------------------------------------------------------

double
ExplicitModule::getPrecision() const
{
  return prec_;
}

//-----------------------------------------------------------------------
//   getQuality
//-----------------------------------------------------------------------

double
ExplicitModule::getQuality(const Vector &y_pre, const Vector &y_cor)
{
  Vector y_diff = Vector(y_pre - y_cor);
  for (idx_t i = 0; i < rdofs_.size(1); i++)
    y_diff[rdofs_[i]] *= lenScale_;
  y_diff /= lenScale_;
  return norm2(y_diff) / sqrt((double)y_diff.size());
}

// //-----------------------------------------------------------------------
// //   makeNew
// //-----------------------------------------------------------------------

// Ref<Module> ExplicitModule::makeNew

//     (const String &name, const Properties &conf, const Properties &props,
//      const Properties &globdat)

// {
//   return newInstance<Self>(name);
// }

// //-----------------------------------------------------------------------
// //   declare
// //-----------------------------------------------------------------------

// void ExplicitModule::declare()
// {
//   using jive::app::ModuleFactory;

//   ModuleFactory::declare(TYPE_NAME, &ExplicitModule::makeNew);
// }
