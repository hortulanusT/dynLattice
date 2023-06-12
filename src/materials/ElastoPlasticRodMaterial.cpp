#include "materials/ElastoPlasticRodMaterial.h"
#include "utils/testing.h"
#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(ElastoPlasticRodMaterial);

const char *ElastoPlasticRodMaterial::TYPE_NAME = "ElastoPlasticRod";
const char *ElastoPlasticRodMaterial::YIELD_PROP = "yieldCond";

ElastoPlasticRodMaterial::ElastoPlasticRodMaterial(const String &name,
                                                   const Properties &conf,
                                                   const Properties &props,
                                                   const Properties &globdat) : Super(name, conf, props, globdat)
{
  configure(props, globdat);
  getConfig(conf, globdat);
}

ElastoPlasticRodMaterial::~ElastoPlasticRodMaterial()
{
}

void ElastoPlasticRodMaterial::configure(const Properties &props, const Properties &globdat)
{
  Properties myProps = props.findProps(myName_);

  myProps.get(dofNames_, "dofNames"); // TODO get from the globdat dofs
  String args = jem::util::StringUtils::join(dofNames_, ", ");

  FuncUtils::configFunc(yieldCond_, args, YIELD_PROP, myProps, globdat);
}

void ElastoPlasticRodMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Properties myConf = conf.makeProps(myName_);

  FuncUtils::getConfig(myConf, yieldCond_, YIELD_PROP);
}

void ElastoPlasticRodMaterial::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip) const
{
  Super::getStress(stress, Vector(strain - plastStrains[ielem][ip]));

  double f_trial = yieldCond_->getValue(stress.addr());
  if (f_trial > 0 && !jem::isTiny(f_trial))
  {
    Vector oldStress = stresses[ielem][ip];
    Vector critStress(dofNames_.size());
    Vector deriv(dofNames_.size());

    double f_old = yieldCond_->getValue(oldStress.addr());
    critStress = oldStress - (stress - oldStress) * f_old / (f_trial - f_old);

    for (idx_t idof = 0; idof < dofNames_.size(); idof++)
      deriv[idof] = yieldCond_->getDeriv(idof, critStress.addr());

    double deltaFlow = dotProduct(deriv, stress - critStress) / dotProduct(deriv, matmul(materialK_, deriv));

    plastStrains[ielem][ip] += deltaFlow * deriv;
    Super::getStress(stress, Vector(strain - plastStrains[ielem][ip]));
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
