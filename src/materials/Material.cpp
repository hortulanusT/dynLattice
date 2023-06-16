#include "materials/Material.h"
#include "utils/testing.h"
#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(Material);

Material::Material(const String &name,
                   const Properties &conf,
                   const Properties &props,
                   const Properties &globdat) : Super(name)
{
}

Material::~Material()
{
}

void Material::configure(const Properties &props, const Properties &globdat)
{
}

void Material::getConfig(const Properties &props, const Properties &globdat) const
{
}

Matrix Material::getConsistentStiff(const Vector &stress) const
{
  return getMaterialStiff();
}

Matrix Material::getLumpedMass(double l) const
{
  return Matrix(getMaterialMass() * l);
}

void Material::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip) const
{
  getStress(stress, strain);
}