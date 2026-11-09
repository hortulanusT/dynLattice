
/**
 * @file ExplicitModule.cpp
 * @author Til Gärtner
 * @brief Implementation of ExplicitModule class
 */

#include "modules/ExplicitModule.h"
#include "utils/SolverNames.h"
#include "utils/testing.h"

#include <jem/base/ClassTemplate.h>
#include <jem/base/IllegalInputException.h>
#include <jem/util/StringUtils.h>

//=======================================================================
//   class ExplicitModule
//=======================================================================

JEM_DEFINE_CLASS(ExplicitModule);

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *ExplicitModule::TYPE_NAME = "Explicit";
const char *ExplicitModule::STEP_COUNT = "stepCount";
const char *ExplicitModule::SO3_DOFS = "dofs_SO3";
const char *ExplicitModule::LEN_SCALE = "lengthScale";
const char *ExplicitModule::ROT_SCALE = "rotationScale";
const char *ExplicitModule::MASS_MATRIX = "explicitMass";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

ExplicitModule::ExplicitModule

    (const String &name)
    : Super(name)

{
  dtime_ = 1.0;
  valid_ = false;
  updMassStep_ = false;
  fintCacheValid_ = false;
  rotDofsValid_ = false;
  lenScale_ = 1e-3;
  rotScale_ = lenScale_;
  dofsSO3_.resize(0);
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
  using jem::util::StringUtils;
  using jive::implict::PropNames;

  Properties myConf = conf.makeProps(myName_);
  Properties myProps = props.findProps(myName_);
  StringVector SO3_dof_names;
  Properties params;
  Properties sparams;
  Ref<AbstractMatrix> inertia;
  Ref<DiagMatrixObject> diagInertia;

  // get the length scale
  myProps.find(lenScale_, LEN_SCALE);
  myConf.set(LEN_SCALE, lenScale_);
  rotScale_ = lenScale_;
  myProps.find(rotScale_, ROT_SCALE);
  myConf.set(ROT_SCALE, rotScale_);

  // returning an address of an object pointing to the Model

  model_ = Model::get(globdat, getContext());

  // returning an address of an object pointing to class DofSpace

  dofs_ = DofSpace::get(globdat, getContext());
  cons_ = Constraints::find(dofs_, globdat);

  // invalidate_ the state of this module when the DofSpace changes.

  connect(dofs_->newSizeEvent, this, &Self::invalidate_);
  connect(dofs_->newOrderEvent, this, &Self::invalidate_);

  // SO(3) dofs: dofs_SO3 if given (must equal the model's), else the rotation dofs the model reports

  StringVector modelRotDofs;
  const bool modelHasRot = model_->takeAction(SolverNames::GET_ROT_DOFS, params, globdat) &&
                           params.find(modelRotDofs, SolverNames::ROT_DOFS);
  params.clear();
  const bool givenRot = myProps.find(SO3_dof_names, SO3_DOFS);

  if (modelHasRot && !givenRot)
  {
    SO3_dof_names.ref(modelRotDofs);
    jem::System::info(myName_) << " ...rotation dofs from the model (" << SO3_DOFS << " not given): "
                               << StringUtils::join(SO3_dof_names, ", ") << "\n";
  }
  else if (modelHasRot && !jem::equal(SO3_dof_names, modelRotDofs))
    throw jem::IllegalInputException(getContext(), String(SO3_DOFS) + " = [" + StringUtils::join(SO3_dof_names, ", ") +
                                                       "] differs from the rotation dofs of the model [" +
                                                       StringUtils::join(modelRotDofs, ", ") + "]");

  if (givenRot || modelHasRot)
  {
    JEM_PRECHECK(SO3_dof_names.size() == 3);
    // rdofs_ and the mask follow in updateMass_ at the end of init
    dofsSO3_.resize(SO3_dof_names.size());
    for (idx_t i = 0; i < SO3_dof_names.size(); i++)
      dofsSO3_[i] = dofs_->getTypeIndex(SO3_dof_names[i]);
    myConf.set(SO3_DOFS, SO3_dof_names);

    String modelType;
    props.get(modelType, "model.type");
    if (modelType == "Matrix")
    {
      props.set("model.matrix2.type", FlexMatrixBuilder::getType()->getName());
      props.set("model.matrix2.constant", false);
    }
    else
      ERR("Couldn't find matrix model, matrix2 will not be updated!")
  }

  // Initialize update condition

  if (myProps.contains(PropNames::UPDATE_COND))
    FuncUtils::configCond(updCond_, PropNames::UPDATE_COND, myProps,
                          globdat);
  else
    updCond_ = FuncUtils::newCond(true);
  FuncUtils::getConfig(myConf, updCond_, PropNames::UPDATE_COND);

  // time stepping settings
  configure(props, globdat);
  getConfig(conf, globdat);

  Globdat::getVariables(globdat).set(PropNames::DELTA_TIME, dtime_);

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
  updateMass_(globdat);

  return OK;
}

