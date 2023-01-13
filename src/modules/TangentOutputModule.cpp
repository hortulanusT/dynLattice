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
  rank_ = -1;
}

TangentOutputModule::~TangentOutputModule()
{
}

Module::Status TangentOutputModule::init(const Properties &conf,
                                         const Properties &props,
                                         const Properties &globdat)
{
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  // get the dofs
  StringVector dofs;
  myProps.get(dofs, "dofs");
  rank_ = dofs.size();

  // get the master model
  masterModel_ = Model::get(globdat, getContext());

  // Output Condition
  FuncUtils::configCond(sampleCond_, jive::app::PropNames::SAMPLE_COND,
                        myProps, globdat);
  FuncUtils::getConfig(myConf, sampleCond_,
                       jive::app::PropNames::SAMPLE_COND);

  // get the thickness of the material (for 2D calculations)
  myProps.find(thickness_, "thickness", 0., Float::MAX_VALUE);
  myConf.set("thickness", thickness_);

  // setup group output model
  Properties groupOutProp = myProps.makeProps("groupUpdate");
  groupOutProp.set(jive::app::ModuleFactory::TYPE_PROP,
                   GroupOutputModule::TYPE_NAME);
  groupOutProp.set("elementGroups", "all");
  StringVector edges(rank_ * 2);
  for (idx_t i = 0; i < edges.size(); i++)
    edges[i] = PBCGroupInputModule::EDGES[i];
  groupOutProp.set("nodeGroups", edges);
  groupOutProp.set("dofs", dofs);
  groupUpdate_ = jem::staticCast<GroupOutputModule>(
      jive::app::ModuleFactory::newInstance("groupUpdate", myConf,
                                            myProps, globdat));
  groupUpdate_->init(myConf, myProps, globdat);

  // get strain/stress vectors
  strains_.resize(rank_ * rank_);
  stresses_.resize(rank_ * rank_);
  sizes_.resize(rank_);

  strains_ = PBCGroupOutputModule::getDataSets(rank_, true, false, dofs);
  stresses_ = PBCGroupOutputModule::getDataSets(rank_, false, true, dofs);
  for (idx_t i = 0; i < dofs.size(); i++)
    sizes_[i] = "all.extent." + dofs[i];

  myConf.set("strainMeasures", strains_);
  myConf.set("stressMeasures", stresses_);
  myConf.set("sizeMeasures", sizes_);

  // select the mode for determination of the properties
  // 'finDiff' or 'matCond'
  mode_ = "finDiff";
  myProps.find(mode_, "mode");
  myConf.set("mode", mode_);

  if (mode_ == "finDiff")
  {
    // setup Solver
    // TEST_CONTEXT(myProps)
    if (!myProps.contains("solver"))
    {
      myProps.makeProps("solver").set(
          jive::app::ModuleFactory::TYPE_PROP,
          jive::implict::NonlinModule::TYPE_NAME);
      myProps.getProps("solver").set(jive::implict::PropNames::MAX_ITER,
                                     5);
    }

    solver_ = jive::implict::newSolverModule(myName_ + ".solver", conf,
                                             props, globdat);
    solver_->configure(props, globdat);
    solver_->getConfig(conf, globdat);
    solver_->init(conf, props, globdat);

    // get the perturbation amount
    perturb_ = 1e-9;
    myProps.find(perturb_, "perturb", 0., 1e-2);
    myConf.set("perturb", perturb_);
  }
  else if (mode_ == "matCond")
  {
    cons_ =
        Constraints::get(DofSpace::get(globdat, getContext()), globdat);
    Ref<DofSpace> dofSpace = cons_->getDofSpace();

    strainDofs_.resize(rank_, rank_);
    for (idx_t iCorner = 0; iCorner < rank_; iCorner++)
    {
      idx_t iNode =
          NodeGroup::get(PBCGroupInputModule::CORNERS[iCorner + 1],
                         NodeSet::get(globdat, getContext()), globdat,
                         getContext())
              .getIndex(0);
      for (idx_t iDof = 0; iDof < rank_; iDof++)
        strainDofs_(iDof, iCorner) = dofSpace->getDofIndex(
            iNode, dofSpace->getTypeIndex(dofs[iDof]));
    }
  }

  return OK;
}

void TangentOutputModule::shutdown(const Properties &globdat)
{
}

Module::Status TangentOutputModule::run(const Properties &globdat)
{
  if (!FuncUtils::evalCond(*sampleCond_, globdat))
  {
    Globdat::getVariables(globdat).erase("tangentModuli");
    return OK;
  }
  jem::System::info(myName_)
      << " ...Start calculating tangent properties\n";

  Matrix stresses(rank_ * rank_, rank_ * rank_);
  Matrix strains(rank_ * rank_, rank_ * rank_);

  if (mode_ == "finDiff")
    getStrainStress_(strains, stresses, globdat);
  else if (mode_ == "matCond")
    condenseMatrix_(strains, stresses, globdat);
  else
    throw jem::Error("unknown mode!");

  storeTangentProps_(strains, stresses, globdat);

  jem::System::info(myName_)
      << " ...Done calculating tangent properties\n";

  return OK;
}

