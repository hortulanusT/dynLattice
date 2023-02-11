#pragma once

//-----------------------------------------------------------------------
//    include Statements
//-----------------------------------------------------------------------

#include <jive/app/declare.h>
#include <jive/fem/declare.h>
#include <jive/gl/declare.h>
#include <jive/implict/declare.h>

#include "modules/CSVOutputModule.h"
#include "modules/EmbeddedRKModule.h"
#include "modules/ForceOutputModule.h"
#include "modules/GitReportModule.h"
#include "modules/GroupInputModule.h"
#include "modules/GroupOutputModule.h"
#include "modules/LeapFrogModule.h"
#include "modules/MilneDeviceModule.h"
#include "modules/PBCGroupInputModule.h"
#include "modules/PBCGroupOutputModule.h"
#include "modules/TangentOutputModule.h"
#include "modules/gmshInput/GMSHInputModule.h"
#include "modules/paraview/ParaViewModule.h"

//-----------------------------------------------------------------------
//   declareModules
//-----------------------------------------------------------------------

void declareModules();