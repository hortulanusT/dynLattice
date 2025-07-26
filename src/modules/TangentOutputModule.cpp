/**
 * @file TangentOutputModule.h
 * @author Til Gärtner
 * @brief Module, that calculates the tangent linear elastic equivalent
 * properties for the material
 *
 *
 */

#include "modules/TangentOutputModule.h"

#include <jem/base/ClassTemplate.h>

using jive_helpers::eye;

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
                                     0);
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
  // double size = 1;
  strains = 0.;
  stresses = 0.;

  groupUpdate_->run(globdat);

  // for (String sizeMeas : sizes_)
  //   size *= FuncUtils::evalExpr(sizeMeas, globdat);
  for (idx_t iComp = 0; iComp < rank_ * rank_; iComp++)
    strains[iComp] = FuncUtils::evalExpr(strains_[iComp], globdat);
  for (idx_t iComp = 0; iComp < rank_ * rank_; iComp++)
    stresses[iComp] = FuncUtils::evalExpr(stresses_[iComp], globdat);
}

void TangentOutputModule::reportStrainStress_(const Vector &H,
                                              const Vector &N)
{
  Matrix deformTens(rank_, rank_);
  Matrix engStress(rank_, rank_);
  Matrix engStrain(rank_, rank_);
  double J;

  vec2mat(deformTens, H);
  deformTens += eye(rank_);
  J = jem::numeric::det(deformTens);

  engStrain = 0.5 * (deformTens +
                     deformTens.transpose()) -
              eye(rank_); // linearized strain

  vec2mat(engStress, N);
  engStress =
      matmul(deformTens, engStress) / J; // chauchy stresses

  System::info() << "### Cauchy's strain tensor\n"
                 << engStrain << "\n";
  System::info() << "### Cauchy's stress tensor\n"
                 << engStress << "\n";
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
      }
      catch (const jem::Exception &e)
      {
        print(System::warn(),
              "The Newton-Raphson solver didn't converge, taking non-converged result for tangent calculation \n\n");
      }

      readStrainStress_(pertubStrains, pertubStresses, globdat);

      strains[iPBC] += dir * pertubStrains;
      stresses[iPBC] += dir * pertubStresses;

      globdat.erase(periodicBCModel::FIXEDGRAD_PARAM);
      solver_->cancel(globdat);
    }
    System::info() << " > > > Results from strainig along " << iPBC << " direction:\n";
    reportStrainStress_(strains[iPBC], stresses[iPBC]);
  }

  groupUpdate_->run(globdat);
}

void TangentOutputModule::storeTangentProps_(const Matrix &strains,
                                             const Matrix &stresses,
                                             const Properties &globdat)
{
  JEM_ASSERT(strains.size(0) == strains.size(1));
  JEM_ASSERT(stresses.size(0) == stresses.size(1));
  JEM_ASSERT(strains.size() == stresses.size());
  JEM_ASSERT(strains.size() == rank_ * rank_);

  const idx_t compCount = strains.size();
  Properties myVars = Globdat::getVariables("tangent", globdat);

  Matrix C(stresses.size(0), stresses.size(1));
  Matrix S(stresses.size(0), stresses.size(1));
  Vector C_prop(jem::product(stresses.shape()));
  Vector S_prop(jem::product(stresses.shape()));

  try
  {
    C = Matrix(stresses / thickness_ / perturb_);
    S = jem::numeric::inverse(C);
  }
  catch (const jem::Exception &e)
  {
    print(System::info(myName_),
          "The tangent matrix non-invertible, setting everything to NaN \n\n");
    C = NAN;
    S = NAN;
  }

  for (idx_t i = 0; i < compCount; i++)
  {
    C_prop[slice(i * compCount, (i + 1) * compCount)] = C[i];
    S_prop[slice(i * compCount, (i + 1) * compCount)] = S[i];
  }

  myVars.set("stiffness", C_prop);
  myVars.set("compliance", S_prop);
}

void TangentOutputModule::condenseMatrix_(const Matrix &strains,
                                          const Matrix &stresses,
                                          const Properties &globdat){
    NOT_IMPLEMENTED}

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
