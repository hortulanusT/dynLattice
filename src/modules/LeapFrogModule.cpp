#include "modules/LeapFrogModule.h"
#include "utils/testing.h"

#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(LeapFrogModule);

//=======================================================================
//   class CSVOutputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *LeapFrogModule::TYPE_NAME = "LeapFrog";
//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

LeapFrogModule::LeapFrogModule

    (const String &name)
    :

      Super(name)

{
}

LeapFrogModule::~LeapFrogModule()
{
}

//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------

Module::Status LeapFrogModule::run

    (const Properties &globdat)

{
  using jive::model::STATE;
  const idx_t dofCount = dofs_->dofCount();

  idx_t step;
  Properties params;
  Vector fint(dofCount);
  Vector fext(dofCount);
  Vector fres(dofCount);
  Vector u_old, v_old, a_old;
  Vector dv(dofCount);
  Vector du(dofCount);
  Vector u_new(dofCount);
  Vector v_new(dofCount);
  Vector a_new(dofCount);

  // skip if no model exists
  if (!(model_))
    return DONE;

  // update mass matrix if necessary
  if (!valid_)
    restart_(globdat);

  globdat.get(step, Globdat::TIME_STEP);

  // Get the state vectors from the last time step (velocities actually at
  // half steps!)
  StateVector::get(u_old, STATE[0], dofs_, globdat);
  StateVector::get(v_old, STATE[1], dofs_, globdat);
  StateVector::get(a_old, STATE[2], dofs_, globdat);
  params.clear();

  // Compute new accelerations
  fres = getForce_(fint, fext, globdat);
  getAcce_(a_new, cons_, fres, globdat);

  // update velocity
  if (stepCount_ == 2 && step > 0)
    ABupdate_(dv, a_new, a_old);
  else
    ABupdate_(dv, a_new);

  updateVec_(v_new, v_old, dv);

  // update position
  if (stepCount_ >= 2 && step > 2)
    ABupdate_(du, v_new, v_old);
  else
    ABupdate_(du, v_new);

  updateVec_(u_new, u_old, du, true);

  // commit everything
  Globdat::commitStep(globdat);
  Globdat::commitTime(globdat);
  model_->takeAction(Actions::COMMIT, params, globdat);

  StateVector::updateOld(dofs_, globdat);
  StateVector::store(a_new, STATE[2], dofs_, globdat);
  StateVector::store(v_new, STATE[1], dofs_, globdat);
  StateVector::store(u_new, STATE[0], dofs_, globdat);

  advance_(globdat);

  // check if the mass matrix is still valid
  if (FuncUtils::evalCond(*updCond_, globdat))
    invalidate_();

  // if the engergy needs to be reported, do so
  if (report_energy_)
    store_energy_(globdat);

  return OK;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> LeapFrogModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<Self>(name);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void LeapFrogModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &LeapFrogModule::makeNew);
}
