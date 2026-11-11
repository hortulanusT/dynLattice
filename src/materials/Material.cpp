/**
 * @file Material.cpp
 * @author Til Gärtner
 * @brief Material class for representing different materials in the simulation.
 *
 */
#include "materials/Material.h"
#include <jem/base/ClassTemplate.h>
#include <jem/base/IllegalInputException.h>
#include <jem/base/IllegalOperationException.h>
#include <jem/base/System.h>

JEM_DEFINE_CLASS(Material);

const char *Material::VERBOSITY_PROP = "verbosity";

Material::Material(const String &name,
                   const Properties &conf,
                   const Properties &props,
                   const Properties &) : Super(name)
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

void Material::configure(const Properties &, const Properties &)
{
}

void Material::getConfig(const Properties &, const Properties &) const
{
}

void Material::getStress(const Vector &stress, const Vector &strain, const idx_t &, const idx_t &, const bool)
{
  getStress(stress, strain);
}

void Material::getMaterialStiff(const Matrix &stiff) const
{
  stiff = getMaterialStiff();
}

void Material::getMaterialStiff(const Matrix &stiff, const idx_t &, const idx_t &) const
{
  stiff = getMaterialStiff();
}

void Material::getMaterialMass(const Matrix &mass, const idx_t &, const idx_t &) const
{
  mass = getMaterialMass();
}

Matrix Material::getLumpedMass(double size) const
{
  return Matrix(getMaterialMass() * size);
}

Matrix Material::getLumpedMass(double size, const idx_t &) const
{
  return getLumpedMass(size);
}

void Material::getTable(XTable &, const Vector &, const String &name, const IdxVector &) const
{
  jem::System::warn() << name << " not supported by this material\n";
}

double Material::getDissipatedEnergy(const idx_t &, const idx_t &) const
{
  return 0.;
}

double Material::getHardeningPotential(const idx_t &, const idx_t &) const
{
  return 0.;
}

void Material::getStress(const Matrix &, const Matrix &, const idx_t &, const idx_t &, const bool)
{
  throw jem::IllegalOperationException(
      getContext(), "this material does not support deformation gradient input");
}

void Material::getMaterialStiff(const Matrix &, const Matrix &, const idx_t &, const idx_t &) const
{
  throw jem::IllegalOperationException(
      getContext(), "this material does not support deformation gradient input");
}

void Material::readCounts_(const Properties &myProps, idx_t &ipCount, idx_t &elemCount) const
{
  myProps.get(ipCount, "ipCount");
  myProps.get(elemCount, "elemCount");

  if (ipCount <= 0 || elemCount < 0)
    throw jem::IllegalInputException(
        getContext(), "'ipCount' has to be positive and 'elemCount' non-negative, got " +
                          String(ipCount) + " and " + String(elemCount));
}
