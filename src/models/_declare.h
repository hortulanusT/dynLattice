#pragma once

#include <jive/fem/declare.h>
#include <jive/model/declare.h>
#include <jive/implict/declare.h>

#include "models/specialCosseratRodModel.h"
#include "models/rodJointModel.h"
#include "models/DirichletModel.h"
#include "models/NeumannModel.h"
#include "models/PeriodicBCModel.h"
#include "models/FollowerLoadModel.h"
#include "models/LatticeModel.h"
#include "models/InitLoadModel.h"
#include "models/ImpactModel.h"

//-----------------------------------------------------------------------
//   declareModels
//-----------------------------------------------------------------------

void declareModels ()
{
  jive::fem     ::declareMBuilders();
  jive::model   ::declareModels   ();
  jive::implict ::declareModels   ();

  specialCosseratRodModel::declare();
  rodJointModel::declare          ();
  periodicBCModel::declare        ();
  FollowerLoadModel::declare      ();
  DirichletModel::declare         ();
  NeumannModel::declare           ();
  LatticeModel::declare           ();
  InitLoadModel::declare          ();
  ImpactModel::declare            ();
}