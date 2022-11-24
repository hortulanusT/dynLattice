/**
 * @file TangentOutputModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Module, that calculates the tangent linear elastic equivalent
 * properties for the material
 * @version 0.1
 * @date 2022-11-21
 *
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 *
 */

#include "modules/TangentOutputModule.h"

#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(TangentOutputModule);

const char *TangentOutputModule::TYPE_NAME = "TangentOutput";

TangentOutputModule::TangentOutputModule(const String &name) : Super(name)
{
  sampleCond_ = FuncUtils::newCond();
  thickness_ = 1.;
  perturb_ = 1e-4;
  rank_ = 0;
}

TangentOutputModule::~TangentOutputModule()
{
}

Module::Status TangentOutputModule::init(const Properties &conf,
                                         const Properties &props,
                                         const Properties &globdat)
{
  Properties params, sparams;
  Ref<AbstractMatrix> stiff;

  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);
  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());

  cons_ = Constraints::get(dofs, globdat);
  masterModel_ = Model::get(globdat, getContext());

  // Output Condition
  FuncUtils::configCond(sampleCond_, jive::app::PropNames::SAMPLE_COND,
                        myProps, globdat);
  FuncUtils::getConfig(myConf, sampleCond_,
                       jive::app::PropNames::SAMPLE_COND);

  // setup Solver
  jive::solver::declareSolvers();

  params.set(jive::model::ActionParams::CONSTRAINTS, cons_);
  masterModel_->takeAction(jive::model::Actions::NEW_MATRIX0, params,
                           globdat);
  params.get(stiff, jive::model::ActionParams::MATRIX0);
  params.clear();

  sparams = newSolverParams(globdat, stiff, nullptr, dofs);
  masterModel_->takeAction(Actions::GET_SOLVER_PARAMS, sparams, globdat);
  solver_ = newSolver("solver", myConf, myProps, sparams, globdat);
  solver_->configure(myProps);
  solver_->getConfig(myConf);

  // setup pbcModels
  StringVector dof_names, rdof_names;
  Properties pbcProp, lockProp;
  myProps.get(dof_names, periodicBCModel::DOF_NAMES_PROP);
  myProps.get(rdof_names, periodicBCModel::ROT_NAMES_PROP);
  rank_ = dof_names.size();
  pbcModels_.resize(rank_ * rank_);

  for (idx_t i = 0; i < dof_names.size(); i++)
    for (idx_t j = 0; j < dof_names.size(); j++)
    {
      String modelName = String::format("model%i%i", i + 1, j + 1);

      pbcProp = myProps.makeProps(modelName);
      pbcProp.set(jive::model::ModelFactory::TYPE_PROP,
                  periodicBCModel::TYPE_NAME);
      pbcProp.set(periodicBCModel::MODE_PROP, "upd");
      pbcProp.set(periodicBCModel::DOF_NAMES_PROP, dof_names);
      pbcProp.set(periodicBCModel::ROT_NAMES_PROP, rdof_names);
      pbcProp.set(String::format("H%i%i", i + 1, j + 1), 1.);

      pbcModels_[i * dof_names.size() + j] =
          jem::staticCast<periodicBCModel>(
              jive::model::ModelFactory::newInstance(modelName, myConf,
                                                     myProps, globdat));
    }

  StringVector lockDofs;
  if (myProps.find(lockDofs, "lockModel.dofs"))
  {
    StringVector groups(lockDofs.size());
    Vector factors(lockDofs.size());
    groups = "all";
    factors = 0;
    lockProp = myProps.makeProps("lockModel");
    lockProp.set(jive::model::ModelFactory::TYPE_PROP,
                 DirichletModel::TYPE_NAME);
    lockProp.set(DirichletModel::DISP_INCR_PROP, 0.);
    lockProp.set(DirichletModel::FACTORS_PROP, factors);
    lockProp.set(DirichletModel::NODES_PROP, groups);
    lockModel_ = jem::staticCast<DirichletModel>(
        jive::model::ModelFactory::newInstance("lockModel", myConf,
                                               myProps, globdat));
    lockModel_->configure(myProps, globdat);
    lockModel_->getConfig(myConf, globdat);
    lockModel_->takeAction(jive::model::Actions::INIT, params, globdat);
  }

  // setup group output model
  Properties groupOutProp = myProps.makeProps("groupUpdate");
  groupOutProp.set(jive::model::ModelFactory::TYPE_PROP,
                   GroupOutputModule::TYPE_NAME);
  groupOutProp.set("elementGroups", "all");
  StringVector edges(6);
  for (idx_t i = 0; i < 6; i++)
    edges[i] = PBCGroupInputModule::EDGES[i];
  groupOutProp.set("nodeGroups", edges);
  groupOutProp.set("dofs", dof_names);
  groupUpdate_ = jem::staticCast<GroupOutputModule>(
      jive::app::ModuleFactory::newInstance("groupUpdate", myConf,
                                            myProps, globdat));
  groupUpdate_->init(myConf, myProps, globdat);

  // get strain vectors
  strains_.resize(rank_ * rank_);
  stresses_.resize(rank_ * rank_);

  strains_ = PBCGroupOutputModule::getDataSets(rank_, true, false);
  stresses_ = PBCGroupOutputModule::getDataSets(rank_, false, true);

  myConf.set("strainMeasures", strains_);
  myConf.set("stressMeasures", stresses_);

  // get the thickness of the material (if less than 3 dofs)
  if (dof_names.size() < 3)
  {
    myProps.find(thickness_, "thickness", 0., Float::MAX_VALUE);
    myConf.set("thickness", thickness_);
  }

  // get the perturbatio amount
  myProps.find(perturb_, "perturbation", 0., 1.);
  myConf.set("perturb", perturb_);

  return OK;
}

