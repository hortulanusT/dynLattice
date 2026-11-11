/**
 * @file NeoHookeanSolidMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of the compressible Neo-Hookean solid
 */
#include "materials/NeoHookeanSolidMaterial.h"

#include "materials/MaterialFactory.h"

#include <jem/base/ClassTemplate.h>
#include <jem/base/Float.h>
#include <jem/base/IllegalInputException.h>
#include <jem/numeric/algebra/LUSolver.h>
#include <jive/util/ObjectConverter.h>

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

  if (myProps.contains("symmetry") || myProps.contains("C_11"))
    throw jem::IllegalInputException(
        getContext(), "the material is isotropic: 'symmetry' and 'C_ij' are not supported, "
                      "give 'young' and either 'poisson_ratio' or 'shear_modulus'");

  getLame_(myProps, mu_, lambda_);
}

void NeoHookeanSolidMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Super::getConfig(conf, globdat);

  Properties myConf = conf.makeProps(myName_);

  myConf.set("young", mu_ * (3. * lambda_ + 2. * mu_) / (lambda_ + mu_));
  myConf.set("shear_modulus", mu_);
}

double NeoHookeanSolidMaterial::invertF_(jem::Tuple<double, GLOB_RANK, GLOB_RANK> &Finv, const Matrix &F) const
{
  for (idx_t i = 0; i < GLOB_RANK; i++)
    for (idx_t j = 0; j < GLOB_RANK; j++)
      Finv(i, j) = F(i, j);

  double detF;
  jem::numeric::invert(Finv, detF);

  // NaN and infinity fail these comparisons as well
  if (!(detF > 0. && detF <= jem::Float::MAX_VALUE))
    throw jem::IllegalInputException(
        getContext(), "the determinant of the deformation gradient has to be positive and finite, got " + String(detF));

  return detF;
}

double NeoHookeanSolidMaterial::calcStress_(const Matrix &P, const Matrix &F) const
{
  jem::Tuple<double, GLOB_RANK, GLOB_RANK> Finv;
  const double detF = invertF_(Finv, F); // J = det F, the volume ratio
  const double lnJ = log(detF);

  double I1 = 0.;
  for (idx_t i = 0; i < GLOB_RANK; i++)
    for (idx_t j = 0; j < GLOB_RANK; j++)
      I1 += F(i, j) * F(i, j);

  // P = mu F + (lambda ln J - mu) F^-T
  const double c = lambda_ * lnJ - mu_;
  for (idx_t i = 0; i < GLOB_RANK; i++)
    for (idx_t j = 0; j < GLOB_RANK; j++)
      P(i, j) = mu_ * F(i, j) + c * Finv(j, i);

  return 0.5 * mu_ * (I1 - 3.) - mu_ * lnJ + 0.5 * lambda_ * lnJ * lnJ;
}

void NeoHookeanSolidMaterial::calcTangent_(const Matrix &A, const Matrix &F) const
{
  jem::Tuple<double, GLOB_RANK, GLOB_RANK> Finv;
  const double detF = invertF_(Finv, F); // J = det F, the volume ratio
  const double lnJ = log(detF);

  // dP_iJ / dF_kL = mu delta_ik delta_JL + (mu - lambda ln J) Finv_Jk Finv_Li
  //                 + lambda Finv_Ji Finv_Lk   (here J and L are indices, ln J = ln det F)
  const double c = mu_ - lambda_ * lnJ;
  for (idx_t J = 0; J < GLOB_RANK; J++)
  {
    for (idx_t L = 0; L < GLOB_RANK; L++)
    {
      for (idx_t i = 0; i < GLOB_RANK; i++)
      {
        for (idx_t k = 0; k < GLOB_RANK; k++)
        {
          double value = c * Finv(J, k) * Finv(L, i) + lambda_ * Finv(J, i) * Finv(L, k);
          if (i == k && J == L)
            value += mu_;
          A(i + GLOB_RANK * J, k + GLOB_RANK * L) = value;
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
