#pragma once

#include <jive/fem/declare.h>
#include <jive/implict/declare.h>
#include <jive/model/declare.h>

#include "models/DirichletModel.h"
#include "models/FollowerLoadModel.h"
#include "models/ImpactModel.h"
#include "models/InitLoadModel.h"
#include "models/LatticeModel.h"
#include "models/NeumannModel.h"
#include "models/PeriodicBCModel.h"
#include "models/hingeModel.h"
#include "models/specialCosseratRodModel.h"

//-----------------------------------------------------------------------
//   declareModels
//-----------------------------------------------------------------------

void declareModels();