void TangentOutputModule::readStrainStress_(const Vector &strains,
                                            const Vector &stresses,
                                            const Properties &globdat)
{
  double size = 1;
  strains = 0.;
  stresses = 0.;

  groupUpdate_->run(globdat);
  for (String sizeMeas : sizes_)
    size *= FuncUtils::evalExpr(sizeMeas, globdat);
  for (idx_t iComp = 0; iComp < rank_ * rank_; iComp++)
    strains[iComp] = FuncUtils::evalExpr(strains_[iComp], globdat);
  for (idx_t iComp = 0; iComp < rank_ * rank_; iComp++)
    stresses[iComp] = FuncUtils::evalExpr(stresses_[iComp], globdat);
}

void TangentOutputModule::readStresses_(const Vector &stresses,
                                        const Vector &fint,
                                        const Properties &globdat)
{
  stresses = 0.;

  globdat.set(ActionParams::INT_VECTOR, fint);
  groupUpdate_->run(globdat);

  for (idx_t iComp = 0; iComp < rank_ * rank_; iComp++)
    stresses[iComp] = FuncUtils::evalExpr(stresses_[iComp], globdat);
}

void TangentOutputModule::reportStrainStress_(const Vector &H,
                                              const Vector &P)
{
  Matrix deformTens(rank_, rank_);
  Matrix engStress(rank_, rank_);
  Matrix engStrain(rank_, rank_);
  double J;

  vec2mat(deformTens, H);
  deformTens += eye(rank_);
  J = jem::numeric::det(deformTens);

  vec2mat(engStrain, H);
  engStrain = matmul(
      engStrain,
      jem::numeric::inverse(deformTens)); // spatial displacement gradient
  vec2mat(engStress, P);
  engStress =
      matmul(engStress, deformTens.transpose()) / J; // chauchy stresses

  System::info() << "### Spatial displacement gradient\n"
                 << engStrain << "\n";
  System::info() << "### Cauchy stress\n" << engStress << "\n";
}

void TangentOutputModule::getStrainStress_(const Matrix &strains,
                                           const Matrix &stresses,
                                           const Properties &globdat)
{
  Properties info;
  Vector strains0(rank_ * rank_);
  Vector stresses0(rank_ * rank_);
  Vector pertubStrains(rank_ * rank_);
  Vector pertubStresses(rank_ * rank_);
  Vector applStrains(rank_ * rank_);

  strains = 0.;
  stresses = 0.;

  readStrainStress_(strains0, stresses0, globdat);
  reportStrainStress_(strains0, stresses0);

  // TEST_CONTEXT(strains0)

  for (idx_t iPBC = 0; iPBC < rank_ * rank_; iPBC++)
  {
    for (double dir : Vector({-1., 1.}))
    {
      applStrains = strains0;
      applStrains[iPBC] += dir * .5 * perturb_;

      globdat.set(periodicBCModel::FIXEDGRAD_PARAM, applStrains);
      try
      {
        solver_->solve(info, globdat);
        readStrainStress_(pertubStrains, pertubStresses, globdat);

        strains[iPBC] += dir * pertubStrains;
        stresses[iPBC] += dir * pertubStresses;
      }
      catch (const jem::Exception &e)
      {
        System::warn() << e.what()
                       << "\nStress/Strain Data set to NAN!\n";
        strains[iPBC] = NAN;
        stresses[iPBC] = NAN;
      }
      globdat.erase(periodicBCModel::FIXEDGRAD_PARAM);
      StateVector::restoreNew(DofSpace::get(globdat, getContext()),
                              globdat);
    }
    reportStrainStress_(strains[iPBC], stresses[iPBC]);
  }

  groupUpdate_->run(globdat);
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
    myVars.set("E", stresses(0, 0) / strains(0, 0) / thickness_);
    break;

  default:
    NOT_IMPLEMENTED
    break;
  }

  // TEST_CONTEXT(strains)
  // TEST_CONTEXT(stresses)
  // TEST_CONTEXT(myVars)
}

void TangentOutputModule::condenseMatrix_(const Matrix &strains,
                                          const Matrix &stresses,
                                          const Properties &globdat)
{
  Vector fint(cons_->dofCount());
  Ref<FEMatrixBuilder> mB = newInstance<FEMatrixBuilder>(
      "tangentBuilder", ElementSet::get(globdat, "tangentBuild"),
      cons_->getDofSpace());
  Ref<AbstractMatrix> K;
  Properties params;

  mB->setToZero();
  params.set(ActionParams::INT_VECTOR, fint);
  params.set(ActionParams::MATRIX0, mB);
  params.set(ActionParams::LOAD_CASE, "condenseMatrix");
  masterModel_->takeAction(Actions::GET_MATRIX0, params, globdat);
  masterModel_->takeAction(Actions::GET_CONSTRAINTS, params, globdat);
  params.clear();
  mB->updateMatrix();
  K = mB->getMatrix();

  // reduce the stiffness matrix
  K = newInstance<ConstrainedMatrix>(K, cons_);

  // iterate over the unit strains
  Vector u(cons_->dofCount());
  strains = 0.;
  for (idx_t i = 0; i < rank_; i++)
    for (idx_t j = 0; j < rank_; j++)
    {
      strains(i * rank_ + j, i * rank_ + j) = 1.;

      u = 0.;
      fint = 0.;

      u[strainDofs_(i, j)] = 1.;

      K->matmul(fint, u);

      readStresses_(stresses[i * rank_ + j], fint, globdat);

      TEST_CONTEXT(strains[i * rank_ + j])
      TEST_CONTEXT(stresses[i * rank_ + j])
    }
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