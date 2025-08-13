/**
 * @file Material.cpp
 * @author Til Gärtner
 * @brief Material class for representing different materials in the simulation.
 *
 */
#include "materials/Material.h"
#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(Material);

const char *Material::VERBOSITY_PROP = "verbosity";

Material::Material(const String &name,
                   const Properties &conf,
                   const Properties &props,
                   const Properties &globdat) : Super(name)
{
  verbosity_ = 1;

  Properties myProps = props.findProps(name);
  Properties myConf = conf.makeProps(name);

  myProps.find(verbosity_, VERBOSITY_PROP);
  myConf.set(VERBOSITY_PROP, verbosity_);
}

Material::~Material()
{
}

String Material::getContext() const
{
  return NamedObject::makeContext("material", myName_);
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
