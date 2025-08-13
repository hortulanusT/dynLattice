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

LeapFrogModule::LeapFrogModule(const String &name) : Super(name)
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

  globdat.get(step, Globdat::TIME_STEP);

  // Get the state vectors from the last time step (velocities actually at
  // half steps!)
  StateVector::get(u_old, STATE[0], dofs_, globdat);
  StateVector::get(v_old, STATE[1], dofs_, globdat);
  StateVector::get(a_old, STATE[2], dofs_, globdat);
  params.clear();

  // Compute new accelerations
  fres = getForce(fint, fext, globdat);
  getAcce(a_new, cons_, fres, globdat);

  // update velocity
  ABupdate(dv, a_new);
  updateVec(v_new, v_old, dv);

  // update position
  ABupdate(du, v_new);
  updateVec(u_new, u_old, du, true);

  info.set(SolverInfo::RESIDUAL, 0.);
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> LeapFrogModule::makeNew(const String &name, const Properties &conf,
                                    const Properties &props, const Properties &globdat)

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
