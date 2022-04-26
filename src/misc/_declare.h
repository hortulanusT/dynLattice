#pragma once

#include "misc/rotationStepper.h"

//-----------------------------------------------------------------------
//   declareTimeSteppers
//-----------------------------------------------------------------------

void declareTimeSteppers ()
{
  rotationStepper::declare();
}