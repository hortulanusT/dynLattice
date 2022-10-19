#include <jive/app/Application.h>

#include "misc/_declare.h"
#include "models/_declare.h"
#include "modules/_declare.h"

//-----------------------------------------------------------------------
//   mainModule
//-----------------------------------------------------------------------

using jem::newInstance;
using jem::Ref;
using jive::app::Module;

Ref<Module> mainModule()
{
  using jive::app::ChainModule;
  using jive::app::ControlModule;
  using jive::app::InfoModule;
  using jive::app::ReportModule;
  using jive::app::UserconfModule;
  using jive::fem::InitModule;
  using jive::fem::InputModule;
  using jive::fem::ShapeModule;

  // Declare everything, that is needed for the models
  declareModels();

  // Declare everything, that is needed for the modules
  declareModules();

  // Declare everything, that is needed for the timesteppers
  declareTimeSteppers();

  // Set up the module chain. These modules will be called by Jive in
  // the order that they have been added to the chain.
  Ref<ChainModule> chain = newInstance<ChainModule>();

  // Git Report Module: Report the current status of the git repo
  chain->pushBack(newInstance<GitReportModule>());

  // UserConf: reads user input
  chain->pushBack(newInstance<UserconfModule>("Input"));

  // Shepe Module: Stores all the shapes needed by other modules
  chain->pushBack(newInstance<ShapeModule>());

  // Init Module: creates the main model and initilazies it
  chain->pushBack(newInstance<InitModule>());

  // Info Module: prints information about the current calculation
  chain->pushBack(newInstance<InfoModule>());

  // UserConf: Specify the solver by the user
  chain->pushBack(newInstance<UserconfModule>("Solver"));

  // UserConf: Specify the output
  chain->pushBack(newInstance<UserconfModule>("Output"));

  // ControlModule: controll the iterations
  chain->pushBack(newInstance<ControlModule>());

  // ReportModule: wrapper around the chain to print infos
  return newInstance<ReportModule>("report", chain);
}

//-----------------------------------------------------------------------
//   main
//-----------------------------------------------------------------------

int main(int argc, char **argv)
{
  using jive::app::Application;

  return Application::exec(argc, argv, &mainModule);
}