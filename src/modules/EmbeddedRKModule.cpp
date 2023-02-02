#include "modules/EmbeddedRKModule.h"
#include "utils/testing.h"

#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(EmbeddedRKModule);

//=======================================================================
//   class CSVOutputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *EmbeddedRKModule::TYPE_NAME = "EmbeddedRK";
//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

EmbeddedRKModule::EmbeddedRKModule

    (const String &name)
    :

      Super(name)

{
}

EmbeddedRKModule::~EmbeddedRKModule()
{
}

//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------

Module::Status EmbeddedRKModule::init

    (const Properties &conf, const Properties &props,
     const Properties &globdat)
{
  Status superStatus = Super::init(conf, props, globdat);
  JEM_ASSERT(superStatus == Status::OK);

  Properties myConf = conf.makeProps(myName_);
  Properties myProps = props.findProps(myName_);

  myProps.get(kind_, "kind");
  myConf.set("kind", kind_);

  if (kind_ == "ode23")
    initODE23_();
  else if (kind_ == "ode45")
    initODE45_();
  else
    throw jem::IllegalInputException(
        "Unkown kind of embedded RK method!");

  return Status::OK;
}

void EmbeddedRKModule::initODE45_()
{
  butchSize_ = 7;
  order_ = 5;

  a_.resize(butchSize_, butchSize_);
  a_ = 0.;
  a_(1, 0) = 1. / 5.;
  a_(2, 0) = 3. / 40.;
  a_(2, 1) = 9. / 40.;
  a_(3, 0) = 44. / 45.;
  a_(3, 1) = -56. / 15.;
  a_(3, 2) = 32. / 9.;
  a_(4, 0) = 19372. / 6561.;
  a_(4, 1) = -25360. / 3298.;
  a_(4, 2) = 64448. / 6561.;
  a_(4, 3) = -212. / 729.;
  a_(5, 0) = 9017. / 3168.;
  a_(5, 1) = -355. / 33.;
  a_(5, 2) = 46732. / 5247.;
  a_(5, 3) = 49. / 176.;
  a_(5, 4) = -5103. / 18656.;
  a_(6, 0) = 35. / 384.;
  a_(6, 2) = 500. / 1113.;
  a_(6, 3) = 125. / 192.;
  a_(6, 4) = -2187. / 6784.;
  a_(6, 5) = 11. / 84.;

  b_.resize(butchSize_);
  b_ = 0.;
  b_[0] = 5179. / 57600.;
  b_[2] = 7571. / 16695.;
  b_[3] = 393. / 640.;
  b_[4] = -92097. / 339200.;
  b_[5] = 187. / 2100.;
  b_[6] = 1. / 40.;

  c_.resize(butchSize_);
  c_ = 0.;
  c_[1] = 1. / 5.;
  c_[2] = 3. / 10.;
  c_[3] = 4. / 5.;
  c_[4] = 8. / 9.;
  c_[5] = 1.;
  c_[6] = 1.;

  fsal_ = true;
}

void EmbeddedRKModule::initODE23_()
{
  butchSize_ = 4;
  order_ = 3;

  a_.resize(butchSize_, butchSize_);
  a_ = 0.;
  a_(1, 0) = 1. / 2.;
  a_(2, 1) = 3. / 4.;
  a_(3, 0) = 2. / 9.;
  a_(3, 1) = 3. / 9.;
  a_(3, 2) = 4. / 9.;

  b_.resize(butchSize_);
  b_ = 0.;
  b_[0] = 7. / 24.;
  b_[1] = 6. / 24.;
  b_[2] = 8. / 24.;
  b_[3] = 3. / 24.;

  c_.resize(butchSize_);
  c_ = 0.;
  c_[1] = 1. / 2.;
  c_[2] = 3. / 4.;
  c_[3] = 1.;

  fsal_ = true;
}

//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------

Module::Status EmbeddedRKModule::run

    (const Properties &globdat)

