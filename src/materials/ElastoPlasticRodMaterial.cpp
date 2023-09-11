#include "materials/ElastoPlasticRodMaterial.h"
#include "utils/testing.h"
#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(ElastoPlasticRodMaterial);

const char *ElastoPlasticRodMaterial::TYPE_NAME = "ElastoPlasticRod";
const char *ElastoPlasticRodMaterial::YIELD_PROP = "yieldCond";
const char *ElastoPlasticRodMaterial::ISO_HARD_PROP = "hardeningCoefficient";
const char *ElastoPlasticRodMaterial::KIN_HARD_PROP = "hardeningTensor";

ElastoPlasticRodMaterial::ElastoPlasticRodMaterial(const String &name,
                                                   const Properties &conf,
                                                   const Properties &props,
                                                   const Properties &globdat) : Super(name, conf, props, globdat)
{
  isoParams_.resize(0);
  kinParams_.resize(0);
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

  Properties myProps = props.findProps(myName_);

  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());

  idx_t ipCount;
  idx_t elemCount;
  idx_t dofCount;
  StringVector dofNames = dofs->getTypeNames();
  String args = StringUtils::join(dofNames, ", ");
  Vector kinHard;

  myProps.find(ipCount, "ipCount");
  myProps.find(elemCount, "elemCount");
  dofCount = dofs->typeCount();
  argCount_ = dofCount;

  if (myProps.find(kinHard, KIN_HARD_PROP))
  {
    kinFacts_.resize(dofCount, dofCount);
    jive_helpers::vec2mat(kinFacts_.transpose(), kinHard);
    for (String dofName : dofNames)
    {
      args = args + ", b_" + dofName;
      argCount_ += 1;
    }
    kinParams_.resize(dofCount, ipCount, elemCount);
    kinParams_ = 0.;
  }
  else if (myProps.find(isoCoeff_, ISO_HARD_PROP)) // Kinematic hardening overides isotropic hardening!
  {
    args = args + ", a";
    argCount_ += 1;
    isoParams_.resize(ipCount, elemCount);
    isoParams_ = 0.;
  }

  plastStrains_.resize(dofCount, ipCount, elemCount);
  plastStrains_ = 0.;
  oldYieldArgs_.resize(argCount_, ipCount, elemCount);
  oldYieldArgs_ = 0.;

  if (!myProps.contains(YIELD_PROP))
    throw jem::util::PropertyException("Expected a yield function for an elasto-plastic material!");
  FuncUtils::configFunc(yieldCond_, args, YIELD_PROP, myProps, globdat);
}

void ElastoPlasticRodMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Properties myConf = conf.makeProps(myName_);

  FuncUtils::getConfig(myConf, yieldCond_, YIELD_PROP);

  if (isoParams_.size())
  {
    myConf.set(ISO_HARD_PROP, isoCoeff_);
  }

  if (kinParams_.size())
  {
    Vector kinHard(kinFacts_.size(0) * kinFacts_.size(1));
    jive_helpers::mat2vec(kinHard, kinFacts_);
    myConf.set(KIN_HARD_PROP, kinHard);
  }
}

void ElastoPlasticRodMaterial::update(const Vector &strain, const idx_t &ielem, const idx_t &ip)
{
  Vector args(argCount_);
  Vector oldArgs(argCount_);
  Vector critArgs(argCount_);
  Vector deriv(argCount_);

  Super::getStress(args[jem::SliceTo(strain.size())], Vector(strain - plastStrains_[ielem][ip]));
  if (isoParams_.size())
  {
    args[jem::SliceFrom(strain.size())] = -1. * isoCoeff_ * isoParams_[ielem][ip];
  }
  else if (kinParams_.size())
  {
    args[jem::SliceFrom(strain.size())] = -1. * matmul(kinFacts_, kinParams_[ielem][ip]);
  }

  double f_trial = yieldCond_->getValue(args.addr());
  if (f_trial > 0 && !jem::isTiny(f_trial))
  {
    double deltaFlow = 0.;

    oldArgs = oldYieldArgs_[ielem][ip];
    double f_old = yieldCond_->getValue(oldArgs.addr());

    critArgs = oldArgs - (args - oldArgs) * f_old / (f_trial - f_old);
    deriv = jive_helpers::funcGrad(yieldCond_, critArgs);

    Vector d_dStress = deriv[jem::SliceTo(strain.size())];
    double deltaFlow = dotProduct(d_dStress, args[jem::SliceTo(strain.size())] - critArgs[jem::SliceTo(strain.size())]) /
                       dotProduct(d_dStress, matmul(materialK_, d_dStress));

    if (isoParams_.size())
    {
      double d_dAlpha = deriv[strain.size()];
      isoParams_[ielem][ip] += deltaFlow * d_dAlpha;
    }
    else if (kinParams_.size())
    {
      Vector d_dBeta = deriv[jem::SliceFrom(strain.size())];
      kinParams_[ielem][ip] += deltaFlow * d_dBeta;
    }

    plastStrains_[ielem][ip] += deltaFlow * d_dStress;
  }

  Super::getStress(oldYieldArgs_[ielem][ip][jem::SliceTo(strain.size())], Vector(strain - plastStrains_[ielem][ip]));
  if (isoParams_.size())
  {
    oldYieldArgs_[ielem][ip][jem::SliceFrom(strain.size())] = -1. * isoCoeff_ * isoParams_[ielem][ip];
  }
  if (kinParams_.size())
  {
    oldYieldArgs_[ielem][ip][jem::SliceFrom(strain.size())] = -1. * matmul(kinFacts_, kinParams_[ielem][ip]);
  }
}

void ElastoPlasticRodMaterial::getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const
{
  if (name != "plast_strain")
  {
    WARN(name + " not supported by this material");
    return;
  }

  const idx_t elemCount = items.size();
  const idx_t ipCount = plastStrains_.size(1);
  const IdxVector columns(strain_table.columnCount());
  columns = jem::iarray(strain_table.columnCount());

  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    for (idx_t ip = 0; ip < ipCount; ip++)
    {
      strain_table.addRowValues(items[ie], columns, plastStrains_(ALL, ip, ie));
      weights[items[ie]] += 1.;
    }
  }
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