//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------

void ExplicitModule::shutdown(const Properties &globdat)
{
  (void)globdat; // unused

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
  (void)globdat; // unused

  using jive::implict::PropNames;
  Properties myProps = props.findProps(myName_);

  myProps.find(prec_, PropNames::PRECISION);
  myProps.find(dtime_, PropNames::DELTA_TIME, 0., NAN);

  minDtime_ = dtime_ / 100.;
  myProps.find(minDtime_, PropNames::MIN_DTIME, 0., dtime_);

  maxDtime_ = dtime_ * 100.;
  myProps.find(maxDtime_, PropNames::MAX_DTIME, dtime_, NAN);

  myProps.find(saftey_, "stepSaftey", 0.5, 1.);
  myProps.find(incrFact_, "increaseFactor", 1., 2.);
  myProps.find(decrFact_, "decreaseFactor", 0., 1.);
}

//-----------------------------------------------------------------------
//   getConfig
//-----------------------------------------------------------------------

void ExplicitModule::getConfig

    (const Properties &conf, const Properties &globdat) const

{
  (void)globdat; // unused

  using jive::implict::PropNames;
  Properties myConf = conf.makeProps(myName_);

  myConf.set(PropNames::PRECISION, prec_);
  myConf.set(PropNames::DELTA_TIME, dtime_);
  myConf.set(PropNames::MIN_DTIME, minDtime_);
  myConf.set(PropNames::MAX_DTIME, maxDtime_);
  myConf.set("stepSaftey", saftey_);
  myConf.set("increaseFactor", incrFact_);
  myConf.set("decreaseFactor", decrFact_);
}

//-----------------------------------------------------------------------
//   advance
//-----------------------------------------------------------------------
void ExplicitModule::advance(const Properties &globdat)
{
  Properties params;

  // outputs of the finished step have been written by now; the cached copy
  // stays valid for getForce(), which compares the state before using it
  globdat.erase(ActionParams::INT_VECTOR);
  globdat.erase(MASS_MATRIX);

  // check if mass needs to be updated
  updMassStep_ = FuncUtils::evalCond(*updCond_, globdat);
  valid_ = valid_ && !updMassStep_;
  if (!valid_)
    updateMass_(globdat);

  // update time in models and boundary conditions
  Globdat::advanceTime(dtime_, globdat);
  Globdat::advanceStep(globdat);
  model_->takeAction(Actions::ADVANCE, params, globdat);
}

//-----------------------------------------------------------------------
//   cancel
//-----------------------------------------------------------------------
void ExplicitModule::cancel(const Properties &globdat)
{
  Properties params;

  // update time in models and boundary conditions
  dropCommittedForce_(globdat);
  Globdat::restoreTime(globdat);
  Globdat::restoreStep(globdat);
  StateVector::restoreNew(dofs_, globdat);
  model_->takeAction(Actions::CANCEL, params, globdat);
}

//-----------------------------------------------------------------------
//   commit
//-----------------------------------------------------------------------
bool ExplicitModule::commit(const Properties &globdat)
{
  double error;
  Properties params;
  bool accept;

  SolverInfo::get(globdat).get(error, SolverInfo::RESIDUAL);

  double dtime_opt = dtime_ * pow(prec_ / error, 1. / (static_cast<double>(order_) + 1.));

  accept = true;
  if (model_->takeAction(Actions::CHECK_COMMIT, params, globdat))
  {
    params.find(accept, ActionParams::ACCEPT);
  }
  accept = accept && (error <= prec_ || dtime_ <= minDtime_);

  // next step from the error estimate: after an accepted step at most incrFact_ times larger, never below
  // decrFact_ times the old one, within [minDtime_, maxDtime_] (a step rejected by the model can have a small error)
  double dtime_next = saftey_ * dtime_opt;
  if (accept)
    dtime_next = jem::min(dtime_next, incrFact_ * dtime_);
  dtime_next = jem::min(jem::max(dtime_next, decrFact_ * dtime_, minDtime_), maxDtime_);

  if (accept)
  {
    dtime_ = dtime_next;
    params.clear();
    evalCommittedForce_(globdat);
    model_->takeAction(Actions::COMMIT, params, globdat);
    Globdat::commitStep(globdat);
    Globdat::commitTime(globdat);
    StateVector::updateOld(dofs_, globdat);
    publishCommittedForce_(globdat);
  }
  else
  {
    dropCommittedForce_(globdat);
    dtime_ = dtime_next;
  }

  jem::System::info(myName_) << " ...Adapting time step size to " << dtime_ << "\n";
  if (dtime_ >= maxDtime_ && dtime_ > minDtime_)
    jem::System::info(myName_) << " !!! Largest allowed time step !!!\n";
  if (dtime_ <= minDtime_ && dtime_ < maxDtime_)
    jem::System::info(myName_) << " !!! Smallest allowed time step !!!\n";
  Globdat::getVariables(globdat).set(jive::implict::PropNames::DELTA_TIME,
                                     dtime_);

  return accept;
}

