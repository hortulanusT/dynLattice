#pragma once

#include <jive/fem/declare.h>
#include <jive/model/declare.h>
#include <jive/implict/declare.h>

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
  jive::fem     ::declareMBuilders();
  jive::model   ::declareModels   ();
  jive::implict ::declareModels   ();

  specialCosseratRodModel::declare();
  rodJointModel::declare          ();
  periodicBCModel::declare        ();
  FollowerLoadModel::declare      ();
  DirichletModel::declare         ();
  NeumannModel::declare           ();
}