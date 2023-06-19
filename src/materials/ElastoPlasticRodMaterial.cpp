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

  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());
  StringVector dofNames = dofs->getTypeNames();
  String args = jem::util::StringUtils::join(dofNames, ", ");

  idx_t ipCount;
  idx_t elemCount;
  myProps.find(ipCount, "ipCount");
  myProps.find(elemCount, "elemCount");
  plastStrains_.resize(dofNames.size(), ipCount, elemCount);
  plastStrains_ = 0.;
  oldStresses_.resize(dofNames.size(), ipCount, elemCount);
  oldStresses_ = 0.;

  yieldCond_ = FuncUtils::newFunc(args, "-1.");
  FuncUtils::configFunc(yieldCond_, args, YIELD_PROP, myProps, globdat);
}

void ElastoPlasticRodMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Properties myConf = conf.makeProps(myName_);

  FuncUtils::getConfig(myConf, yieldCond_, YIELD_PROP);
}

void ElastoPlasticRodMaterial::update(const Vector &strain, const idx_t &ielem, const idx_t &ip)
{
  Vector stress(strain.size());
  Vector oldStress = oldStresses_[ielem][ip];
  Vector critStress(strain.size());
  Vector deriv(strain.size());

  Super::getStress(stress, Vector(strain - plastStrains_[ielem][ip]));

  double f_trial = yieldCond_->getValue(stress.addr());
  if (f_trial > 0 && !jem::isTiny(f_trial))
  {
    TEST_CONTEXT(strain)
    TEST_CONTEXT(stress)
    TEST_CONTEXT(f_trial)
    double f_old = yieldCond_->getValue(oldStress.addr());
    TEST_CONTEXT(f_old)
    critStress = oldStress - (stress - oldStress) * f_old / (f_trial - f_old);

    deriv = jive_helpers::funcGrad(yieldCond_, critStress);

    double deltaFlow = dotProduct(deriv, stress - critStress) / dotProduct(deriv, matmul(materialK_, deriv));

    plastStrains_[ielem][ip] += deltaFlow * deriv;
  }

  Super::getStress(oldStresses_[ielem][ip], Vector(strain - plastStrains_[ielem][ip]));
}

Matrix ElastoPlasticRodMaterial::getConsistentStiff(const Vector &stress) const
{
  return Super::getConsistentStiff(stress);

  // Vector deriv = jive_helpers::funcGrad(yieldCond_, stress);
  // double deltaFlow = dotProduct(deriv, stress) / dotProduct(deriv, matmul(materialK_, deriv));
  // if (isnan(deltaFlow))
  //   return materialK_;

  // Matrix H = jive_helpers::eye(stress.size());
  // H += deltaFlow * matmul(materialK_, jive_helpers::funcHessian(yieldCond_, stress));
  // H = matmul(jem::numeric::inverse(H), materialK_);

  // H -= matmul(matmul(H, matmul(deriv, deriv)), H) / dotProduct(deriv, matmul(H, deriv));

  // return H;
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