//-----------------------------------------------------------------------
//   getAcce
//-----------------------------------------------------------------------
void ExplicitModule::getAcce(const Vector &a,
                             const Ref<Constraints> &cons,
                             const Vector &fres,
                             const Properties &globdat)
{
  (void)globdat; // unused

  // Compute acceleration
  if (mode_ == CONSISTENT)
  {
    solver_->solve(a, fres);
  }

  if (mode_ == LUMPED)
  {
    a = massInv_ * fres;
    jive::util::setSlaveDofs(a, *cons);
  }
}

//-----------------------------------------------------------------------
//   updateVec
//-----------------------------------------------------------------------
void ExplicitModule::updateVec(const Vector &y_new,
                               const Vector &y_old,
                               const Vector &delta_y,
                               const bool rot)
{
  y_new = y_old + delta_y;

  if (rot)
  {
    const idx_t rotCount = dofsSO3_.size();
    Vector r_node(rotCount);
    Vector d_r(rotCount);
    Matrix R_old(rotCount, rotCount);
    Matrix R_new(rotCount, rotCount);
    Matrix V_upd(rotCount, rotCount);
    Vector y_add(rotCount);

    for (idx_t inode = 0; inode < rdofs_.size(1); inode++)
    {
      r_node = y_old[rdofs_[inode]];
      d_r = delta_y[rdofs_[inode]];

      expVec(R_old, r_node);
      expVec(V_upd, d_r);
      mul3(R_new, V_upd, R_old);

      logMat(r_node, R_new);

      // logMat gives |r| <= pi: of r + 2 pi k r/|r| take the one closest to the additive prediction y_add
      y_add = y_new[rdofs_[inode]];
      const double r_norm = norm2(r_node);
      if (r_norm > jive_helpers::TINY)
      {
        const double turns = (dotProduct(y_add, r_node) / r_norm - r_norm) / (2. * M_PI);
        const idx_t k = static_cast<idx_t>(floor(turns + 0.5));
        if (k != 0)
          r_node *= (r_norm + 2. * M_PI * static_cast<double>(k)) / r_norm;
      }
      else
      {
        // no rotation: a multiple of 2 pi about the axis of the prediction
        const double y_norm = norm2(y_add);
        const idx_t k = static_cast<idx_t>(floor(y_norm / (2. * M_PI) + 0.5));
        if (k != 0)
          r_node += (2. * M_PI * static_cast<double>(k) / y_norm) * y_add;
      }

      y_new[rdofs_[inode]] = r_node;
    }
  }
}

//-----------------------------------------------------------------------
//   getForce
//-----------------------------------------------------------------------
Vector
ExplicitModule::getForce(const Vector &fint,
                         const Vector &fext,
                         const Properties &globdat,
                         const bool mayReuse)
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

  // The state is the committed one if nothing touched it since the commit;
  // then the internal force of the output pass can be reused.
  bool reuse = false;
  if (fintCacheValid_ && mayReuse)
  {
    Vector disp;
    StateVector::get(disp, dofs_, globdat);
    reuse = jem::testall(disp == fintCacheDisp_);
  }

  if (reuse)
  {
    fint = fintCache_;
    // only the gyroscopic term depends on the (new) mass matrix
    if (mode_ == CONSISTENT)
      model_->takeAction("GET_GYRO_VECTOR", params, globdat);

#ifndef NDEBUG
    Vector fintFresh(fint.size());
    Properties checkParams;
    fintFresh = 0.0;
    checkParams.set(ActionParams::INT_VECTOR, fintFresh);
    if (mode_ == CONSISTENT)
      checkParams.set(ActionParams::MATRIX2, solver_->getMatrix());
    model_->takeAction(Actions::GET_INT_VECTOR, checkParams, globdat);
    const double scale = 1. + norm2(fintFresh);
    JEM_ASSERT2(norm2(Vector(fintFresh - fint)) <= 1e-9 * scale,
                "reused internal force differs from a fresh evaluation");
#endif
  }
  else
  {
    model_->takeAction(Actions::GET_INT_VECTOR, params, globdat);
  }

  return Vector(fext - fint);
}

//-----------------------------------------------------------------------
//   getStartForce_
//-----------------------------------------------------------------------

