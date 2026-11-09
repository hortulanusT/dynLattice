/**
 * @file SolidMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of the common base class of the solid materials
 */
#include "materials/SolidMaterial.h"

#include <jem/base/ClassTemplate.h>
#include <jem/base/IllegalInputException.h>
#include <jive/util/ObjectConverter.h>

#include <math.h>

using jem::IllegalInputException;
using jem::newInstance;

JEM_DEFINE_CLASS(SolidMaterial);

const char *SolidMaterial::DENSITY = "density";

const idx_t SolidMaterial::VOIGT[3][3] = {{0, 5, 4}, {5, 1, 3}, {4, 3, 2}};
const idx_t SolidMaterial::VOIGT_I[6] = {0, 1, 2, 1, 0, 0};
const idx_t SolidMaterial::VOIGT_J[6] = {0, 1, 2, 2, 2, 1};

bool SolidMaterial::isPositiveDefinite_(const Matrix &C)
{
  double L[6][6];

  for (idx_t i = 0; i < 6; i++)
  {
    for (idx_t j = 0; j <= i; j++)
    {
      double sum = C(i, j);
      for (idx_t k = 0; k < j; k++)
        sum -= L[i][k] * L[j][k];

      if (i == j)
      {
        if (!(sum > 0.)) // also catches NaN
          return false;
        L[i][i] = sqrt(sum);
      }
      else
      {
        L[i][j] = sum / L[j][j];
      }
    }
  }

  return true;
}

bool SolidMaterial::readDirectStiffness_(const Matrix &C, const Properties &myProps) const
{
  if (!myProps.contains("C_11"))
    return false;

  // the other stiffness input would be silently ignored
  static const char *OTHER_INPUT[] = {"symmetry", "young", "poisson_ratio", "shear_modulus", "young_1"};
  for (const char *key : OTHER_INPUT)
  {
    if (myProps.contains(key))
      throw IllegalInputException(
          getContext(), String("'") + key + "' given together with 'C_11': specify either the entries "
                                            "'C_ij' or the elastic constants, not both");
  }

  C = 0.;
  for (idx_t i = 0; i < 6; i++)
  {
    for (idx_t j = i; j < 6; j++)
    {
      String key = "C_" + String(i + 1) + String(j + 1);
      double value = 0.;

      if (i == j)
        myProps.get(value, key);
      else
        myProps.find(value, key);

      C(i, j) = value;
      C(j, i) = value;
    }
  }

  if (!isPositiveDefinite_(C))
    throw IllegalInputException(getContext(), "the stiffness 'C_ij' is not positive definite");

  return true;
}

void SolidMaterial::readOrientation_(const Matrix &R, const Properties &myProps) const
{
  Vector dir1(3), dir2(3);
  dir1 = 0.;
  dir2 = 0.;
  dir1[0] = 1.;
  dir2[1] = 1.;

  myProps.find(dir1, "dir1");
  if (!myProps.find(dir2, "dir2") && dir1.size() == 3 && dir1[0] == 0. && dir1[2] == 0.)
  {
    // default dir2 would be parallel to dir1
    dir2[0] = 1.;
    dir2[1] = 0.;
  }

  if (dir1.size() != 3 || dir2.size() != 3)
    throw IllegalInputException(getContext(), "'dir1' and 'dir2' must have 3 components");

  double norm1 = sqrt(dir1[0] * dir1[0] + dir1[1] * dir1[1] + dir1[2] * dir1[2]);
  if (!(norm1 > 0.))
    throw IllegalInputException(getContext(), "'dir1' must not be zero");
  double length2 = sqrt(dir2[0] * dir2[0] + dir2[1] * dir2[1] + dir2[2] * dir2[2]);
  if (!(length2 > 0.))
    throw IllegalInputException(getContext(), "'dir2' must not be zero");
  for (idx_t i = 0; i < 3; i++)
    R(i, 0) = dir1[i] / norm1;

  double proj = dir2[0] * R(0, 0) + dir2[1] * R(1, 0) + dir2[2] * R(2, 0);
  double norm2 = 0.;
  for (idx_t i = 0; i < 3; i++)
  {
    R(i, 1) = dir2[i] - proj * R(i, 0);
    norm2 += R(i, 1) * R(i, 1);
  }
  norm2 = sqrt(norm2);
  // relative to the length of dir2: the result must not depend on the scale of the input
  if (!(norm2 > 1.e-12 * length2))
    throw IllegalInputException(getContext(), "'dir1' and 'dir2' must not be parallel");
  for (idx_t i = 0; i < 3; i++)
    R(i, 1) /= norm2;

  R(0, 2) = R(1, 0) * R(2, 1) - R(2, 0) * R(1, 1);
  R(1, 2) = R(2, 0) * R(0, 1) - R(0, 0) * R(2, 1);
  R(2, 2) = R(0, 0) * R(1, 1) - R(1, 0) * R(0, 1);
}


