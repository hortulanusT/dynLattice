/**
 * @file _declareModels.cpp
 * @author Til Gärtner
 * @brief Implementation of model factory registration
 */

#include "models/_declareModels.h"

//-----------------------------------------------------------------------
//   declareModels
//-----------------------------------------------------------------------

void declareModels()
{
  // Declare base JIVE framework components
  jive::fem ::declareMBuilders();
  jive::model ::declareModels();
  jive::implict ::declareModels();

  // Declare dynLattice-specific models
  SpecialCosseratRodModel::declare(); // Geometrically nonlinear rod elements
  HingeModel::declare();              // Rigid hinge joints
  PeriodicBCModel::declare();         // Periodic boundary conditions
  FollowerLoadModel::declare();       // Direction-dependent loading
  DirichletModel::declare();          // Displacement boundary conditions
  NeumannModel::declare();            // Force boundary conditions
  LatticeModel::declare();            // Lattice structure models
  InitLoadModel::declare();           // Initial loading conditions
  ImpactModel::declare();             // Impact and contact mechanics
  RodContactModel::declare();         // Rod-to-rod contact
  JointContactModel::declare();       // Joint contact mechanics
  MassModel::declare();               // Mass and inertia models
  SpringMassModel::declare();         // Spring-mass systems
  SymBCModel::declare();              // Symmetry boundary conditions
}
