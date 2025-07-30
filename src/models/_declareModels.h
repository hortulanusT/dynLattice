/**
 * @file _declareModels.h
 * @author Til Gärtner
 * @brief Model factory registration declarations
 */

#pragma once

#include <jive/fem/declare.h>
#include <jive/implict/declare.h>
#include <jive/model/declare.h>

#include "models/DirichletModel.h"
#include "models/FollowerLoadModel.h"
#include "models/HingeModel.h"
#include "models/ImpactModel.h"
#include "models/InitLoadModel.h"
#include "models/JointContactModel.h"
#include "models/LatticeModel.h"
#include "models/MassModel.h"
#include "models/NeumannModel.h"
#include "models/PeriodicBCModel.h"
#include "models/RodContactModel.h"
#include "models/SpecialCosseratRodModel.h"
#include "models/SpringMassModel.h"
#include "models/SymBCModel.h"

//-----------------------------------------------------------------------
//   declareModels
//-----------------------------------------------------------------------

/// @brief Register all model types with their respective factories
/// @details This function registers all available model types in the dynLattice
/// framework with the ModelFactory system, enabling their creation through
/// configuration files. Must be called during application initialization.
void declareModels();
