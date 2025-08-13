/**
 * @file _declareModules.cpp
 * @brief Implementation of module registration for dynLattice
 *
 * Registers all custom modules with the JIVE framework to make them available
 * for use in input files and configuration. Includes time integration modules,
 * I/O modules, and specialized analysis modules.
 */

#include "modules/_declareModules.h"

//-----------------------------------------------------------------------
//   declareModules
//-----------------------------------------------------------------------

void declareModules()
{
  // Register JIVE framework modules
  jive::app::declareModules();     // Application framework modules
  jive::implict::declareModules(); // Implicit solver modules
  jive::gl::declareModules();      // Graphics/visualization modules
  jive::fem::declareModules();     // Finite element modules

  // Register I/O and mesh modules
  GMSHInputModule::declare(); // GMSH mesh input
  ParaViewModule::declare();  // ParaView VTK output
  CSVOutputModule::declare(); // CSV data output
  GitReportModule::declare(); // Git repository reporting

  // Register group management modules
  GroupInputModule::declare();     // Generic node/element group input
  GroupOutputModule::declare();    // Generic group data output
  PBCGroupInputModule::declare();  // Periodic BC group generation
  PBCGroupOutputModule::declare(); // Periodic BC data output

  // Register force and analysis modules
  ForceOutputModule::declare();   // Force extraction and output
  TangentOutputModule::declare(); // Tangent stiffness homogenization

  // Register time integration modules
  LeapFrogModule::declare();      // Leap-frog explicit integration
  MilneDeviceModule::declare();   // Milne predictor-corrector method
  EmbeddedRKModule::declare();    // Embedded Runge-Kutta methods
  AdaptiveStepModule::declare();  // Adaptive time stepping
  LenientNonlinModule::declare(); // Lenient nonlinear solver
}
