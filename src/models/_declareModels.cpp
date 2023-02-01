#include "models/_declareModels.h"

//-----------------------------------------------------------------------
//   declareModels
//-----------------------------------------------------------------------

void declareModels()
{
  jive::fem ::declareMBuilders();
  jive::model ::declareModels();
  jive::implict ::declareModels();

  specialCosseratRodModel::declare();
  rodJointModel::declare();
  periodicBCModel::declare();
  FollowerLoadModel::declare();
  DirichletModel::declare();
  NeumannModel::declare();
  LatticeModel::declare();
  InitLoadModel::declare();
  ImpactModel::declare();
}