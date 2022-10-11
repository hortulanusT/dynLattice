#pragma once

//-----------------------------------------------------------------------
//    include Statements
//-----------------------------------------------------------------------

#include <jive/app/ChainModule.h>
#include <jive/app/ControlModule.h>
#include <jive/app/InfoModule.h>
#include <jive/app/OutputModule.h>
#include <jive/app/ReportModule.h>
#include <jive/app/UserconfModule.h>
#include <jive/app/declare.h>
#include <jive/fem/InitModule.h>
#include <jive/fem/InputModule.h>
#include <jive/fem/ShapeModule.h>
#include <jive/gl/declare.h>
#include <jive/implict/declare.h>

#include "modules/CSVOutputModule.h"
#include "modules/ExplicitModule.h"
#include "modules/ForceOutputModule.h"
#include "modules/GitReportModule.h"
#include "modules/GroupInputModule.h"
#include "modules/GroupOutputModule.h"
#include "modules/PBCGroupInputModule.h"
#include "modules/PBCGroupOutputModule.h"
#include "modules/gmshInput/GMSHInputModule.h"
#include "modules/paraview/ParaViewModule.h"

//-----------------------------------------------------------------------
//   declareModules
//-----------------------------------------------------------------------

void declareModules()
{
  jive::app ::declareModules();
  jive::implict ::declareModules();
  jive::gl ::declareModules();
  jive::fem ::declareModules();
  GMSHInputModule::declare();
  ParaViewModule::declare();
  GroupOutputModule::declare();
  PBCGroupOutputModule::declare();
  GroupInputModule::declare();
  PBCGroupInputModule::declare();
  ExplicitModule::declare();
  ForceOutputModule::declare();
  CSVOutputModule::declare();
  GitReportModule::declare();
}