/**
 * @file SaintVenantKirchhoffSolidMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of the St. Venant-Kirchhoff solid
 */
#include "materials/SaintVenantKirchhoffSolidMaterial.h"

#include "materials/MaterialFactory.h"

#include <jem/base/ClassTemplate.h>
#include <jive/util/ObjectConverter.h>

using jem::newInstance;

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

  Properties myProps = props.getProps(myName_);
  myProps.setConverter(newInstance<jive::util::ObjConverter>(globdat));

  materialK_.resize(6, 6);
  materialK_ = makeStiffness(myProps);
}

void SaintVenantKirchhoffSolidMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Super::getConfig(conf, globdat);

  Properties myConf = conf.makeProps(myName_);

  for (idx_t i = 0; i < 6; i++)
    for (idx_t j = i; j < 6; j++)
      myConf.set("C_" + String(i + 1) + String(j + 1), materialK_(i, j));
}

double SaintVenantKirchhoffSolidMaterial::calcSecondPK_(double (&S)[3][3], const Matrix &F) const
{
  // Green-Lagrange strain in Voigt form (engineering shear)
  double strain[6];
  for (idx_t i = 0; i < 3; i++)
  {
    for (idx_t j = i; j < 3; j++)
    {
      double e = (i == j) ? -1. : 0.;
      for (idx_t k = 0; k < 3; k++)
        e += F(k, i) * F(k, j);
      strain[VOIGT[i][j]] = (i == j) ? 0.5 * e : e;
    }
  }

  double stress[6];
  double energy = 0.;
  for (idx_t a = 0; a < 6; a++)
  {
    stress[a] = 0.;
    for (idx_t b = 0; b < 6; b++)
      stress[a] += materialK_(a, b) * strain[b];
    energy += 0.5 * strain[a] * stress[a];
  }

  for (idx_t i = 0; i < 3; i++)
    for (idx_t j = 0; j < 3; j++)
      S[i][j] = stress[VOIGT[i][j]];

  return energy;
}

double SaintVenantKirchhoffSolidMaterial::calcStress_(const Matrix &P, const Matrix &F) const
{
  double S[3][3];
  const double energy = calcSecondPK_(S, F);

  // P = F S, collected first: P may be the same array as F
  double FS[3][3];
  for (idx_t i = 0; i < 3; i++)
  {
    for (idx_t J = 0; J < 3; J++)
    {
      double sum = 0.;
      for (idx_t K = 0; K < 3; K++)
        sum += F(i, K) * S[K][J];
      FS[i][J] = sum;
    }
  }

  for (idx_t i = 0; i < 3; i++)
    for (idx_t J = 0; J < 3; J++)
      P(i, J) = FS[i][J];

  return energy;
}

void SaintVenantKirchhoffSolidMaterial::calcTangent_(const Matrix &A, const Matrix &F) const
{
  double S[3][3];
  calcSecondPK_(S, F);

  // dP_iJ / dF_kL = delta_ik S_JL + F_iM C_MJNL F_kN
  for (idx_t J = 0; J < 3; J++)
  {
    for (idx_t L = 0; L < 3; L++)
    {
      for (idx_t i = 0; i < 3; i++)
      {
        for (idx_t k = 0; k < 3; k++)
        {
          double sum = (i == k) ? S[J][L] : 0.;
          for (idx_t M = 0; M < 3; M++)
            for (idx_t N = 0; N < 3; N++)
              sum += F(i, M) * materialK_(VOIGT[M][J], VOIGT[N][L]) * F(k, N);
          A(i + 3 * J, k + 3 * L) = sum;
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
