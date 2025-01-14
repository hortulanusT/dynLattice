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

#ifndef LENIENT_NONLIN_MODULE_H
#define LENIENT_NONLIN_MODULE_H

#include <jem/util/Properties.h>

#include <jive/implict/Names.h>
#include <jive/implict/NonlinModule.h>
#include <jive/model/Model.h>

using jem::idx_t;
using jem::Ref;
using jem::String;

using jem::util::Properties;
using jive::app::Module;
using jive::implict::NonlinModule;
using jive::implict::PropNames;
using jive::model::Model;

//-----------------------------------------------------------------------
//   class NonlinModule
//-----------------------------------------------------------------------

class LenientNonlinModule : public NonlinModule
{
public:
  typedef LenientNonlinModule Self;
  typedef NonlinModule Super;

  static const char *TYPE_NAME;

  // public member functions

  explicit LenientNonlinModule

      (const String &name = "lenient");

  static Ref<Module> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  static void declare();

  virtual Status init

      (const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  virtual Status run

      (const Properties &globdat);

  virtual void configure

      (const Properties &props,
       const Properties &globdat);

  virtual void getConfig

      (const Properties &conf,
       const Properties &globdat) const;

protected:
  virtual ~LenientNonlinModule();

private:
  idx_t maxIter_;
};

#endif
