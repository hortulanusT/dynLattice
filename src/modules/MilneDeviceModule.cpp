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
  order_ = 1;
}

MilneDeviceModule::~MilneDeviceModule()
{
}

//-----------------------------------------------------------------------
//   solve
//-----------------------------------------------------------------------

void MilneDeviceModule::solve

    (const Properties &info, const Properties &globdat)

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

  // set the predictions to zero
  u_pre = 0.;
  v_pre = 0.;
  a_pre = 0.;

  // get the current vectors
  StateVector::get(u_cur, jive::model::STATE0, dofs_, globdat);
  StateVector::get(v_cur, jive::model::STATE1, dofs_, globdat);
  fres = getForce(fint, fext, globdat);
  getAcce(a_cur, cons_, fres, globdat);
  globdat.get(step, Globdat::TIME_STEP);

  /////////////////////////////////////////////////
  ////////  predictor step
  /////////////////////////////////////////////////

  // predict velocity
  ABupdate(dv, a_cur);
  updateVec(v_pre, v_cur, dv);

  // predict displacements
  ABupdate(du, v_cur);
  updateVec(u_pre, u_cur, du, true);

  // store predicted variables in StateVectors
  StateVector::store(u_pre, jive::model::STATE0, dofs_, globdat);
  StateVector::store(v_pre, jive::model::STATE1, dofs_, globdat);

  // predict accelerations
  fres = updForce(fint, fext, globdat);
  getAcce(a_pre, cons_, fres, globdat);

  /////////////////////////////////////////////////
  ////////  corrector step
  /////////////////////////////////////////////////

  // correct velocity
  AMupdate_(dv, a_pre);
  updateVec(v_new, v_cur, dv);

  // correct displacements
  AMupdate_(du, v_pre);
  updateVec(u_new, u_cur, du, true);

  // store corrected variables in StateVectors
  StateVector::store(u_new, jive::model::STATE0, dofs_, globdat);
  StateVector::store(v_new, jive::model::STATE1, dofs_, globdat);

  /////////////////////////////////////////////////
  ////////  step size adaption
  /////////////////////////////////////////////////
  correction = 0.;
  correction += getQuality(u_pre, u_new);
  correction += getQuality(v_pre, v_new) * dtime_;

  info.set(SolverInfo::RESIDUAL, 0.5 * correction);
}

//-----------------------------------------------------------------------
//   updForce
//-----------------------------------------------------------------------
Vector
MilneDeviceModule::updForce(const Vector &fint,
                            const Vector &fext,
                            const Properties &globdat)
{
  Properties params;

  // update the internal force vector for this configuration
  fint = 0.0;

  params.set(ActionParams::INT_VECTOR, fint);
  if (mode_ == CONSISTENT)
    params.set(ActionParams::MATRIX2, solver_->getMatrix());

  model_->takeAction(Actions::GET_INT_VECTOR, params, globdat);

  return Vector(fext - fint);
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