{
  const idx_t dofCount = dofs_->dofCount();
  idx_t step;
  double error;
  double t_cur;
  Vector u_cur, v_cur, a_cur;
  Vector du(dofCount);
  Vector dv(dofCount);
  double t_step;
  Vector u_step(dofCount);
  Vector v_step(dofCount);
  Vector a_step(dofCount);
  Vector fres(dofCount);
  Vector fint(dofCount);
  Vector fext(dofCount);
  Matrix ku_tab(dofCount, butchSize_);
  Matrix kv_tab(dofCount, butchSize_);
  Vector u_new(dofCount);
  Vector v_new(dofCount);
  Vector a_new(dofCount);

  // skip if no model exists
  if (!(model_))
    return DONE;

  // update mass matrix if necessary
  if (!valid_)
    restart_(globdat);

  // get the current vectors
  StateVector::get(u_cur, jive::model::STATE0, dofs_, globdat);
  StateVector::get(v_cur, jive::model::STATE1, dofs_, globdat);
  StateVector::get(a_cur, jive::model::STATE2, dofs_, globdat);
  globdat.get(t_cur, Globdat::TIME);

  /////////////////////////////////////////////////
  ////////  walk over the butcher tableau
  /////////////////////////////////////////////////
  for (idx_t i = 0; i < butchSize_; i++)
  {
    jem::System::info(myName_)
        << "\n ...Runge Kutta Level " << i + 1 << "\n\n";

    // get the updates (U_j in RKMK reference)
    dv = dtime_ * a_cur;
    for (idx_t j = 0; j < i; j++)
      dv += a_(i, j) * kv_tab[j];
    du = dtime_ * v_cur;
    for (idx_t j = 0; j < i; j++)
      du += a_(i, j) * ku_tab[j];

    // use the updates to compute new functions
    updateVec_(v_step, v_cur, dv);
    updateVec_(u_step, u_cur, du, true);
    t_step = t_cur + c_[i] * dtime_;

    StateVector::store(u_step, jive::model::STATE0, dofs_, globdat);
    StateVector::store(v_step, jive::model::STATE1, dofs_, globdat);
    globdat.set(Globdat::TIME, t_step);

    fres = getForce_(fint, fext, globdat);
    getAcce_(a_step, cons_, fres, globdat);

    // compute the k - values (F_j in the reference)
    kv_tab[i] = dtime_ * a_step;
    ku_tab[i] = dtime_ * v_step;
    // correct the displacements (K_j in the reference)
    correctDisp_(ku_tab[i], du);
  }

  jem::System::info(myName_) << "\n ...Runge Kutta Advancement\n\n";

  /////////////////////////////////////////////////
  ////////  high order solution
  /////////////////////////////////////////////////
  if (fsal_)
  {
    u_new = u_step;
    v_new = v_step;
    a_new = a_step;
  }
  else
    NOT_IMPLEMENTED

  /////////////////////////////////////////////////
  ////////  low order solution
  /////////////////////////////////////////////////
  // compute the values at the current step

  // get the updates (U_j in RKMK reference)
  dv = dtime_ * a_cur;
  for (idx_t j = 0; j < butchSize_; j++)
    dv += b_[j] * kv_tab[j];
  du = dtime_ * v_cur;
  for (idx_t j = 0; j < butchSize_; j++)
    du += b_[j] * ku_tab[j];

  // use the updates to compute new functions
  updateVec_(v_step, v_cur, dv);
  updateVec_(u_step, u_cur, du, true);

  /////////////////////////////////////////////////
  ////////  step size adaption
  /////////////////////////////////////////////////
  error = 0.;
  error += getQuality_(u_step, u_new);
  error += getQuality_(v_step, v_new) * dtime_;

  // compute the optimal step size and decide wether to accept this step
  if (updStep_(error, globdat))
  {
    // commit everything
    Properties params;
    Globdat::commitStep(globdat);
    Globdat::commitTime(globdat);
    model_->takeAction(Actions::COMMIT, params, globdat);

    StateVector::updateOld(dofs_, globdat);
    StateVector::store(u_new, jive::model::STATE0, dofs_, globdat);
    StateVector::store(v_new, jive::model::STATE1, dofs_, globdat);
    StateVector::store(a_new, jive::model::STATE2, dofs_, globdat);

    // advance to the next step
    step = advance_(globdat);

    // check if the mass matrix is still valid
    if (FuncUtils::evalCond(*updCond_, globdat))
      invalidate_();

    // if the engergy needs to be reported, do so
    if (report_energy_)
      store_energy_(globdat);

    return Status::OK;
  }
  else
  {
    Globdat::restoreStep(globdat);
    Globdat::restoreTime(globdat);
    StateVector::restoreNew(dofs_, globdat);

    jem::System::info(myName_) << " ...restarting time step\n";

    return run(globdat);
  }
}

// correct the caluclated function results
// (see
// https://link.springer.com/referenceworkentry/10.1007/978-3-540-70529-1_122#Sec1139)
void EmbeddedRKModule::correctDisp_(const Vector &uncorrected,
                                    const Vector &delta)
{
  const idx_t rotCount = rdofs_.size(1);
  const idx_t nodeCount = rdofs_.size(1);

  Vector localu(rotCount);
  Vector localf(rotCount);
  Vector localk(rotCount);
  Matrix localU(rotCount, rotCount);
  Matrix localF(rotCount, rotCount);
  Matrix localK(rotCount, rotCount);

  for (idx_t inode = 0; inode < nodeCount; inode++)
  {
    localu = delta[rdofs_[inode]];
    localf = uncorrected[rdofs_[inode]];

    expVec(localU, localu);
    expVec(localF, localf);

    invDerivExpMap_(localK, localF, localU);

    logMat(localk, localK);

    uncorrected[rdofs_[inode]] = localk;
  }
}

// compute the inverse of the Darboux derivative of the exponential map
// (see
// https://link.springer.com/referenceworkentry/10.1007/978-3-540-70529-1_122#Sec1139)
void EmbeddedRKModule::invDerivExpMap_(const Matrix &res,
                                       const Matrix &point,
                                       const Matrix &about)
{
  res = 0.;

  for (idx_t i = 0; i < order_; i++)
    res += bernoulliCoeff_(i) * adjoint_(point, about, i);
}

double EmbeddedRKModule::bernoulliCoeff_(const idx_t i)
{
  switch (i)
  {
  case 0:
    return 1;
  case 1:
    return -1. / 2.;
  case 2:
    return 1. / 12.;
  case 3:
    return 0.;
  case 4:
    return -1. / 720.;
  case 5:
    return 0.;
  case 6:
    return 1. / 30240.;
  case 7:
    return 0.;
  case 8:
    return -1. / 1209600.;
  case 9:
    return 0.;
  case 10:
    return 1. / 47900160.;
  default:
    throw jem::Exception("We do not dare to go this high!");
  }
}
Matrix EmbeddedRKModule::adjoint_(const Matrix &point,
                                  const Matrix &about,
                                  const idx_t iterate)
{
  JEM_ASSERT(iterate >= 0);

  if (iterate == 0)
    return point;
  else
  {
    Matrix res = adjoint_(point, about, iterate - 1);
    res = matmul(about, res) - matmul(res, about);
    return res;
  }
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> EmbeddedRKModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<Self>(name);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void EmbeddedRKModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &EmbeddedRKModule::makeNew);
}
