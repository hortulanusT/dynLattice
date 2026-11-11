/**
 * @file ElasticSolidMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of the small-strain linear elastic solid
 */
#include "materials/ElasticSolidMaterial.h"

#include "materials/MaterialFactory.h"

#include <jem/base/ClassTemplate.h>

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

  materialK_.resize(STRAIN_COUNT, STRAIN_COUNT);
  materialK_ = readStiffness_(props, globdat);
}

void ElasticSolidMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Super::getConfig(conf, globdat);

  writeStiffness_(conf, materialK_);
}

void ElasticSolidMaterial::getStress(const Vector &stress, const Vector &strain)
{
  // sigma = C epsilon, collected first: stress may be the same array as strain
  jem::Tuple<double, STRAIN_COUNT> sigma;
  for (idx_t i = 0; i < STRAIN_COUNT; i++)
  {
    sigma[i] = 0.;
    for (idx_t j = 0; j < STRAIN_COUNT; j++)
      sigma[i] += materialK_(i, j) * strain[j];
  }

  for (idx_t i = 0; i < STRAIN_COUNT; i++)
    stress[i] = sigma[i];
}

void ElasticSolidMaterial::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool)
{
  // energy density 1/2 epsilon : sigma, taken before stress may overwrite strain
  jem::Tuple<double, STRAIN_COUNT> sigma;
  double energy = 0.;
  for (idx_t i = 0; i < STRAIN_COUNT; i++)
  {
    sigma[i] = 0.;
    for (idx_t j = 0; j < STRAIN_COUNT; j++)
      sigma[i] += materialK_(i, j) * strain[j];
    energy += strain[i] * sigma[i];
  }
  currEnergy_(ip, ielem) = 0.5 * energy;

  for (idx_t i = 0; i < STRAIN_COUNT; i++)
    stress[i] = sigma[i];
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