Matrix SolidMaterial::orthotropicStiffness_(double E1, double E2, double E3,
                                            double nu12, double nu13, double nu23,
                                            double G12, double G13, double G23) const
{
  if (E1 <= 0. || E2 <= 0. || E3 <= 0. || G12 <= 0. || G13 <= 0. || G23 <= 0.)
    throw IllegalInputException(getContext(), "Young's and shear moduli must be positive");

  double S[3][3] = {{1. / E1, -nu12 / E1, -nu13 / E1},
                    {-nu12 / E1, 1. / E2, -nu23 / E2},
                    {-nu13 / E1, -nu23 / E2, 1. / E3}};

  double minor2 = S[0][0] * S[1][1] - S[0][1] * S[1][0];
  double det = S[0][0] * (S[1][1] * S[2][2] - S[1][2] * S[2][1]) -
               S[0][1] * (S[1][0] * S[2][2] - S[1][2] * S[2][0]) +
               S[0][2] * (S[1][0] * S[2][1] - S[1][1] * S[2][0]);

  if (minor2 <= 0. || det <= 0.)
    throw IllegalInputException(getContext(), "elastic constants do not give a positive definite stiffness");

  Matrix C(6, 6);
  C = 0.;

  // inverse of the symmetric 3x3 block via the adjugate
  for (idx_t i = 0; i < 3; i++)
  {
    for (idx_t j = 0; j < 3; j++)
    {
      idx_t i1 = (i + 1) % 3, i2 = (i + 2) % 3;
      idx_t j1 = (j + 1) % 3, j2 = (j + 2) % 3;
      C(j, i) = (S[i1][j1] * S[i2][j2] - S[i1][j2] * S[i2][j1]) / det;
    }
  }

  C(3, 3) = G23;
  C(4, 4) = G13;
  C(5, 5) = G12;

  return C;
}


void SolidMaterial::rotateStiffness_(const Matrix &C, const Matrix &R)
{
  double T[6][6];

  for (idx_t p = 0; p < 6; p++)
  {
    idx_t i = VOIGT_I[p], j = VOIGT_J[p];
    for (idx_t q = 0; q < 6; q++)
    {
      idx_t k = VOIGT_I[q], l = VOIGT_J[q];
      T[p][q] = (q < 3) ? R(i, k) * R(j, k)
                        : R(i, k) * R(j, l) + R(i, l) * R(j, k);
    }
  }

  double TC[6][6];
  for (idx_t p = 0; p < 6; p++)
    for (idx_t q = 0; q < 6; q++)
    {
      TC[p][q] = 0.;
      for (idx_t r = 0; r < 6; r++)
        TC[p][q] += T[p][r] * C(r, q);
    }

  for (idx_t p = 0; p < 6; p++)
    for (idx_t q = 0; q < 6; q++)
    {
      double sum = 0.;
      for (idx_t r = 0; r < 6; r++)
        sum += TC[p][r] * T[q][r];
      C(p, q) = sum;
    }
}

SolidMaterial::SolidMaterial(const String &name,
                             const Properties &conf,
                             const Properties &props,
                             const Properties &globdat) : Super(name, conf, props, globdat)
{
}

SolidMaterial::~SolidMaterial()
{
}

void SolidMaterial::configure(const Properties &props, const Properties &globdat)
{
  Properties myProps = props.getProps(myName_);
  myProps.setConverter(newInstance<jive::util::ObjConverter>(globdat));

  density_ = 0.;
  myProps.find(density_, DENSITY);
  if (density_ < 0.0)
    throw IllegalInputException(
        getContext(), "Density cannot be negative, got " + String(density_));

  materialM_.resize(3, 3);
  materialM_ = 0.;
  for (idx_t i = 0; i < 3; i++)
    materialM_(i, i) = density_;

  // set by the model: they size the energy storage
  idx_t ipCount;
  idx_t elemCount;
  myProps.get(ipCount, "ipCount");
  myProps.get(elemCount, "elemCount");

  if (ipCount <= 0 || elemCount < 0)
    throw IllegalInputException(
        getContext(), "'ipCount' has to be positive and 'elemCount' non-negative, got " +
                          String(ipCount) + " and " + String(elemCount));

  energyPot_.resize(ipCount, elemCount);
  energyPot_ = 0.;
  currEnergy_.resize(ipCount, elemCount);
  currEnergy_ = 0.;
}

