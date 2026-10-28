/**
 * @file ElastoPlasticRodMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of elasto-plastic rod material with yield conditions and hardening
 */
#include "materials/ElastoPlasticRodMaterial.h"

#include "materials/MaterialFactory.h"
#include "utils/helpers.h"
#include "utils/testing.h"

#include <jem/base/Exception.h>
#include <jem/base/Float.h>
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
const char *ElastoPlasticRodMaterial::YIELD_TYPE_PROP = "yieldType";
const char *ElastoPlasticRodMaterial::YIELD_STRENGTH_PROP = "yieldStrength";
const char *ElastoPlasticRodMaterial::YIELD_EXPONENT_PROP = "yieldExponent";
const char *ElastoPlasticRodMaterial::ISO_HARD_PROP = "isotropicCoefficient";
const char *ElastoPlasticRodMaterial::KIN_HARD_PROP = "kinematicTensor";

ElastoPlasticRodMaterial::ElastoPlasticRodMaterial(const String &name,
                                                   const Properties &conf,
                                                   const Properties &props,
                                                   const Properties &globdat) : Super(name, conf, props, globdat)
{
  maxIter_ = 20;
  precision_ = 1e-5;
  yieldType_ = EXPRESSION;
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
  StringVector dofNames = dofs->getTypeNames();
  String args = StringUtils::join(dofNames, ", ");

  myProps.find(ipCount, "ipCount");
  myProps.find(elemCount, "elemCount");
  dofCount_ = dofs->typeCount();
  argCount_ = dofCount_;

  double isoCoeff;
  Vector kinHard;
  Matrix kinFacts(dofCount_, dofCount_);
  isoHard_ = myProps.find(isoCoeff, ISO_HARD_PROP);
  kinHard_ = false;
  if (isoHard_)
  {
    args = args + ", h_0";
    argCount_ += 1;
  }
  if (myProps.find(kinHard, KIN_HARD_PROP))
  {
    kinHard_ = true;
    jive_helpers::vec2mat(kinFacts.transpose(), kinHard);
    for (String dofName : dofNames)
    {
      args = args + ", h_" + dofName;
      argCount_ += 1;
    }
  }

  materialH_.resize(argCount_ - dofCount_, argCount_ - dofCount_);
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

  oldHardParams_.resize(argCount_ - dofCount_, ipCount, elemCount);
  oldHardParams_ = 0.;
  currHardParams_.resize(argCount_ - dofCount_, ipCount, elemCount);
  currHardParams_ = 0.;

  plastStrain_.resize(dofCount_);
  hardParams_.resize(argCount_ - dofCount_);
  hardStress_.resize(argCount_ - dofCount_);
  elasticStrain_.resize(dofCount_);
  yieldArgs_.resize(argCount_);
  yieldGrad_.resize(argCount_);

  oldPlastStrains_.resize(dofCount_, ipCount, elemCount);
  oldPlastStrains_ = 0.;
  currPlastStrains_.resize(dofCount_, ipCount, elemCount);
  currPlastStrains_ = 0.;

  oldStresses_.resize(dofCount_, ipCount, elemCount);
  oldStresses_ = 0.;

  currDeltaFlow_.resize(ipCount, elemCount);
  currDeltaFlow_ = 0.;

  energyDiss_.resize(ipCount, elemCount);
  energyDiss_ = 0.;
  energyHardPot_.resize(ipCount, elemCount);
  energyHardPot_ = 0.;

  String yieldTypeName;
  if (myProps.find(yieldTypeName, YIELD_TYPE_PROP))
  {
    if (yieldTypeName == "Expression")
    {
      yieldType_ = EXPRESSION;
    }
    else if (yieldTypeName == "SuperEllipse")
    {
      yieldType_ = SUPER_ELLIPSE;
    }
    else
    {
      myProps.propertyError(YIELD_TYPE_PROP,
                            "invalid yield type: " + yieldTypeName + "; valid types are `Expression' and `SuperEllipse'");
    }
  }
  yieldDeriv_.resize(0);

  switch (yieldType_)
  {
  case EXPRESSION:
    if (!myProps.contains(YIELD_PROP))
      throw jem::util::PropertyException("Expected a yield function for an elasto-plastic material!");
    FuncUtils::configFunc(yieldCond_, args, YIELD_PROP, myProps, globdat);
    if (myProps.contains(YIELD_DERIV_PROP))
    {
      FuncUtils::configFuncs(yieldDeriv_, args, YIELD_DERIV_PROP, myProps, globdat);
      JEM_PRECHECK(yieldDeriv_.size() == argCount_);
    }
    break;

  case SUPER_ELLIPSE:
    if (myProps.contains(YIELD_PROP) || myProps.contains(YIELD_DERIV_PROP))
      throw jem::util::PropertyException(
          "The '" + String(YIELD_PROP) + "' and '" + YIELD_DERIV_PROP + "' properties cannot be combined with "
          + YIELD_TYPE_PROP + " = \"SuperEllipse\"!");
    if (argCount_ == dofCount_)
      throw jem::util::PropertyException(
          "The super-ellipse yield condition requires isotropic and/or kinematic hardening!");
    if (!myProps.find(yieldStrength_, YIELD_STRENGTH_PROP) || !myProps.find(yieldExponent_, YIELD_EXPONENT_PROP))
      throw jem::util::PropertyException(
          "The super-ellipse yield condition requires '" + String(YIELD_STRENGTH_PROP) + "' and '" + YIELD_EXPONENT_PROP + "'!");
    if (yieldStrength_.size() != dofCount_ || yieldExponent_.size() != dofCount_)
      throw jem::util::PropertyException(
          "'" + String(YIELD_STRENGTH_PROP) + "' and '" + YIELD_EXPONENT_PROP + "' need one entry per degree of freedom!");
    break;
  }

  myProps.find(maxIter_, jive::implict::PropNames::MAX_ITER);
  myProps.find(precision_, jive::implict::PropNames::PRECISION);
}

void ElastoPlasticRodMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  (void)globdat; // unused

  Properties myConf = conf.makeProps(myName_);

  switch (yieldType_)
  {
  case EXPRESSION:
    myConf.set(YIELD_TYPE_PROP, "Expression");
    FuncUtils::getConfig(myConf, yieldCond_, YIELD_PROP);
    if (yieldDeriv_.size() > 0)
    {
      FuncUtils::getConfig(myConf, yieldDeriv_, YIELD_DERIV_PROP);
    }
    break;

  case SUPER_ELLIPSE:
    myConf.set(YIELD_TYPE_PROP, "SuperEllipse");
    myConf.set(YIELD_STRENGTH_PROP, yieldStrength_);
    myConf.set(YIELD_EXPONENT_PROP, yieldExponent_);
    break;
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

double ElastoPlasticRodMaterial::getYield_(const Vector &args) const
{
  switch (yieldType_)
  {
  case SUPER_ELLIPSE:
    return getSuperEllipse_(args);

  case EXPRESSION:
    break;
  }
  return yieldCond_->getValue(args.addr());
}

void ElastoPlasticRodMaterial::getYieldGrad_(const Vector &grad, const Vector &args) const
{
  switch (yieldType_)
  {
  case SUPER_ELLIPSE:
    getSuperEllipseGrad_(grad, args);
    return;

  case EXPRESSION:
    break;
  }

  if (yieldDeriv_.size() > 0)
  {
    grad = jive_helpers::evalFuncs(yieldDeriv_, args);
  }
  else
  {
    grad = jive_helpers::funcGrad(yieldCond_, args);
    for (idx_t i = 0; i < dofCount_; i++)
      if (jem::numeric::abs(args[i]) < jem::Float::EPSILON)
        grad[i] = 0.;
  }
}

// args = [s (dofCount_), h_0 (if isoHard_), h_i (dofCount_, if kinHard_)]
// f = sum_i |s_i / d_i|^p_i - (1 + h_0),  d_i = c_i - h_i

double ElastoPlasticRodMaterial::getSuperEllipse_(const Vector &args) const
{
  const idx_t kinOffset = dofCount_ + (isoHard_ ? 1 : 0);

  double f = -1.;

  for (idx_t i = 0; i < dofCount_; i++)
  {
    const double d = yieldStrength_[i] - (kinHard_ ? args[kinOffset + i] : 0.);
    f += pow(jem::numeric::abs(args[i] / d), yieldExponent_[i]);
  }

  if (isoHard_)
    f -= args[dofCount_];

  return f;
}

// df/ds_i = p_i |s_i/d_i|^p_i sign(s_i) / |s_i|,  df/dh_i = p_i |s_i/d_i|^p_i / d_i,  df/dh_0 = -1

void ElastoPlasticRodMaterial::getSuperEllipseGrad_(const Vector &grad, const Vector &args) const
{
  const idx_t kinOffset = dofCount_ + (isoHard_ ? 1 : 0);

  for (idx_t i = 0; i < dofCount_; i++)
  {
    const double s = args[i];
    const double p = yieldExponent_[i];
    const double d = yieldStrength_[i] - (kinHard_ ? args[kinOffset + i] : 0.);
    const double ppow = p * pow(jem::numeric::abs(s / d), p);

    grad[i] = (jem::numeric::abs(s) < jem::Float::EPSILON) ? 0. : jem::numeric::sign(ppow / jem::numeric::abs(s), s);
    if (kinHard_)
      grad[kinOffset + i] = ppow / d;
  }

  if (isoHard_)
    grad[dofCount_] = -1.;
}

void ElastoPlasticRodMaterial::getHardVals(const Vector &hardVals, const Vector &hardParams) const
{
  // output form of the same product: no temporary
  matmul(hardVals, materialH_, hardParams);
  hardVals *= -1.;
}

void ElastoPlasticRodMaterial::getStress(const Vector &, const Vector &)
{
  throw jem::Exception(
      JEM_FUNC,
      "ElastoPlasticRodMaterial requires element and integration-point indices; "
      "use getStress(stress, strain, ielem, ip, inelastic)");
}

void ElastoPlasticRodMaterial::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic)
{
  if (verbosity_ > 1)
    jem::System::debug(myName_) << "elastoplastic material behavior for element " << ielem << " and integration point " << ip << "\n";
  // REPORT("Step 1")
  idx_t liter = 0;
  // start from the old state of this integration point (copied into the work arrays)
  plastStrain_ = oldPlastStrains_(ALL, ip, ielem);
  hardParams_ = oldHardParams_(ALL, ip, ielem);
  double deltaFlow = 0.;

  double yieldValue = 0.;
  double deltaDeltaFlow = 0.;

  while (true)
  {
    // SUBHEADER2("Step 2", liter)
    elasticStrain_ = strain - plastStrain_;
    Super::getStress(stress, elasticStrain_, ielem, ip, false);
    getHardVals(hardStress_, hardParams_);

    if (!inelastic || ((jem::numeric::abs(edgeFact_ - 1.0) > jem::Float::EPSILON) && (ielem < edgeElems_ || ielem > nElem_ - edgeElems_ - 1)))
    {
      if (verbosity_ > 1)
        jem::System::debug(myName_) << "        elastic calculation\n";
      break;
    }

    yieldArgs_[jem::SliceTo(dofCount_)] = stress;
    yieldArgs_[jem::SliceFromTo(dofCount_, argCount_)] = hardStress_;

    yieldValue = getYield_(yieldArgs_);

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

    getYieldGrad_(yieldGrad_, yieldArgs_);

    deltaDeltaFlow = yieldValue / (dotProduct(yieldGrad_[jem::SliceTo(dofCount_)], matmul(materialK_, yieldGrad_[jem::SliceTo(dofCount_)])) + dotProduct(yieldGrad_[jem::SliceFromTo(dofCount_, argCount_)], matmul(materialH_, yieldGrad_[jem::SliceFromTo(dofCount_, argCount_)])));

    // SUBHEADER2("Step 4", liter)
    plastStrain_ += deltaDeltaFlow * yieldGrad_[jem::SliceTo(dofCount_)];
    hardParams_ += deltaDeltaFlow * yieldGrad_[jem::SliceFromTo(dofCount_, argCount_)];
    deltaFlow += deltaDeltaFlow;

    liter++;
  }

  currStrains_(ALL, ip, ielem) = strain;
  currPlastStrains_(ALL, ip, ielem) = plastStrain_;
  currHardParams_(ALL, ip, ielem) = hardParams_;
  currDeltaFlow_(ip, ielem) = deltaFlow;
}

