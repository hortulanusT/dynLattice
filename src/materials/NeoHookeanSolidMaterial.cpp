/**
 * @file NeoHookeanSolidMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of the compressible Neo-Hookean solid
 */
#include "materials/NeoHookeanSolidMaterial.h"

#include "materials/MaterialFactory.h"

#include <jem/base/ClassTemplate.h>
#include <jem/base/IllegalInputException.h>
#include <jive/util/ObjectConverter.h>

#include <cmath>
#include <math.h>

using jem::newInstance;

JEM_DEFINE_CLASS(NeoHookeanSolidMaterial);

const char *NeoHookeanSolidMaterial::TYPE_NAME = "NeoHookeanSolid";

NeoHookeanSolidMaterial::NeoHookeanSolidMaterial(const String &name,
                                                 const Properties &conf,
                                                 const Properties &props,
                                                 const Properties &globdat) : Super(name, conf, props, globdat)
{
  configure(props, globdat);
  getConfig(conf, globdat);
}

NeoHookeanSolidMaterial::~NeoHookeanSolidMaterial()
{
}

void NeoHookeanSolidMaterial::configure(const Properties &props, const Properties &globdat)
{
  Super::configure(props, globdat);

  Properties myProps = props.getProps(myName_);
  myProps.setConverter(newInstance<jive::util::ObjConverter>(globdat));

  getLame(myProps, mu_, lambda_);
}

void NeoHookeanSolidMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Super::getConfig(conf, globdat);

  Properties myConf = conf.makeProps(myName_);

  myConf.set("young", mu_ * (3. * lambda_ + 2. * mu_) / (lambda_ + mu_));
  myConf.set("shear_modulus", mu_);
}

double NeoHookeanSolidMaterial::checkedDet_(const Matrix &F) const
{
  const double det = F(0, 0) * (F(1, 1) * F(2, 2) - F(1, 2) * F(2, 1)) -
                     F(0, 1) * (F(1, 0) * F(2, 2) - F(1, 2) * F(2, 0)) +
                     F(0, 2) * (F(1, 0) * F(2, 1) - F(1, 1) * F(2, 0));

  // also catches NaN, for which every comparison is false, and infinity
  if (!(det > 0. && std::isfinite(det)))
    throw jem::IllegalInputException(
        getContext(), "the determinant of the deformation gradient has to be positive and finite, got " + String(det));

  return det;
}

void NeoHookeanSolidMaterial::invert_(double (&Finv)[3][3], const Matrix &F, double det)
{
  for (idx_t i = 0; i < 3; i++)
  {
    for (idx_t j = 0; j < 3; j++)
    {
      idx_t i1 = (i + 1) % 3, i2 = (i + 2) % 3;
      idx_t j1 = (j + 1) % 3, j2 = (j + 2) % 3;
      Finv[j][i] = (F(i1, j1) * F(i2, j2) - F(i1, j2) * F(i2, j1)) / det;
    }
  }
}

double NeoHookeanSolidMaterial::calcStress_(const Matrix &P, const Matrix &F) const
{
  const double J = checkedDet_(F);
  const double lnJ = log(J);

  double Finv[3][3];
  invert_(Finv, F, J);

  double I1 = 0.;
  for (idx_t i = 0; i < 3; i++)
    for (idx_t j = 0; j < 3; j++)
      I1 += F(i, j) * F(i, j);

  // P = mu F + (lambda ln J - mu) F^-T
  const double c = lambda_ * lnJ - mu_;
  for (idx_t i = 0; i < 3; i++)
    for (idx_t j = 0; j < 3; j++)
      P(i, j) = mu_ * F(i, j) + c * Finv[j][i];

  return 0.5 * mu_ * (I1 - 3.) - mu_ * lnJ + 0.5 * lambda_ * lnJ * lnJ;
}

void NeoHookeanSolidMaterial::calcTangent_(const Matrix &A, const Matrix &F) const
{
  const double J = checkedDet_(F);
  const double lnJ = log(J);

  double Finv[3][3];
  invert_(Finv, F, J);

  // dP_iJ / dF_kL = mu delta_ik delta_JL + (mu - lambda ln J) Finv_Jk Finv_Li
  //                 + lambda Finv_Ji Finv_Lk
  const double c = mu_ - lambda_ * lnJ;
  for (idx_t J_ = 0; J_ < 3; J_++)
  {
    for (idx_t L = 0; L < 3; L++)
    {
      for (idx_t i = 0; i < 3; i++)
      {
        for (idx_t k = 0; k < 3; k++)
        {
          double value = c * Finv[J_][k] * Finv[L][i] + lambda_ * Finv[J_][i] * Finv[L][k];
          if (i == k && J_ == L)
            value += mu_;
          A(i + 3 * J_, k + 3 * L) = value;
        }
      }
    }
  }
}

Ref<Material> NeoHookeanSolidMaterial::makeNew(const String &name, const Properties &conf,
                                               const Properties &props, const Properties &globdat)
{
  return newInstance<NeoHookeanSolidMaterial>(name, conf, props, globdat);
}

void NeoHookeanSolidMaterial::declare()
{
  MaterialFactory::declare(TYPE_NAME, &makeNew);
  MaterialFactory::declare(CLASS_NAME, &makeNew);
}
