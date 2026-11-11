/**
 * @file SolidMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of the common base class of the solid materials
 */
#include "materials/SolidMaterial.h"

#include <jem/base/ClassTemplate.h>
#include <jem/base/IllegalInputException.h>
#include <jem/base/System.h>
#include <jem/base/Tuple.h>
#include <jem/numeric/algebra.h>
#include <jem/numeric/algebra/Cholesky.h>
#include <jem/numeric/algebra/LUSolver.h>
#include <jive/util/ObjectConverter.h>

using jem::IllegalInputException;
using jem::ALL;
using jem::newInstance;
using jem::numeric::det;
using jem::numeric::dotProduct;
using jem::numeric::inverse;
using jem::numeric::norm2;

JEM_DEFINE_CLASS(SolidMaterial);

const char *SolidMaterial::DENSITY = "density";

const int SolidMaterial::STRAIN_COUNT;
const int SolidMaterial::GLOB_RANK;

const idx_t SolidMaterial::VOIGT[GLOB_RANK][GLOB_RANK] = {{0, 5, 4}, {5, 1, 3}, {4, 3, 2}};
const idx_t SolidMaterial::VOIGT_I[STRAIN_COUNT] = {0, 1, 2, 1, 0, 0};
const idx_t SolidMaterial::VOIGT_J[STRAIN_COUNT] = {0, 1, 2, 2, 2, 1};

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

  materialM_.resize(GLOB_RANK, GLOB_RANK);
  materialM_ = 0.;
  for (idx_t i = 0; i < GLOB_RANK; i++)
    materialM_(i, i) = density_;

  idx_t ipCount;
  idx_t elemCount;
  readCounts_(myProps, ipCount, elemCount);

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

void SolidMaterial::getLame_(const Properties &myProps, double &mu, double &lambda) const
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

