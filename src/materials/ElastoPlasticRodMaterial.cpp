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
  oldStrains_.resize(dofCount, ipCount, elemCount);
  oldStrains_ = 0.;

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
  const jem::Slice strain_part = jem::SliceTo(strain.size());
  const jem::Slice harden_part = jem::SliceFrom(strain.size());
  Vector args(argCount_);
  args = 0.;

  Super::getStress(args[strain_part], Vector(strain - plastStrains_[ielem][ip]));

  if (isoParams_.size())
  {
    args[harden_part] = 1. - isoParams_[ielem][ip];
  }
  else if (kinParams_.size())
  {
    args[harden_part] = -1. * matmul(kinFacts_, kinParams_[ielem][ip]);
  }

  double f_trial = yieldCond_->getValue(args.addr());
  if (f_trial > 0 && !jem::isTiny(f_trial))
  {
    double deltaFlow;
    Vector oldArgs(argCount_);
    Vector critArgs(argCount_);
    Vector critStrain(strain.size());
    Vector deriv(argCount_);
    Matrix A(argCount_, argCount_);
    Vector dStrain(argCount_);

    oldArgs = critArgs = args;
    Super::getStress(oldArgs[strain_part], Vector(oldStrains_[ielem][ip] - plastStrains_[ielem][ip]));
    double f_old = yieldCond_->getValue(oldArgs.addr());

    critStrain = oldStrains_[ielem][ip] - (strain - oldStrains_[ielem][ip]) * f_old / (f_trial - f_old);
    Super::getStress(critArgs[strain_part], Vector(critStrain - plastStrains_[ielem][ip]));

    deriv = jive_helpers::funcGrad(yieldCond_, critArgs);

    A = 0.;
    A(strain_part, strain_part) = Super::getMaterialStiff();

    if (isoParams_.size())
    {
      A(harden_part, harden_part) = isoCoeff_;
    }
    else if (kinParams_.size())
    {
      A(harden_part, harden_part) = kinFacts_;
    }

    dStrain = 0.;
    dStrain[strain_part] = strain - critStrain;

    deltaFlow = -1. * dotProduct(deriv, matmul(A, dStrain)) / dotProduct(deriv, matmul(Matrix(-1. * A), deriv));

    plastStrains_[ielem][ip] += deltaFlow * deriv[strain_part];
    if (isoParams_.size())
    {
      isoParams_[ielem][ip] += deltaFlow * deriv[strain.size()];
    }
    else if (kinParams_.size())
    {
      kinParams_[ielem][ip] += deltaFlow * deriv[harden_part];
    }
  }

  oldStrains_[ielem][ip] = strain;
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
