/**
 * @file LeapFrogModule.cpp
 * @author Til Gärtner
 * @brief Implementation of leap-frog time integration algorithm
 *
 * Leap-frog in its velocity-Verlet form: all state vectors at the full step.
 */

#include "modules/LeapFrogModule.h"
#include "utils/testing.h"

#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(LeapFrogModule);

//=======================================================================
//   class LeapFrogModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *LeapFrogModule::TYPE_NAME = "LeapFrog";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

LeapFrogModule::LeapFrogModule(const String &name) : Super(name), accValid_(false)
{
}

LeapFrogModule::~LeapFrogModule()
{
}

//-----------------------------------------------------------------------
//   solve
//-----------------------------------------------------------------------

void LeapFrogModule::solve(const Properties &info, const Properties &globdat)

{
  using jive::model::STATE;
  const idx_t dofCount = dofs_->dofCount();

  Vector fint(dofCount);
  Vector fext(dofCount);
  Vector fres(dofCount);
  Vector u_old, v_old;
  Vector a_old(dofCount);
  Vector dv(dofCount);
  Vector du(dofCount);
  Vector u_new(dofCount);
  Vector v_half(dofCount);
  Vector v_new(dofCount);
  Vector a_new(dofCount);

  // the state of the last step, all at t_n
  StateVector::get(u_old, STATE[0], dofs_, globdat);
  StateVector::get(v_old, STATE[1], dofs_, globdat);

  // a_n from the end of the last step; on the first step and after a cancel or a dof change from u_n at t_n
  if (accValid_)
    StateVector::get(a_old, STATE[2], dofs_, globdat);
  else
  {
    fres = getStartForce_(fint, fext, globdat);
    getAcce(a_old, cons_, fres, globdat);
  }

  // kick: v_{n+1/2} = v_n + dt/2 a_n
  dv = 0.5 * dtime_ * a_old;
  updateVec(v_half, v_old, dv);

  // drift: exp(u_{n+1}) = exp(dt v_{n+1/2}) exp(u_n) for the rotations
  du = dtime_ * v_half;
  updateVec(u_new, u_old, du, true);

  // the gyroscopic force at the end needs v_{n+1}, which needs a_{n+1}: predicted with a_n (second order)
  updateVec(v_new, v_half, dv);
  StateVector::store(u_new, STATE[0], dofs_, globdat);
  StateVector::store(v_new, STATE[1], dofs_, globdat);

  // the rotational inertia at u_{n+1}; it serves the next step too
  if (updMassStep_)
  {
    updateMass_(globdat);
    massAtState_ = true;
  }

  // a_{n+1} at u_{n+1}, t_{n+1}
  fres = getEndForce_(fint, fext, globdat);
  getAcce(a_new, cons_, fres, globdat);

  // kick: v_{n+1} = v_{n+1/2} + dt/2 a_{n+1}
  dv = 0.5 * dtime_ * a_new;
  updateVec(v_new, v_half, dv);

  StateVector::store(v_new, STATE[1], dofs_, globdat);
  StateVector::store(a_new, STATE[2], dofs_, globdat);
  accValid_ = true;

  // no error estimate: the step is fixed (see configure)
  info.set(SolverInfo::RESIDUAL, 0.);
}

//-----------------------------------------------------------------------
//   init, configure
//-----------------------------------------------------------------------

Module::Status LeapFrogModule::init(const Properties &conf, const Properties &props, const Properties &globdat)
{
  Status status = Super::init(conf, props, globdat);
  accValid_ = false;
  jem::util::connect(dofs_->newSizeEvent, this, &Self::invalidateAcce_);
  jem::util::connect(dofs_->newOrderEvent, this, &Self::invalidateAcce_);

  return status;
}

void LeapFrogModule::configure(const Properties &props, const Properties &globdat)
{
  // also called by init. Without an error estimate the adaptive control would grow the step every step:
  // keep deltaTime
  Super::configure(props, globdat);
  minDtime_ = dtime_;
  maxDtime_ = dtime_;
}

//-----------------------------------------------------------------------
//   cancel, invalidateAcce_
//-----------------------------------------------------------------------

void LeapFrogModule::cancel(const Properties &globdat)
{
  // the restored acceleration may not belong to the restored state (first step): evaluate it again
  Super::cancel(globdat);
  accValid_ = false;
}

void LeapFrogModule::invalidateAcce_()
{
  accValid_ = false;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> LeapFrogModule::makeNew(const String &name, const Properties &conf,
                                    const Properties &props, const Properties &globdat)

{
  (void)conf;    // unused
  (void)props;   // unused
  (void)globdat; // unused

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
