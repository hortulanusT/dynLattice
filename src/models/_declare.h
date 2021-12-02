#pragma once

#include <jive/fem/declare.h>
#include <jive/model/declare.h>

#include "specialCosseratRodModel.h"
#include "rodJointModel.h"
#include "DirichletModel.h"
#include "NeumannModel.h"
#include "PeriodicBCModel.h"
#include "FollowerLoadModel.h"

//-----------------------------------------------------------------------
//   declareModels
//-----------------------------------------------------------------------

void declareModels ()
{
  jive::fem   ::declareMBuilders  ();
  jive::model ::declareModels     ();
  specialCosseratRodModel::declare();
  rodJointModel::declare          ();
  periodicBCModel::declare        ();
  FollowerLoadModel::declare      ();
  declareDirichletModel           ();
  declareNeumannModel             ();
}