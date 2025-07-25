/**
 * @file ElasticRodMaterial.cpp
 * @author Til Gärtner
 * @brief Implementation of linear elastic rod material with cross-sectional properties
 */
#include "materials/ElasticRodMaterial.h"

#include "materials/MaterialFactory.h"
#include "utils/helpers.h"
#include "utils/testing.h"

#include <jem/base/Array.h>
#include <jem/base/ClassTemplate.h>
#include <jem/base/IllegalInputException.h>
#include <jem/base/System.h>
#include <jem/numeric/algebra/matmul.h>
#include <jem/util/StringUtils.h>

#include <jive/util/DofSpace.h>
#include <jive/util/ObjectConverter.h>

#include <math.h>

using jem::newInstance;
using jem::numeric::matmul;
using jive::Ref;
using jive::Vector;

JEM_DEFINE_CLASS(ElasticRodMaterial);

const char *ElasticRodMaterial::TYPE_NAME = "ElasticRod";
const char *ElasticRodMaterial::YOUNGS_MODULUS = "young";
const char *ElasticRodMaterial::SHEAR_MODULUS = "shear_modulus";
const char *ElasticRodMaterial::POISSON_RATIO = "poisson_ratio";
const char *ElasticRodMaterial::AREA = "area";
const char *ElasticRodMaterial::DENSITY = "density";
const char *ElasticRodMaterial::AREA_MOMENT = "area_moment";
const char *ElasticRodMaterial::POLAR_MOMENT = "polar_moment";
const char *ElasticRodMaterial::SHEAR_FACTOR = "shear_correction";
const char *ElasticRodMaterial::CROSS_SECTION = "cross_section";
const char *ElasticRodMaterial::RADIUS = "radius";
const char *ElasticRodMaterial::SIDE_LENGTH = "side_length";
const char *ElasticRodMaterial::N_ELEM = "elemCount";
const char *ElasticRodMaterial::EDGE_FACTOR = "edge_factor";
const char *ElasticRodMaterial::EDGE_ELEMS = "edge_elements";

ElasticRodMaterial::ElasticRodMaterial(const String &name,
                                       const Properties &conf,
                                       const Properties &props,
                                       const Properties &globdat) : Super(name, conf, props, globdat)
{
  rodName_ = jem::util::StringUtils::split(myName_, '.')[0];
  edgeFact_ = 1.0;
  edgeElems_ = 1;

  configure(props, globdat);
  getConfig(conf, globdat);
}

ElasticRodMaterial::~ElasticRodMaterial()
{
}

