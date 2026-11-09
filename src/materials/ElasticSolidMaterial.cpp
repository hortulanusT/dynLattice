/**
 * @file ElasticSolidMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of the small-strain linear elastic solid
 */
#include "materials/ElasticSolidMaterial.h"

#include "materials/MaterialFactory.h"

#include <jem/base/ClassTemplate.h>
#include <jem/base/System.h>
#include <jive/util/ObjectConverter.h>

using jem::newInstance;

JEM_DEFINE_CLASS(ElasticSolidMaterial);

const char *ElasticSolidMaterial::TYPE_NAME = "ElasticSolid";

ElasticSolidMaterial::ElasticSolidMaterial(const String &name,
                                           const Properties &conf,
                                           const Properties &props,
                                           const Properties &globdat) : Super(name, conf, props, globdat)
{
  configure(props, globdat);
  getConfig(conf, globdat);
}

ElasticSolidMaterial::~ElasticSolidMaterial()
{
}

void ElasticSolidMaterial::configure(const Properties &props, const Properties &globdat)
{
  Super::configure(props, globdat);

  Properties myProps = props.getProps(myName_);
  myProps.setConverter(newInstance<jive::util::ObjConverter>(globdat));

  materialK_.resize(6, 6);
  materialK_ = makeStiffness(myProps);

  if (verbosity_ > 0)
    jem::System::debug(myName_)
        << " ...Stiffness matrix of the material '" << myName_ << "':\n"
        << materialK_ << "\n";
}

void ElasticSolidMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Super::getConfig(conf, globdat);

  Properties myConf = conf.makeProps(myName_);

  for (idx_t i = 0; i < 6; i++)
    for (idx_t j = i; j < 6; j++)
      myConf.set("C_" + String(i + 1) + String(j + 1), materialK_(i, j));
}

void ElasticSolidMaterial::getStress(const Vector &stress, const Vector &strain)
{
  for (idx_t i = 0; i < 6; i++)
  {
    double sum = 0.;
    for (idx_t j = 0; j < 6; j++)
      sum += materialK_(i, j) * strain[j];
    stress[i] = sum;
  }
}

void ElasticSolidMaterial::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool)
{
  getStress(stress, strain);

  double energy = 0.;
  for (idx_t i = 0; i < 6; i++)
    energy += strain[i] * stress[i];
  currEnergy_(ip, ielem) = 0.5 * energy;
}

Matrix ElasticSolidMaterial::getMaterialStiff() const
{
  return materialK_.clone();
}

void ElasticSolidMaterial::getMaterialStiff(const Matrix &stiff) const
{
  stiff = materialK_;
}

void ElasticSolidMaterial::getMaterialStiff(const Matrix &stiff, const idx_t &, const idx_t &) const
{
  stiff = materialK_;
}

Ref<Material> ElasticSolidMaterial::makeNew(const String &name, const Properties &conf,
                                            const Properties &props, const Properties &globdat)
{
  return newInstance<ElasticSolidMaterial>(name, conf, props, globdat);
}

void ElasticSolidMaterial::declare()
{
  MaterialFactory::declare(TYPE_NAME, &makeNew);
  MaterialFactory::declare(CLASS_NAME, &makeNew);
}