Vector ExplicitModule::getStartForce_(const Vector &fint,
                                      const Vector &fext,
                                      const Properties &globdat)
{
  double t_new;
  double t_old;

  globdat.get(t_new, Globdat::TIME);
  globdat.get(t_old, Globdat::OLD_TIME);
  JEM_ASSERT2(jem::numeric::abs(t_new - dtime_ - t_old) <= 1e-12 * (jem::numeric::abs(t_new) + dtime_),
              "the old time is not the start of the step");

  globdat.set(Globdat::TIME, t_old);
  Vector fres = getForce(fint, fext, globdat);
  globdat.set(Globdat::TIME, t_new);

  return fres;
}

//-----------------------------------------------------------------------
//   evalCommittedForce_
//-----------------------------------------------------------------------

void ExplicitModule::evalCommittedForce_(const Properties &globdat)
{
  Properties params;
  const idx_t dofCount = dofs_->dofCount();

  fintCache_.resize(dofCount);
  fintCache_ = 0.0;

  // the same evaluation the next step starts with (no output load case,
  // no MATRIX2, i.e. without the gyroscopic term)
  params.set(ActionParams::INT_VECTOR, fintCache_);
  model_->takeAction(Actions::GET_INT_VECTOR, params, globdat);
}

//-----------------------------------------------------------------------
//   publishCommittedForce_
//-----------------------------------------------------------------------

void ExplicitModule::publishCommittedForce_(const Properties &globdat)
{
  Vector disp;
  StateVector::get(disp, dofs_, globdat);
  fintCacheDisp_.ref(Vector(disp.clone()));
  fintCacheValid_ = true;

  // Publish the cached force to the global data container
  globdat.set(ActionParams::INT_VECTOR, fintCache_.clone());

  // only a reference: output modules can evaluate the gyroscopic force with
  // it on demand (consistent mass only; lumped mode has no gyroscopic term)
  if (mode_ == CONSISTENT)
    globdat.set(MASS_MATRIX, solver_->getMatrix());
}

//-----------------------------------------------------------------------
//   dropCommittedForce_
//-----------------------------------------------------------------------

void ExplicitModule::dropCommittedForce_(const Properties &globdat)
{
  if (fintCacheValid_)
    globdat.erase(ActionParams::INT_VECTOR);
  globdat.erase(MASS_MATRIX);
  fintCacheValid_ = false;
}

//-----------------------------------------------------------------------
//   updateMass_
//-----------------------------------------------------------------------

void ExplicitModule::updateMass_(const Properties &globdat)
{
  Properties params;

  jem::System::info(myName_)
      << " ...Updating mass information for explicit solver\n";
  model_->takeAction(Actions::UPD_MATRIX2, params, globdat);

  // the SO(3) dof indices change only with the dof space
  if (dofsSO3_.size() && !rotDofsValid_)
  {
    IdxVector iitems(dofs_->getItems()->size());
    iitems = jem::iarray(dofs_->getItems()->size());

    jem::System::info(myName_)
        << " ...Updating SO(3) dof indices for explicit solver\n";

    rdofs_.resize(dofsSO3_.size(), dofs_->getItems()->size());
    for (idx_t idof = 0; idof < dofsSO3_.size(); idof++)
      dofs_->getDofsForType(rdofs_(idof, ALL), iitems, dofsSO3_[idof]);
    updRotMask_();
    rotDofsValid_ = true;
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
//   updRotMask_
//-----------------------------------------------------------------------

void ExplicitModule::updRotMask_()
{
  isRotDof_.resize(dofs_->dofCount());
  isRotDof_ = false;

  for (idx_t iRot = 0; iRot < rdofs_.size(0); iRot++)
    for (idx_t iNode = 0; iNode < rdofs_.size(1); iNode++)
    {
      const idx_t idof = rdofs_(iRot, iNode);
      if (idof >= 0 && idof < isRotDof_.size())
        isRotDof_[idof] = true;
    }
}

//-----------------------------------------------------------------------
//   invalidate_
//-----------------------------------------------------------------------

void ExplicitModule::invalidate_()
{
  valid_ = false;
  fintCacheValid_ = false;
  rotDofsValid_ = false;
}

//-----------------------------------------------------------------------
//   setPrecision
//-----------------------------------------------------------------------

void ExplicitModule::setPrecision(double eps)
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
  // one division for the rotations: lenScale_ / rotScale_ = 1 exactly for the default rotScale_ = lenScale_
  const double rotDiv = lenScale_ / rotScale_;
  Vector y_diff = Vector(y_pre - y_cor);
  for (idx_t i = 0; i < y_diff.size(); i++)
    if (i >= isRotDof_.size() || !isRotDof_[i])
      y_diff[i] /= lenScale_;
    else
      y_diff[i] /= rotDiv;
  return norm2(y_diff) / sqrt(static_cast<double>(y_diff.size()));
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
