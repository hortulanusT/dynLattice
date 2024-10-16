#pragma once

#include <jive/fem/declare.h>
#include <jive/implict/declare.h>
#include <jive/model/declare.h>

#include "models/DirichletModel.h"
#include "models/FollowerLoadModel.h"
#include "models/HingeModel.h"
#include "models/ImpactModel.h"
#include "models/InitLoadModel.h"
#include "models/LatticeModel.h"
#include "models/MassModel.h"
#include "models/NeumannModel.h"
#include "models/PeriodicBCModel.h"
#include "models/RodContactModel.h"
#include "models/SpringMassModel.h"
#include "models/specialCosseratRodModel.h"

//-----------------------------------------------------------------------
//   declareModels
//-----------------------------------------------------------------------

void declareModels();