void SolidMaterial::getConfig(const Properties &conf, const Properties &) const
{
  conf.makeProps(myName_).set(DENSITY, density_);
}

Matrix SolidMaterial::getMaterialMass() const
{
  return materialM_.clone();
}

void SolidMaterial::getMaterialMass(const Matrix &mass, const idx_t &, const idx_t &) const
{
  mass = materialM_;
}

void SolidMaterial::applyDeform()
{
  energyPot_ = currEnergy_;
}

void SolidMaterial::rejectDeform()
{
  currEnergy_ = energyPot_;
}

double SolidMaterial::getPotentialEnergy(const idx_t &ielem, const idx_t &ip) const
{
  return energyPot_(ip, ielem);
}

void SolidMaterial::getLame(const Properties &myProps, double &mu, double &lambda) const
{
  double young;
  myProps.get(young, "young");

  if (!(young > 0.0))
    throw IllegalInputException(
        getContext(), "Young's modulus must be positive, got " + String(young));

  double nu;
  const bool hasShear = myProps.contains("shear_modulus");
  const bool hasPoisson = myProps.contains("poisson_ratio");

  if (hasShear && hasPoisson)
    throw IllegalInputException(
        getContext(), "'shear_modulus' and 'poisson_ratio' given together with 'young': "
                      "the isotropic material is over-determined, give only one of them");

  if (hasShear)
  {
    myProps.get(mu, "shear_modulus");

    if (!(mu > 0.0))
      throw IllegalInputException(
          getContext(), "shear modulus must be positive, got " + String(mu));

    nu = young / 2. / mu - 1.;
  }
  else
  {
    myProps.get(nu, "poisson_ratio");
    mu = young / 2. / (nu + 1.);
  }

  if (!(nu > -1.0 && nu < 0.5))
    throw IllegalInputException(
        getContext(), "Poisson's ratio must be in range (-1, 0.5), got " + String(nu));

  lambda = young * nu / ((1. + nu) * (1. - 2. * nu));
}

Matrix SolidMaterial::makeStiffness(const Properties &myProps) const
{
  Matrix C(6, 6);

  if (readDirectStiffness_(C, myProps))
    return C;

  String symmetry = "isotropic";
  myProps.find(symmetry, "symmetry");

  if (symmetry == "isotropic")
  {
    double mu, lambda;
    getLame(myProps, mu, lambda);

    C = 0.;
    for (idx_t i = 0; i < 3; i++)
      for (idx_t j = 0; j < 3; j++)
        C(i, j) = (i == j) ? lambda + 2. * mu : lambda;
    for (idx_t i = 3; i < 6; i++)
      C(i, i) = mu;

    return C;
  }

  if (symmetry == "transversely_isotropic")
  {
    double E1, E2, nu12, nu23, G12;
    myProps.get(E1, "young_1");
    myProps.get(E2, "young_2");
    myProps.get(nu12, "poisson_12");
    myProps.get(nu23, "poisson_23");
    myProps.get(G12, "shear_12");

    if (E2 <= 0. || nu23 <= -1.)
      throw IllegalInputException(getContext(), "invalid transverse elastic constants");

    C = orthotropicStiffness_(E1, E2, E2, nu12, nu12, nu23,
                              G12, G12, E2 / 2. / (1. + nu23));
  }
  else if (symmetry == "orthotropic")
  {
    double E1, E2, E3, nu12, nu13, nu23, G12, G13, G23;
    myProps.get(E1, "young_1");
    myProps.get(E2, "young_2");
    myProps.get(E3, "young_3");
    myProps.get(nu12, "poisson_12");
    myProps.get(nu13, "poisson_13");
    myProps.get(nu23, "poisson_23");
    myProps.get(G12, "shear_12");
    myProps.get(G13, "shear_13");
    myProps.get(G23, "shear_23");

    C = orthotropicStiffness_(E1, E2, E3, nu12, nu13, nu23, G12, G13, G23);
  }
  else
  {
    throw IllegalInputException(
        getContext(), "unknown symmetry '" + symmetry +
                     "', only 'isotropic', 'transversely_isotropic' and 'orthotropic' are supported");
  }

  Matrix R(3, 3);
  readOrientation_(R, myProps);
  rotateStiffness_(C, R);

  return C;
}