void TangentOutputModule::shutdown(const Properties &globdat)
{
}

Module::Status TangentOutputModule::run(const Properties &globdat)
{
  if (!FuncUtils::evalCond(*sampleCond_, globdat))
    return OK;

  jem::System::info(myName_) << " ...Calculating tangent properties\n";

  using jive::model::ActionParams;
  using jive::model::Actions;

  jive::SparseMatrix globalConstraints;
  Vector globalU(cons_->dofCount());
  Matrix stresses(rank_ * rank_, rank_ * rank_);
  Matrix strains(rank_ * rank_, rank_ * rank_);

  globalConstraints = cons_->toMatrix();
  StateVector::get(globalU, cons_->getDofSpace(), globdat);

  getStrainStress_(strains, stresses, globdat);
  storeTangentProps_(strains, stresses, globdat);

  cons_->setConstraints(globalConstraints);
  StateVector::store(globalU, cons_->getDofSpace(), globdat);

  return OK;
}

void TangentOutputModule::getStrainStress_(const Matrix &strains,
                                           const Matrix &stresses,
                                           const Properties &globdat)
{
  Properties params;
  Vector fint(cons_->dofCount());
  Vector u(cons_->dofCount());
  Vector strains0(rank_ * rank_);

  fint = 0.;
  strains = 0.;
  stresses = 0.;

  params.set(ActionParams::INT_VECTOR, fint);
  masterModel_->takeAction(Actions::GET_INT_VECTOR, params, globdat);
  params.clear();

  groupUpdate_->run(globdat);
  for (idx_t iComp = 0; iComp < rank_ * rank_; iComp++)
    strains0[iComp] = FuncUtils::evalExpr(strains_[iComp], globdat);

  // TEST_CONTEXT(strains0)
  // TEST_PRINTER((*cons_))

  for (idx_t iPBC = 0; iPBC < rank_ * rank_; iPBC++)
    for (double dir : Vector({-1., 1.}))
    {
      cons_->clear();

      params.set(ActionParams::SCALE_FACTOR, dir * .5 * perturb_);
      params.set(periodicBCModel::CURRENTGRAD_PARAM, strains0);

      pbcModels_[iPBC]->takeAction(Actions::INIT, params, globdat);
      pbcModels_[iPBC]->takeAction(Actions::GET_CONSTRAINTS, params,
                                   globdat);
      // TEST_PRINTER((*cons_))
      lockModel_->takeAction(Actions::INIT, params, globdat);
      lockModel_->takeAction(Actions::GET_CONSTRAINTS, params, globdat);

      params.erase(ActionParams::SCALE_FACTOR);

      solver_->solve(u, Vector(-1. * fint));
      StateVector::store(u, cons_->getDofSpace(), globdat);
      groupUpdate_->run(globdat);

      for (idx_t iComp = 0; iComp < rank_ * rank_; iComp++)
      {
        strains[iPBC][iComp] +=
            dir * FuncUtils::evalExpr(strains_[iComp], globdat);
        stresses[iPBC][iComp] +=
            dir * FuncUtils::evalExpr(stresses_[iComp], globdat);
      }
    }

  // TEST_CONTEXT(strains)
  // TEST_CONTEXT(stresses)
}

void TangentOutputModule::storeTangentProps_(const Matrix &strains,
                                             const Matrix &stresses,
                                             const Properties &globdat)
{
  Properties myVars = Globdat::getVariables("tangentModuli", globdat);

  switch (strains.size(0))
  {
  case 4:
    myVars.set("G_xy", stresses[1][1] / strains[1][1] / thickness_);
    myVars.set("G_yx", stresses[2][2] / strains[2][2] / thickness_);

    myVars.set("nu_xy", stresses[3][0] / stresses[3][3]);
    myVars.set("nu_yx", stresses[0][3] / stresses[0][0]);

    myVars.set("E_x", (stresses[0][0] -
                       stresses[3][0] * stresses[0][3] / stresses[3][3]) /
                          strains[0][0] / thickness_);
    myVars.set("E_y", (stresses[3][3] -
                       stresses[0][3] * stresses[3][0] / stresses[0][0]) /
                          strains[3][3] / thickness_);
    break;

  case 1:
    myVars.set("E", stresses(0, 0) / strains(0, 0));
    break;

  default:
    NOT_IMPLEMENTED
    break;
  }

  TEST_CONTEXT(strains)
  TEST_CONTEXT(stresses)
  TEST_CONTEXT(myVars)
}

Ref<Module> TangentOutputModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return jem::newInstance<Self>(name);
}

void TangentOutputModule::declare()
{
  ModuleFactory::declare(TYPE_NAME, &makeNew);
  ModuleFactory::declare(CLASS_NAME, &makeNew);
}