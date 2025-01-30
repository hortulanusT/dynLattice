/*
 *
 *  Copyright (C) 2014 TU Delft. All rights reserved.
 *
 *  F.P. van der Meer, April 2014
 *
 *  Iterative solver that optionally continues with non-converged solution
 *
 *  Changes January 2025: (T. Gärtner)
 *  - just accept the solution if its not converged directly
 *  - check if the maximum amount of iterations have been reached
 *
 */

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
  maxIter_ = jem::maxOf<idx_t>();

  return Super::init(conf, props, globdat);
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

  conf.getProps(myName_).get(maxIter_, jive::implict::PropNames::MAX_ITER);
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
