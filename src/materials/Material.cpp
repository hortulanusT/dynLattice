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

Matrix Material::getLumpedMass(double l) const
{
  return Matrix(getMaterialMass() * l);
}
