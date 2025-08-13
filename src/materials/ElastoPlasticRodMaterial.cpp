/**
 * @file ElastoPlasticRodMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of elasto-plastic rod material with yield conditions and hardening
 */
#include "materials/ElastoPlasticRodMaterial.h"

#include "materials/MaterialFactory.h"
#include "utils/helpers.h"
#include "utils/testing.h"

#include <jem/base/ClassTemplate.h>
#include <jem/util/PropertyException.h>
#include <jem/util/StringUtils.h>

#include <jive/implict/Names.h>
#include <jive/util/DofSpace.h>
#include <jive/util/ObjectConverter.h>
#include <jive/util/utilities.h>

using jem::newInstance;
using jem::numeric::dotProduct;
using jive::Cubix;
using jive::Ref;
using jive::Vector;

JEM_DEFINE_CLASS(ElastoPlasticRodMaterial);

const char *ElastoPlasticRodMaterial::TYPE_NAME = "ElastoPlasticRod";
const char *ElastoPlasticRodMaterial::YIELD_PROP = "yieldCond";
const char *ElastoPlasticRodMaterial::YIELD_DERIV_PROP = "yieldDeriv";
const char *ElastoPlasticRodMaterial::ISO_HARD_PROP = "isotropicCoefficient";
const char *ElastoPlasticRodMaterial::KIN_HARD_PROP = "kinematicTensor";

ElastoPlasticRodMaterial::ElastoPlasticRodMaterial(const String &name,
                                                   const Properties &conf,
                                                   const Properties &props,
                                                   const Properties &globdat) : Super(name, conf, props, globdat)
{
  maxIter_ = 20;
  precision_ = 1e-5;
  materialH_.resize(0);
  argCount_ = 0;

  configure(props, globdat);
  getConfig(conf, globdat);
}

ElastoPlasticRodMaterial::~ElastoPlasticRodMaterial()
{
}

void ElastoPlasticRodMaterial::configure(const Properties &props, const Properties &globdat)
{
  using jem::util::StringUtils;
  using jive::util::DofSpace;

  Properties myProps = props.findProps(myName_);
  myProps.setConverter(newInstance<jive::util::ObjConverter>(globdat));

  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());

  idx_t ipCount;
  idx_t elemCount;
  idx_t dofCount;
  StringVector dofNames = dofs->getTypeNames();
  String args = StringUtils::join(dofNames, ", ");

  myProps.find(ipCount, "ipCount");
  myProps.find(elemCount, "elemCount");
  dofCount = dofs->typeCount();
  argCount_ = dofCount;

  double isoCoeff;
  Vector kinHard;
  Matrix kinFacts(dofCount, dofCount);
  if (myProps.find(isoCoeff, ISO_HARD_PROP))
  {
    args = args + ", h_0";
    argCount_ += 1;
  }
  if (myProps.find(kinHard, KIN_HARD_PROP))
  {
    jive_helpers::vec2mat(kinFacts.transpose(), kinHard);
    for (String dofName : dofNames)
    {
      args = args + ", h_" + dofName;
      argCount_ += 1;
    }
  }

  stressPart_ = jem::SliceTo(dofCount);
  hardPart_ = jem::SliceFromTo(dofCount, argCount_);

  materialH_.resize(argCount_ - dofCount, argCount_ - dofCount);
  if (argCount_ == 7)
  {
    materialH_ = isoCoeff;
  }
  if (argCount_ == 12)
  {
    materialH_ = kinFacts;
  }
  if (argCount_ == 13)
  {
    materialH_ = 0.;
    materialH_(0, 0) = isoCoeff;
    materialH_(jem::SliceFrom(1), jem::SliceFrom(1)) = kinFacts;
  }

  if (verbosity_ > 0)
    jem::System::debug(myName_)
        << " ...Hardening matrix of the material '" << myName_ << "':\n"
        << materialH_ << "\n";

  oldHardParams_.resize(argCount_ - dofCount, ipCount, elemCount);
  oldHardParams_ = 0.;
  currHardParams_.resize(argCount_ - dofCount, ipCount, elemCount);
  currHardParams_ = 0.;

  oldPlastStrains_.resize(dofCount, ipCount, elemCount);
  oldPlastStrains_ = 0.;
  currPlastStrains_.resize(dofCount, ipCount, elemCount);
  currPlastStrains_ = 0.;

  currDeltaFlow_.resize(ipCount, elemCount);
  currDeltaFlow_ = 0.;

  energyDiss_.resize(ipCount, elemCount);
  energyDiss_ = 0.;
  energyHardPot_.resize(ipCount, elemCount);
  energyHardPot_ = 0.;

  if (!myProps.contains(YIELD_PROP))
    throw jem::util::PropertyException("Expected a yield function for an elasto-plastic material!");
  FuncUtils::configFunc(yieldCond_, args, YIELD_PROP, myProps, globdat);
  if (myProps.contains(YIELD_DERIV_PROP))
  {
    FuncUtils::configFuncs(yieldDeriv_, args, YIELD_DERIV_PROP, myProps, globdat);
    JEM_PRECHECK(yieldDeriv_.size() == argCount_);
  }
  else
  {
    yieldDeriv_.resize(0);
  }

  myProps.find(maxIter_, jive::implict::PropNames::MAX_ITER);
  myProps.find(precision_, jive::implict::PropNames::PRECISION);
}

void ElastoPlasticRodMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Properties myConf = conf.makeProps(myName_);

  FuncUtils::getConfig(myConf, yieldCond_, YIELD_PROP);
  if (yieldDeriv_.size() > 0)
  {
    FuncUtils::getConfig(myConf, yieldDeriv_, YIELD_DERIV_PROP);
  }

  if (argCount_ == 7)
  {
    myConf.set(ISO_HARD_PROP, materialH_(0, 0));
  }
  if (argCount_ == 12)
  {
    Vector kinHard(materialH_.size(0) * materialH_.size(1));
    jive_helpers::mat2vec(kinHard, materialH_);
    myConf.set(KIN_HARD_PROP, kinHard);
  }
  if (argCount_ == 13)
  {
    myConf.set(ISO_HARD_PROP, materialH_(0, 0));

    Vector kinHard((materialH_.size(0) - 1) * (materialH_.size(1) - 1));
    Matrix kinFacts((materialH_.size(0) - 1), (materialH_.size(1) - 1));
    jive_helpers::vec2mat(kinFacts.transpose(), kinHard);
    myConf.set(KIN_HARD_PROP, kinHard);
  }

  myConf.set(jive::implict::PropNames::MAX_ITER, maxIter_);
  myConf.set(jive::implict::PropNames::PRECISION, precision_);
}

void ElastoPlasticRodMaterial::getHardVals(const Vector &hardVals, const Vector &hardParams) const
{
  hardVals = -1. * matmul(materialH_, hardParams);
}

void ElastoPlasticRodMaterial::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic)
{
  if (verbosity_ > 1)
    jem::System::debug(myName_) << "elastoplastic material behavior for element " << ielem << " and integration point " << ip << "\n";
  // REPORT("Step 1")
  idx_t liter = 0;
  Vector plastStrain = oldPlastStrains_(ALL, ip, ielem).clone();
  Vector hardParams = oldHardParams_(ALL, ip, ielem).clone();
  double deltaFlow = 0.;

  Vector hardStress(argCount_ - stress.size());
  Vector args(argCount_);
  double yieldValue = 0.;
  Vector yieldGrad(argCount_);
  double deltaDeltaFlow = 0.;

  while (true)
  {
    // SUBHEADER2("Step 2", liter)
    Super::getStress(stress, Vector(strain - plastStrain), ielem, ip, false);
    getHardVals(hardStress, hardParams);

    if (!inelastic || ((edgeFact_ != 1.) && (ielem < edgeElems_ || ielem > nElem_ - edgeElems_ - 1)))
    {
      if (verbosity_ > 1)
        jem::System::debug(myName_) << "        elastic calculation\n";
      break;
    }

    args[stressPart_] = stress;
    args[hardPart_] = hardStress;

    yieldValue = yieldCond_->getValue(args.addr());

    if (verbosity_ > 2)
      jem::System::debug(myName_) << "        iter = " << liter << ", f = " << yieldValue << "\n";
    JEM_PRECHECK2(liter < maxIter_, "Too many iterations in plasticity loop");
    if (yieldValue < precision_)
    {
      if (verbosity_ > 1)
        jem::System::debug(myName_) << "        converged after " << liter << " iterations\n";
      break;
    }
    // SUBHEADER2("Step 3", liter)

    if (yieldDeriv_.size() > 0)
    {
      yieldGrad = jive_helpers::evalFuncs(yieldDeriv_, args);
    }
    else
    {
      yieldGrad = jive_helpers::funcGrad(yieldCond_, args);
      for (idx_t i = 0; i < strain.size(); i++)
        if (args[i] == 0.)
          yieldGrad[i] = 0.;
    }

    deltaDeltaFlow = yieldValue / (dotProduct(yieldGrad[stressPart_], matmul(materialK_, yieldGrad[stressPart_])) + dotProduct(yieldGrad[hardPart_], matmul(materialH_, yieldGrad[hardPart_])));

    // SUBHEADER2("Step 4", liter)
    plastStrain += deltaDeltaFlow * yieldGrad[stressPart_];
    hardParams += deltaDeltaFlow * yieldGrad[hardPart_];
    deltaFlow += deltaDeltaFlow;

    liter++;
  }

  currStrains_(ALL, ip, ielem) = strain;
  currPlastStrains_(ALL, ip, ielem) = plastStrain;
  currHardParams_(ALL, ip, ielem) = hardParams;
  currDeltaFlow_(ip, ielem) = deltaFlow;
}

