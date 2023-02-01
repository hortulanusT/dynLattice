#include "modules/_declareModules.h"

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
  LeapFrogModule::declare();
  ForceOutputModule::declare();
  CSVOutputModule::declare();
  GitReportModule::declare();
  TangentOutputModule::declare();
  CorrectorAdaptModule::declare();
}