void ElasticRodMaterial::configure(const Properties &props, const Properties &globdat)
{
  using jive::util::DofSpace;

  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());

  Properties myProps = props.getProps(myName_);
  myProps.setConverter(newInstance<jive::util::ObjConverter>(globdat));

  idx_t ipCount;
  idx_t elemCount;
  idx_t dofCount;
  myProps.find(ipCount, "ipCount");
  myProps.find(elemCount, "elemCount");
  dofCount = dofs->typeCount();

  myProps.get(young_, YOUNGS_MODULUS);

  // Validate Young's modulus
  if (young_ <= 0.0)
  {
    throw jem::IllegalInputException(
        getContext() + ": Young's modulus must be positive, got " + String(young_));
  }

  if (!myProps.find(shearMod_, SHEAR_MODULUS))
  {
    double nu;
    myProps.get(nu, POISSON_RATIO);

    // Validate Poisson's ratio
    if (nu <= -1.0 || nu >= 0.5)
    {
      throw jem::IllegalInputException(
          getContext() + ": Poisson's ratio must be in range (-1, 0.5), got " + String(nu));
    }

    shearMod_ = young_ / 2. / (nu + 1.);
  }
  else
  {
    // Validate shear modulus
    if (shearMod_ <= 0.0)
    {
      throw jem::IllegalInputException(
          getContext() + ": Shear modulus must be positive, got " + String(shearMod_));
    }
  }

  areaMoment_.resize(2);

  if (!myProps.find(crossSection_, CROSS_SECTION))
  {
    myProps.get(area_, AREA);
    myProps.get(areaMoment_, AREA_MOMENT);

    // Validate geometric properties
    if (area_ <= 0.0)
    {
      throw jem::IllegalInputException(
          getContext() + ": Cross-sectional area must be positive, got " + String(area_));
    }

    if (areaMoment_.size() == 1)
    {
      areaMoment_.reshape(2);
      areaMoment_[1] = areaMoment_[0];
    }

    // Validate area moments
    for (idx_t i = 0; i < areaMoment_.size(); ++i)
    {
      if (areaMoment_[i] <= 0.0)
      {
        throw jem::IllegalInputException(
            getContext() + ": Area moment of inertia must be positive, got " + String(areaMoment_[i]));
      }
    }

    shearParam_ = 5. / 6.; // standard square cross-section
  }
  else if (crossSection_ == "square")
  {
    myProps.get(sideLength_, SIDE_LENGTH);
    JEM_ASSERT2(sideLength_.size() == 1, "A square has only one side!");
    sideLength_.reshape(2);
    sideLength_[1] = sideLength_[0];

    area_ = pow(sideLength_[0], 2);
    areaMoment_[0] = areaMoment_[1] = pow(sideLength_[0], 4) / 12.;
    shearParam_ = 5. / 6.;

    crossSection_ = "rectangle";
  }
  else if (crossSection_ == "circle")
  {
    myProps.get(radius_, RADIUS);
    area_ = M_PI * pow(radius_, 2);
    areaMoment_ = M_PI * pow(radius_, 4) / 4.;
    shearParam_ = 9. / 10.;
  }
  else if (crossSection_ == "rectangle")
  {
    myProps.get(sideLength_, SIDE_LENGTH);
    JEM_ASSERT2(sideLength_.size() == 2,
                "A rectangle has only two sides!");
    area_ = jem::product(sideLength_);

    areaMoment_[0] = pow(sideLength_[1], 3) * sideLength_[0] / 12.;
    areaMoment_[1] = pow(sideLength_[0], 3) * sideLength_[1] / 12.;

    shearParam_ = 5. / 6.;
  }
  else
    throw jem::IllegalInputException(
        getContext(), "unknown cross section, only 'rectangle', 'square' and 'circle' "
                      "are supported");

  polarMoment_ = jem::sum(areaMoment_);
  myProps.find(shearParam_, SHEAR_FACTOR);
  myProps.find(polarMoment_, POLAR_MOMENT);

  density_ = 0.;
  myProps.find(density_, DENSITY);

  // Validate density
  if (density_ < 0.0)
  {
    throw jem::IllegalInputException(
        getContext() + ": Density cannot be negative, got " + String(density_));
  }

  calcMaterialStiff_();
  calcMaterialMass_();

  double inertiaCorrect;
  if (myProps.find(inertiaCorrect, "inertia_correct"))
    for (idx_t i = 3; i < 6; i++)
      materialM_(i, i) *= inertiaCorrect;

  if (myProps.find(edgeFact_, EDGE_FACTOR) && edgeFact_ != 1.)
    myProps.get(nElem_, N_ELEM);
  myProps.find(edgeElems_, EDGE_ELEMS);

  if (verbosity_ > 0)
    jem::System::debug(myName_)
        << " ...Stiffness matrix of the material '" << myName_ << "':\n"
        << materialK_ << "\n";
  if (density_ > 0.)
  {
    if (verbosity_ > 0)
      jem::System::debug(myName_)
          << " ...Inertia matrix of the material '" << myName_ << "':\n"
          << materialM_ << "\n";
  }

  oldStrains_.resize(dofCount, ipCount, elemCount);
  oldStrains_ = 0.;
  currStrains_.resize(dofCount, ipCount, elemCount);
  currStrains_ = 0.;

  energyPot_.resize(ipCount, elemCount);
  energyPot_ = 0.;
}

void ElasticRodMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Properties myConf = conf.makeProps(myName_);

  myConf.set(YOUNGS_MODULUS, young_);
  myConf.set(SHEAR_MODULUS, shearMod_);
  myConf.set(SHEAR_FACTOR, shearParam_);

  myConf.set(AREA, area_);
  myConf.set(AREA_MOMENT, areaMoment_);
  myConf.set(POLAR_MOMENT, polarMoment_);

  if (crossSection_ == "rectangle")
  {
    myConf.set(CROSS_SECTION, crossSection_);
    myConf.set(SIDE_LENGTH, sideLength_);
  }
  else if (crossSection_ == "circle")
  {
    myConf.set(CROSS_SECTION, crossSection_);
    myConf.set(RADIUS, radius_);
  }

  myConf.set(DENSITY, density_);

  if (edgeFact_ != 1.)
  {
    myConf.set(EDGE_FACTOR, edgeFact_);
    myConf.set(EDGE_ELEMS, edgeElems_);
    myConf.set(N_ELEM, nElem_);
  }
}

void ElasticRodMaterial::calcMaterialStiff_()
{
  materialK_.resize(6, 6);
  materialK_ = 0.0;
  materialK_(0, 0) = shearMod_ * shearParam_ * area_;
  materialK_(1, 1) = shearMod_ * shearParam_ * area_;
  materialK_(2, 2) = young_ * area_;
  materialK_(3, 3) = young_ * areaMoment_[0];
  materialK_(4, 4) = young_ * areaMoment_[1];
  materialK_(5, 5) = shearMod_ * polarMoment_;
}