void ElastoPlasticRodMaterial::applyDeform()
{
  Vector oldElastStrain(currStrains_.size(0));
  Vector currElastStrain(currStrains_.size(0));
  Vector oldStress(currStrains_.size(0));
  Vector currStress(currStrains_.size(0));
  Vector deltaPlastStrain(currStrains_.size(0));
  Vector currHardParams(currHardParams_.size(0));

  for (idx_t ielem = 0; ielem < currStrains_.size(2); ielem++)
  {
    for (idx_t ip = 0; ip < currStrains_.size(1); ip++)
    {
      oldElastStrain = oldStrains_(ALL, ip, ielem) - oldPlastStrains_(ALL, ip, ielem);
      currElastStrain = currStrains_(ALL, ip, ielem) - currPlastStrains_(ALL, ip, ielem);
      deltaPlastStrain = currPlastStrains_(ALL, ip, ielem) - oldPlastStrains_(ALL, ip, ielem);
      currHardParams = currHardParams_(ALL, ip, ielem);
      Super::getStress(oldStress, oldElastStrain);
      Super::getStress(currStress, currElastStrain);

      energyHardPot_(ip, ielem) = 0.5 * dotProduct(currHardParams, matmul(materialH_, currHardParams));
      energyPot_(ip, ielem) = 0.5 * dotProduct(currElastStrain, currStress);

      WARN_ASSERT2(currDeltaFlow_(ip, ielem) >= 0., "Negative plastic multiplier");
      if (currDeltaFlow_(ip, ielem) != 0.)
      {
        energyDiss_(ip, ielem) += dotProduct((oldStress + currStress) / 2., deltaPlastStrain);
      }
    }
  }

  oldHardParams_ = currHardParams_;
  oldPlastStrains_ = currPlastStrains_;
  oldStrains_ = currStrains_;

  currDeltaFlow_ = 0.;
}

void ElastoPlasticRodMaterial::rejectDeform()
{
  currHardParams_ = oldHardParams_;
  currPlastStrains_ = oldPlastStrains_;
  currStrains_ = oldStrains_;

  currDeltaFlow_ = 0.;
}

void ElastoPlasticRodMaterial::getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const
{
  using jive::IdxVector;
  using jive::util::XTable;

  if (name == "plast_strain")
  {
    const idx_t elemCount = items.size();
    const idx_t ipCount = currPlastStrains_.size(1);
    const IdxVector columns(strain_table.columnCount());
    columns = jem::iarray(strain_table.columnCount());

    for (idx_t ie = 0; ie < elemCount; ie++)
    {
      for (idx_t ip = 0; ip < ipCount; ip++)
      {
        strain_table.addRowValues(items[ie], columns, currPlastStrains_(ALL, ip, ie));
        weights[items[ie]] += 1.;
      }
    }

    return;
  }

  if (name == "hard_params")
  {
    const idx_t elemCount = items.size();
    const idx_t ipCount = currHardParams_.size(1);
    const IdxVector columns(strain_table.columnCount());
    columns = jem::iarray(strain_table.columnCount());

    for (idx_t ie = 0; ie < elemCount; ie++)
    {
      for (idx_t ip = 0; ip < ipCount; ip++)
      {
        strain_table.addRowValues(items[ie], columns, currHardParams_(ALL, ip, ie));
        weights[items[ie]] += 1.;
      }
    }

    return;
  }

  WARN(name + " not supported by this material");
}

double ElastoPlasticRodMaterial::getDissipatedEnergy(const idx_t &ielem, const idx_t &ip) const
{
  return energyDiss_(ip, ielem);
}

double ElastoPlasticRodMaterial::getPotentialEnergy(const idx_t &ielem, const idx_t &ip) const
{
  return energyPot_(ip, ielem);
}

double ElastoPlasticRodMaterial::getHardeningPotential(const idx_t &ielem, const idx_t &ip) const
{
  return energyHardPot_(ip, ielem);
}

Ref<Material> ElastoPlasticRodMaterial::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<ElastoPlasticRodMaterial>(name, conf, props, globdat);
}

void ElastoPlasticRodMaterial::declare()
{
  MaterialFactory::declare(TYPE_NAME, &makeNew);
  MaterialFactory::declare(CLASS_NAME, &makeNew);
}
