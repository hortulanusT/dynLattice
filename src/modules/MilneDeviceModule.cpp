#include "modules/MilneDeviceModule.h"
#include "utils/testing.h"

#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(MilneDeviceModule);

//=======================================================================
//   class CSVOutputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *MilneDeviceModule::TYPE_NAME = "MilneDevice";
//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

MilneDeviceModule::MilneDeviceModule

    (const String &name)
    :

      Super(name)

{
}

MilneDeviceModule::~MilneDeviceModule()
{
}

//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------

Module::Status MilneDeviceModule::run

    (const Properties &globdat)

{
  const idx_t dofCount = dofs_->dofCount();
  idx_t step;
  double correction;
  Vector u_pre(dofCount);
  Vector v_pre(dofCount);
  Vector a_pre(dofCount);
  Vector u_cur, v_cur;
  Vector a_cur(dofCount);
  Vector u_old, v_old, a_old;
  Vector u_new(dofCount);
  Vector v_new(dofCount);
  Vector a_new(dofCount);
  Vector dv(dofCount);
  Vector du(dofCount);
  Vector fres(dofCount);
  Vector fint(dofCount);
  Vector fext(dofCount);

  // skip if no model exists
  if (!(model_))
    return DONE;

  // update mass matrix if necessary
  if (!valid_)
    restart_(globdat);

  // set the predictions to zero
  u_pre = 0.;
  v_pre = 0.;
  a_pre = 0.;

  // get the current vectors
  StateVector::get(u_cur, jive::model::STATE0, dofs_, globdat);
  StateVector::get(v_cur, jive::model::STATE1, dofs_, globdat);
  fres = getForce_(fint, fext, globdat);
  getAcce_(a_cur, cons_, fres, globdat);
  // get the old vectors
  // TODO store in class
  StateVector::getOld(u_old, jive::model::STATE0, dofs_, globdat);
  StateVector::getOld(v_old, jive::model::STATE1, dofs_, globdat);

  globdat.get(step, Globdat::TIME_STEP);

  /////////////////////////////////////////////////
  ////////  predictor step
  /////////////////////////////////////////////////

  // predict velocity
  if (stepCount_ >= 2 && step >= 2)
    ABupdate_(dv, a_cur, a_old);
  else
    ABupdate_(dv, a_cur);
  updateVec_(v_pre, v_cur, dv);

  // predict displacements;
  if (stepCount_ >= 2 && step >= 2)
    ABupdate_(du, v_cur, v_old);
  else
    ABupdate_(du, v_cur);
  updateVec_(u_pre, u_cur, du, true);

  // store predicted variables in StateVectors
  StateVector::store(u_pre, jive::model::STATE0, dofs_, globdat);
  StateVector::store(v_pre, jive::model::STATE1, dofs_, globdat);

  // predict accelerations
  fres = getForce_(fint, fext, globdat);
  getAcce_(a_pre, cons_, fres, globdat);

  /////////////////////////////////////////////////
  ////////  corrector step
  /////////////////////////////////////////////////

  // correct velocity
  if (stepCount_ == 2 && step > 0)
    AMupdate_(dv, a_pre, a_cur);
  else
    AMupdate_(dv, a_pre);
  updateVec_(v_new, v_cur, dv);

  // correct displacements;
  if (stepCount_ == 2 && step > 0)
    AMupdate_(du, v_pre, v_cur);
  else
    AMupdate_(du, v_pre);
  updateVec_(u_new, u_cur, du, true);

  // store corrected variables in StateVectors
  StateVector::store(u_new, jive::model::STATE0, dofs_, globdat);
  StateVector::store(v_new, jive::model::STATE1, dofs_, globdat);

  /////////////////////////////////////////////////
  ////////  step size adaption
  /////////////////////////////////////////////////
  correction = 0.;
  correction += getQuality_(u_pre, u_new);
  correction += getQuality_(v_pre, v_new) * dtime_;

  // compute the optimal step size and decide wether to accept this step
  if (updStep_(correction, globdat))
  {
    // commit everything
    Properties params;
    Globdat::commitStep(globdat);
    Globdat::commitTime(globdat);
    model_->takeAction(Actions::COMMIT, params, globdat);

    StateVector::updateOld(dofs_, globdat);
    StateVector::store(u_new, jive::model::STATE0, dofs_, globdat);
    StateVector::store(v_new, jive::model::STATE1, dofs_, globdat);

    // advance to the next step
    advance_(globdat);

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
    StateVector::store(u_cur, jive::model::STATE0, dofs_, globdat);
    StateVector::store(v_cur, jive::model::STATE1, dofs_, globdat);

    jem::System::info(myName_) << " ...restarting time step\n";

    return run(globdat);
  }
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> MilneDeviceModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<Self>(name);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void MilneDeviceModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &MilneDeviceModule::makeNew);
}
