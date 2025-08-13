/**
 * @file _declareModules.h
 * @brief Module registration declarations for dynLattice
 *
 * Declares the function to register all custom modules with the JIVE framework.
 * This includes time integration modules, I/O modules, and specialized analysis
 * modules for dynamic lattice simulations.
 */

#pragma once

//-----------------------------------------------------------------------
//    Include statements
//-----------------------------------------------------------------------

// JIVE framework modules
#include <jive/app/declare.h>
#include <jive/fem/declare.h>
#include <jive/gl/declare.h>
#include <jive/implict/declare.h>

// Time integration modules
#include "modules/AdaptiveStepModule.h"
#include "modules/EmbeddedRKModule.h"
#include "modules/LeapFrogModule.h"
#include "modules/LenientNonlinModule.h"
#include "modules/MilneDeviceModule.h"

// I/O and visualization modules
#include "modules/CSVOutputModule.h"
#include "modules/ForceOutputModule.h"
#include "modules/GMSHInputModule.h"
#include "modules/GitReportModule.h"
#include "modules/ParaViewModule.h"

// Group management modules
#include "modules/GroupInputModule.h"
#include "modules/GroupOutputModule.h"
#include "modules/PBCGroupInputModule.h"
#include "modules/PBCGroupOutputModule.h"

// Analysis modules
#include "modules/TangentOutputModule.h"

//-----------------------------------------------------------------------
//   Module registration function
//-----------------------------------------------------------------------

/// @brief Register all dynLattice modules with the JIVE framework
/// @details Calls declare() for all custom modules to make them available
/// for instantiation through the ModuleFactory system
void declareModules();
