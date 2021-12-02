#pragma once

//-----------------------------------------------------------------------
//    include Statements
//-----------------------------------------------------------------------

#include <jive/app/ChainModule.h>
#include <jive/app/InfoModule.h>
#include <jive/app/UserconfModule.h>
#include <jive/app/ControlModule.h>
#include <jive/app/ReportModule.h>
#include <jive/app/OutputModule.h>
#include <jive/app/declare.h>
#include <jive/implict/declare.h>
#include <jive/fem/InputModule.h>
#include <jive/fem/ShapeModule.h>
#include <jive/fem/InitModule.h>
#include <jive/gl/declare.h>

#include "ParaViewModule.h"
#include "GroupInputModule.h"
#include "LoadExtentModule.h"

//-----------------------------------------------------------------------
//   declareModules
//-----------------------------------------------------------------------

void declareModules ()
{
  jive::app     ::declareModules ();
  jive::implict ::declareModules ();
  jive::gl      ::declareModules ();
  jive::fem     ::declareModules ();
  ParaViewModule::declare        ();
  LoadExtentModule::declare      ();
  declareGroupInputModule        ();
}