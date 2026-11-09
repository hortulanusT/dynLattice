/**
 * @file LeapFrogModule.cpp
 * @author Til Gärtner
 * @brief Implementation of leap-frog time integration algorithm
 *
 * This module implements the leap-frog time integration scheme for explicit
 * dynamic analysis. The method uses staggered time stepping with second-order
 * accuracy and symplectic properties for energy conservation.
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

LeapFrogModule::LeapFrogModule(const String &name) : Super(name), firstStep_(true)
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

  // Get the state vectors from the last time step (velocities actually at
  // half steps!)
  StateVector::get(u_old, STATE[0], dofs_, globdat);
  StateVector::get(v_old, STATE[1], dofs_, globdat);
  StateVector::get(a_old, STATE[2], dofs_, globdat);
  params.clear();

  // Compute new accelerations
  fres = getForce(fint, fext, globdat);
  getAcce(a_new, cons_, fres, globdat);

  // update velocity; the initial velocity is at t = 0, so the first step of this module is a half kick to t = dt / 2
  ABupdate(dv, a_new);
  if (firstStep_)
    dv *= 0.5;
  updateVec(v_new, v_old, dv);

  // update position
  ABupdate(du, v_new);
  updateVec(u_new, u_old, du, true);

  // store the new state
  StateVector::store(u_new, STATE[0], dofs_, globdat);
  StateVector::store(v_new, STATE[1], dofs_, globdat);

  // no error estimate: the step is fixed (see init)
  info.set(SolverInfo::RESIDUAL, 0.);
}

//-----------------------------------------------------------------------
//   init, configure
//-----------------------------------------------------------------------

Module::Status LeapFrogModule::init(const Properties &conf, const Properties &props, const Properties &globdat)
{
  Status status = Super::init(conf, props, globdat);
  firstStep_ = true;

  // without an error estimate the adaptive control would grow the step every step: keep deltaTime
  minDtime_ = dtime_;
  maxDtime_ = dtime_;
  getConfig(conf, globdat);

  return status;
}

void LeapFrogModule::configure(const Properties &props, const Properties &globdat)
{
  Super::configure(props, globdat);
  minDtime_ = dtime_;
  maxDtime_ = dtime_;
}

//-----------------------------------------------------------------------
//   commit
//-----------------------------------------------------------------------

bool LeapFrogModule::commit(const Properties &globdat)
{
  // a step rejected by the model is solved again, still as the first one
  const bool accepted = Super::commit(globdat);
  if (accepted)
    firstStep_ = false;
  return accepted;
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