void ElastoPlasticRodMaterial::applyDeform()
{
  Vector currElastStrain(currStrains_.size(0));
  Vector currStress(currStrains_.size(0));
  Vector deltaPlastStrain(currStrains_.size(0));
  Vector currHardParams(currHardParams_.size(0));

  for (idx_t ielem = 0; ielem < currStrains_.size(2); ielem++)
  {
    for (idx_t ip = 0; ip < currStrains_.size(1); ip++)
    {
      currElastStrain = currStrains_(ALL, ip, ielem) - currPlastStrains_(ALL, ip, ielem);
      deltaPlastStrain = currPlastStrains_(ALL, ip, ielem) - oldPlastStrains_(ALL, ip, ielem);
      currHardParams = currHardParams_(ALL, ip, ielem);
      Super::getStress(currStress, currElastStrain);

      energyHardPot_(ip, ielem) = 0.5 * dotProduct(currHardParams, matmul(materialH_, currHardParams));
      energyPot_(ip, ielem) = 0.5 * dotProduct(currElastStrain, currStress);

      WARN_ASSERT2(currDeltaFlow_(ip, ielem) >= 0., "Negative plastic multiplier");
      if (jem::numeric::abs(currDeltaFlow_(ip, ielem)) > jem::Float::EPSILON)
      {
        energyDiss_(ip, ielem) += dotProduct((oldStresses_(ALL, ip, ielem) + currStress) / 2., deltaPlastStrain);
      }
      oldStresses_(ALL, ip, ielem) = currStress;
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

void ElastoPlasticRodMaterial::getTable(XTable &strain_table, const Vector &weights, const String &name, const IdxVector &items) const
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
