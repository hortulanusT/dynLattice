/**
 * @file SaintVenantKirchhoffSolidMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of the St. Venant-Kirchhoff solid
 */
#include "materials/SaintVenantKirchhoffSolidMaterial.h"

#include "materials/MaterialFactory.h"
#include "utils/helpers.h"

#include <jem/base/ClassTemplate.h>

using jem::newInstance;
using jive_helpers::mul3;

JEM_DEFINE_CLASS(SaintVenantKirchhoffSolidMaterial);

const char *SaintVenantKirchhoffSolidMaterial::TYPE_NAME = "SaintVenantKirchhoffSolid";

SaintVenantKirchhoffSolidMaterial::SaintVenantKirchhoffSolidMaterial(const String &name,
                                                                     const Properties &conf,
                                                                     const Properties &props,
                                                                     const Properties &globdat) : Super(name, conf, props, globdat)
{
  configure(props, globdat);
  getConfig(conf, globdat);
}

SaintVenantKirchhoffSolidMaterial::~SaintVenantKirchhoffSolidMaterial()
{
}

void SaintVenantKirchhoffSolidMaterial::configure(const Properties &props, const Properties &globdat)
{
  Super::configure(props, globdat);

  materialK_.resize(STRAIN_COUNT, STRAIN_COUNT);
  materialK_ = readStiffness_(props, globdat);
  secondPKWork_.resize(GLOB_RANK, GLOB_RANK);
}

void SaintVenantKirchhoffSolidMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Super::getConfig(conf, globdat);

  writeStiffness_(conf, materialK_);
}

double SaintVenantKirchhoffSolidMaterial::calcSecondPK_(const Matrix &S, const Matrix &F) const
{
  // Green-Lagrange strain in Voigt form (engineering shear)
  jem::Tuple<double, STRAIN_COUNT> strain;
  for (idx_t i = 0; i < GLOB_RANK; i++)
  {
    for (idx_t j = i; j < GLOB_RANK; j++)
    {
      double e = (i == j) ? -1. : 0.;
      for (idx_t k = 0; k < GLOB_RANK; k++)
        e += F(k, i) * F(k, j);
      strain[VOIGT[i][j]] = (i == j) ? 0.5 * e : e;
    }
  }

  jem::Tuple<double, STRAIN_COUNT> stress;
  double energy = 0.;
  for (idx_t a = 0; a < STRAIN_COUNT; a++)
  {
    stress[a] = 0.;
    for (idx_t b = 0; b < STRAIN_COUNT; b++)
      stress[a] += materialK_(a, b) * strain[b];
    energy += 0.5 * strain[a] * stress[a];
  }

  for (idx_t i = 0; i < GLOB_RANK; i++)
    for (idx_t j = 0; j < GLOB_RANK; j++)
      S(i, j) = stress[VOIGT[i][j]];

  return energy;
}

double SaintVenantKirchhoffSolidMaterial::calcStress_(const Matrix &P, const Matrix &F) const
{
  const double energy = calcSecondPK_(secondPKWork_, F);

  // P = F S; mul3 forms the product first, so P may be the same array as F
  mul3(P, F, secondPKWork_);

  return energy;
}

void SaintVenantKirchhoffSolidMaterial::calcTangent_(const Matrix &A, const Matrix &F) const
{
  calcSecondPK_(secondPKWork_, F);

  // dP_iJ / dF_kL = delta_ik S_JL + F_iM C_MJNL F_kN
  for (idx_t J = 0; J < GLOB_RANK; J++)
  {
    for (idx_t L = 0; L < GLOB_RANK; L++)
    {
      for (idx_t i = 0; i < GLOB_RANK; i++)
      {
        for (idx_t k = 0; k < GLOB_RANK; k++)
        {
          double sum = (i == k) ? secondPKWork_(J, L) : 0.;
          for (idx_t M = 0; M < GLOB_RANK; M++)
            for (idx_t N = 0; N < GLOB_RANK; N++)
              sum += F(i, M) * materialK_(VOIGT[M][J], VOIGT[N][L]) * F(k, N);
          A(i + GLOB_RANK * J, k + GLOB_RANK * L) = sum;
        }
      }
    }
  }
}

Ref<Material> SaintVenantKirchhoffSolidMaterial::makeNew(const String &name, const Properties &conf,
                                                         const Properties &props, const Properties &globdat)
{
  return newInstance<SaintVenantKirchhoffSolidMaterial>(name, conf, props, globdat);
}

void SaintVenantKirchhoffSolidMaterial::declare()
{
  MaterialFactory::declare(TYPE_NAME, &makeNew);
  MaterialFactory::declare(CLASS_NAME, &makeNew);
}