Matrix SolidMaterial::makeStiffness_(const Properties &myProps) const
{
  Matrix C(STRAIN_COUNT, STRAIN_COUNT);

  if (readDirectStiffness_(C, myProps))
    return C;

  String symmetry = "isotropic";
  myProps.find(symmetry, "symmetry");

  if (symmetry == "isotropic")
  {
    double mu, lambda;
    getLame_(myProps, mu, lambda);

    C = 0.;
    for (idx_t i = 0; i < GLOB_RANK; i++)
      for (idx_t j = 0; j < GLOB_RANK; j++)
        C(i, j) = (i == j) ? lambda + 2. * mu : lambda;
    for (idx_t i = GLOB_RANK; i < STRAIN_COUNT; i++)
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

  Matrix R(GLOB_RANK, GLOB_RANK);
  readOrientation_(R, myProps);
  rotateStiffness_(C, R);

  return C;
}

Matrix SolidMaterial::readStiffness_(const Properties &props, const Properties &globdat) const
{
  Properties myProps = props.getProps(myName_);
  myProps.setConverter(newInstance<jive::util::ObjConverter>(globdat));

  Matrix stiff = makeStiffness_(myProps);

  if (verbosity_ > 0)
    jem::System::debug(myName_)
        << " ...Stiffness matrix of the material '" << myName_ << "':\n"
        << stiff << "\n";

  return stiff;
}

void SolidMaterial::writeStiffness_(const Properties &conf, const Matrix &stiff) const
{
  Properties myConf = conf.makeProps(myName_);

  for (idx_t i = 0; i < STRAIN_COUNT; i++)
    for (idx_t j = i; j < STRAIN_COUNT; j++)
      myConf.set("C_" + String(i + 1) + String(j + 1), stiff(i, j));
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

  for (idx_t i = 0; i < STRAIN_COUNT; i++)
  {
    for (idx_t j = i; j < STRAIN_COUNT; j++)
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

  // jem factorises in place, hence the copy; fails for a matrix that is not positive definite (or has NaN)
  Matrix L = C.clone();
  if (!jem::numeric::Cholesky::factor(L))
    throw IllegalInputException(getContext(), "the stiffness 'C_ij' is not positive definite");

  return true;
}

void SolidMaterial::readOrientation_(const Matrix &R, const Properties &myProps) const
{
  Vector dir1(GLOB_RANK), dir2(GLOB_RANK);
  dir1 = 0.;
  dir2 = 0.;
  dir1[0] = 1.;
  dir2[1] = 1.;

  myProps.find(dir1, "dir1");
  const bool hasDir2 = myProps.find(dir2, "dir2");

  if (dir1.size() != GLOB_RANK || dir2.size() != GLOB_RANK)
    throw IllegalInputException(getContext(), "'dir1' and 'dir2' must have 3 components");

  const double length1 = norm2(dir1);
  if (!(length1 > 0.))
    throw IllegalInputException(getContext(), "'dir1' must not be zero");

  // first axis: dir1, second axis: the part of dir2 perpendicular to it
  Vector a1(GLOB_RANK), a2(GLOB_RANK);
  a1 = dir1 / length1;
  a2 = dir2 - dotProduct(dir2, a1) * a1;

  // without dir2, the y axis is used, or the x axis if the y axis is parallel to dir1
  if (!hasDir2 && !(norm2(a2) > 1.e-12))
  {
    dir2 = 0.;
    dir2[0] = 1.;
    a2 = dir2 - dotProduct(dir2, a1) * a1;
  }

  const double length2 = norm2(dir2);
  if (!(length2 > 0.))
    throw IllegalInputException(getContext(), "'dir2' must not be zero");

  // relative to the length of dir2: the result must not depend on the scale of the input
  const double perp = norm2(a2);
  if (!(perp > 1.e-12 * length2))
    throw IllegalInputException(getContext(), "'dir1' and 'dir2' must not be parallel");
  a2 /= perp;

  // third axis: cross product
  R(ALL, 0) = a1;
  R(ALL, 1) = a2;
  R(0, 2) = a1[1] * a2[2] - a1[2] * a2[1];
  R(1, 2) = a1[2] * a2[0] - a1[0] * a2[2];
  R(2, 2) = a1[0] * a2[1] - a1[1] * a2[0];
}

Matrix SolidMaterial::orthotropicStiffness_(double E1, double E2, double E3,
                                            double nu12, double nu13, double nu23,
                                            double G12, double G13, double G23) const
{
  if (E1 <= 0. || E2 <= 0. || E3 <= 0. || G12 <= 0. || G13 <= 0. || G23 <= 0.)
    throw IllegalInputException(getContext(), "Young's and shear moduli must be positive");

  // compliance of the normal part
  jem::Tuple<double, GLOB_RANK, GLOB_RANK> S;
  S(0, 0) = 1. / E1;
  S(1, 1) = 1. / E2;
  S(2, 2) = 1. / E3;
  S(0, 1) = S(1, 0) = -nu12 / E1;
  S(0, 2) = S(2, 0) = -nu13 / E1;
  S(1, 2) = S(2, 1) = -nu23 / E2;

  // positive definite if all leading minors are positive
  const double minor2 = S(0, 0) * S(1, 1) - S(0, 1) * S(1, 0);
  if (minor2 <= 0. || det(S) <= 0.)
    throw IllegalInputException(getContext(), "elastic constants do not give a positive definite stiffness");

  jem::Tuple<double, GLOB_RANK, GLOB_RANK> Cn = inverse(S);

  Matrix C(STRAIN_COUNT, STRAIN_COUNT);
  C = 0.;
  for (idx_t i = 0; i < GLOB_RANK; i++)
    for (idx_t j = 0; j < GLOB_RANK; j++)
      C(i, j) = Cn(i, j);

  C(3, 3) = G23;
  C(4, 4) = G13;
  C(5, 5) = G12;

  return C;
}

void SolidMaterial::rotateStiffness_(const Matrix &C, const Matrix &R)
{
  Matrix T(STRAIN_COUNT, STRAIN_COUNT);

  for (idx_t p = 0; p < STRAIN_COUNT; p++)
  {
    idx_t i = VOIGT_I[p], j = VOIGT_J[p];
    for (idx_t q = 0; q < STRAIN_COUNT; q++)
    {
      idx_t k = VOIGT_I[q], l = VOIGT_J[q];
      T(p, q) = (q < GLOB_RANK) ? R(i, k) * R(j, k)
                                : R(i, k) * R(j, l) + R(i, l) * R(j, k);
    }
  }

  // C <- T C T^T
  Matrix TC(STRAIN_COUNT, STRAIN_COUNT);
  jem::numeric::matmul(TC, T, C);
  jem::numeric::matmul(C, TC, T.transpose());
}
