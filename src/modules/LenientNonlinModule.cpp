/**
 * @file LenientNonlinModule.cpp
 * @author F.P. van der Meer, T. Gärtner
 * @brief Implementation of lenient nonlinear solver module
 *
 * This module implements a nonlinear solver that can accept non-converged
 * solutions when a maximum iteration limit is reached. It extends the standard
 * nonlinear solver with relaxed convergence criteria for difficult problems.
 *
 * Original implementation: F.P. van der Meer, April 2014
 * Modified: T. Gärtner, January 2025
 * - Accept solution if not converged directly
 * - Check maximum iteration limit before accepting
 */

#include <jem/base/ClassTemplate.h>
#include <jem/base/Exception.h>
#include <jem/base/System.h>
#include <jive/app/ModuleFactory.h>
#include <jive/implict/SolverInfo.h>

#include "LenientNonlinModule.h"

using jem::newInstance;
using jem::slice;
using jive::implict::SolverInfo;

//=======================================================================
//   class LenientNonlinModule
//=======================================================================

JEM_DEFINE_CLASS(LenientNonlinModule);

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *LenientNonlinModule::TYPE_NAME = "LNonlin";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

LenientNonlinModule::LenientNonlinModule(const String &name) : Super(name)

{
}

LenientNonlinModule::~LenientNonlinModule()

{
}

//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------

Module::Status LenientNonlinModule::init

    (const Properties &conf,
     const Properties &props,
     const Properties &globdat)

{
  Module::Status status = Super::init(conf, props, globdat);
  conf.getProps(myName_).get(maxIter_, jive::implict::PropNames::MAX_ITER);

  return status;
}

//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------

Module::Status LenientNonlinModule::run

    (const Properties &globdat)

{
  using jem::System;
  idx_t iter = 0;

  Properties info = SolverInfo::get(globdat);

  Status ret = OK;

  while (true)
  {
    info.clear();

    advance(globdat);

    try
    {
      solve(info, globdat);
    }
    catch (const jem::Exception &ex)
    {
      info.find(iter, SolverInfo::ITER_COUNT);

      if (ex.what()[slice(0, 14)].equals("no convergence") && iter >= maxIter_)
      {
        System::info(myName_) << " ...accepting non-converged solution after " << iter << "/" << maxIter_ << " iterations\n";
        info.set(SolverInfo::CONVERGED, true);
      }
      else
      {
        cancel(globdat);
        throw;
      }
    }

    if (commit(globdat))
    {
      break;
    }

    print(System::info(myName_),
          "Solution rejected; re-trying\n");

    cancel(globdat);
  }

  return ret;
}

//-----------------------------------------------------------------------
//   configure()
//-----------------------------------------------------------------------

void LenientNonlinModule::configure

    (const Properties &props,
     const Properties &globdat)

{
  Super::configure(props, globdat);
}

//-----------------------------------------------------------------------
//   getConfig()
//-----------------------------------------------------------------------

void LenientNonlinModule::getConfig

    (const Properties &conf,
     const Properties &globdat)

{
  Super::getConfig(conf, globdat);
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> LenientNonlinModule::makeNew

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)

{
  return newInstance<Self>(name);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void LenientNonlinModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &makeNew);
}