void ElasticRodMaterial::calcMaterialMass_()
{
  materialM_.resize(6, 6);
  materialM_ = 0.0;
  materialM_(0, 0) = density_ * area_;
  materialM_(1, 1) = density_ * area_;
  materialM_(2, 2) = density_ * area_;
  materialM_(3, 3) = density_ * areaMoment_[0];
  materialM_(4, 4) = density_ * areaMoment_[1];
  materialM_(5, 5) = density_ * polarMoment_;
}

Matrix ElasticRodMaterial::getLumpedMass(const double l) const
{
  Matrix M = Matrix(getMaterialMass() * l);

  M(3, 3) += area_ * density_ * pow(l, 3) / 12.;
  M(4, 4) += area_ * density_ * pow(l, 3) / 12.;

  return M;
}

Matrix ElasticRodMaterial::getLumpedMass(const double l, const idx_t &ielem) const
{
  if (edgeFact_ == 1.)
    return getLumpedMass(l);
  else
  {
    if (ielem == 0 || ielem == nElem_ - 1)
      return Matrix(edgeFact_ * getLumpedMass(l));
    else
      return getLumpedMass(l);
  }
}

Matrix ElasticRodMaterial::getMaterialStiff() const
{
  return materialK_.clone();
}

Matrix ElasticRodMaterial::getMaterialStiff(const idx_t &ielem, const idx_t &ip) const
{
  if (edgeFact_ == 1.)
    return getMaterialStiff();
  else
  {
    if (ielem < edgeElems_ || ielem > nElem_ - edgeElems_ - 1)
    {
      Matrix stiff = getMaterialStiff();
      stiff(jem::SliceTo(3), jem::SliceTo(3)) *= pow(edgeFact_, 2);
      stiff(jem::SliceTo(3), jem::SliceFrom(3)) *= pow(edgeFact_, 3);
      stiff(jem::SliceFrom(3), jem::SliceTo(3)) *= pow(edgeFact_, 3);
      stiff(jem::SliceFrom(3), jem::SliceFrom(3)) *= pow(edgeFact_, 4);

      return stiff;
    }
    else
      return getMaterialStiff();
  }
}

Matrix ElasticRodMaterial::getMaterialMass() const
{
  return materialM_.clone();
}

Matrix ElasticRodMaterial::getMaterialMass(const idx_t &ielem, const idx_t &ip) const
{
  if (edgeFact_ == 1.)
    return getMaterialMass();
  else
  {
    if (ielem < edgeElems_ || ielem > nElem_ - edgeElems_ - 1)
    {
      Matrix mass = getMaterialMass();
      mass(jem::SliceTo(3), jem::SliceTo(3)) *= pow(2. - edgeFact_, 2);
      mass(jem::SliceTo(3), jem::SliceFrom(3)) *= pow(2. - edgeFact_, 3);
      mass(jem::SliceFrom(3), jem::SliceTo(3)) *= pow(2. - edgeFact_, 3);
      mass(jem::SliceFrom(3), jem::SliceFrom(3)) *= pow(2. - edgeFact_, 4);

      return mass;
    }
    else
      return getMaterialMass();
  }
}

void ElasticRodMaterial::getStress(const Vector &stress, const Vector &strain)
{
  stress = matmul(getMaterialStiff(), strain);
}

void ElasticRodMaterial::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic)
{
  currStrains_(ALL, ip, ielem) = strain;
  stress = matmul(getMaterialStiff(ielem, ip), strain);
}

void ElasticRodMaterial::getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const
{
  using jive::IdxVector;
  using jive::util::XTable;

  WARN(name + " not supported by this material");
}

void ElasticRodMaterial::applyDeform()
{

  for (idx_t ielem = 0; ielem < currStrains_.size(2); ielem++)
  {
    for (idx_t ip = 0; ip < currStrains_.size(1); ip++)
    {
      energyPot_(ip, ielem) = 0.5 * dotProduct(currStrains_(ALL, ip, ielem), matmul(getMaterialStiff(ielem, ip), currStrains_(ALL, ip, ielem)));
    }
  }

  oldStrains_ = currStrains_;
}

void ElasticRodMaterial::rejectDeform()
{
  currStrains_ = oldStrains_;
}

double ElasticRodMaterial::getPotentialEnergy(const idx_t &ielem, const idx_t &ip) const
{
  return energyPot_(ip, ielem);
}

double ElasticRodMaterial::getDissipatedEnergy(const idx_t &ielem, const idx_t &ip) const
{
  return 0.;
}

double ElasticRodMaterial::getHardeningPotential(const idx_t &ielem, const idx_t &ip) const
{
  return 0.;
}

Ref<Material> ElasticRodMaterial::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<ElasticRodMaterial>(name, conf, props, globdat);
}

void ElasticRodMaterial::declare()
{
  MaterialFactory::declare(TYPE_NAME, &makeNew);
  MaterialFactory::declare(CLASS_NAME, &makeNew);
}
