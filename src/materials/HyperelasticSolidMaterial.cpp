/**
 * @file HyperelasticSolidMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of the base class of finite-strain solid materials
 */
#include "materials/HyperelasticSolidMaterial.h"

#include <jem/base/ClassTemplate.h>
#include <jem/base/IllegalOperationException.h>

JEM_DEFINE_CLASS(HyperelasticSolidMaterial);

HyperelasticSolidMaterial::HyperelasticSolidMaterial(const String &name,
                                                     const Properties &conf,
                                                     const Properties &props,
                                                     const Properties &globdat) : Super(name, conf, props, globdat)
{
}

HyperelasticSolidMaterial::~HyperelasticSolidMaterial()
{
}

void HyperelasticSolidMaterial::getStress(const Vector &, const Vector &)
{
  throw jem::IllegalOperationException(
      getContext(), "this material is driven by the deformation gradient, strain vectors are not supported");
}

void HyperelasticSolidMaterial::getStress(const Matrix &stress, const Matrix &F, const idx_t &ielem, const idx_t &ip, const bool)
{
  currEnergy_(ip, ielem) = calcStress_(stress, F);
}

Matrix HyperelasticSolidMaterial::getMaterialStiff() const
{
  throw jem::IllegalOperationException(
      getContext(), "the tangent of this material depends on the deformation gradient");
}

void HyperelasticSolidMaterial::getMaterialStiff(const Matrix &stiff, const Matrix &F, const idx_t &, const idx_t &) const
{
  calcTangent_(stiff, F);
}
