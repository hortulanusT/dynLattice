#include "materials/ElasticMaterial.h"
#include "utils/testing.h"
#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(ElasticMaterial);

const char *ElasticMaterial::TYPE_NAME = "ElasticMat";
const char *ElasticMaterial::YOUNGS_MODULUS = "young";
const char *ElasticMaterial::SHEAR_MODULUS = "shear_modulus";
const char *ElasticMaterial::POISSON_RATIO = "poisson_ratio";
const char *ElasticMaterial::AREA = "area";
const char *ElasticMaterial::DENSITY = "density";
const char *ElasticMaterial::AREA_MOMENT = "area_moment";
const char *ElasticMaterial::POLAR_MOMENT = "polar_moment";
const char *ElasticMaterial::SHEAR_FACTOR = "shear_correction";
const char *ElasticMaterial::CROSS_SECTION = "cross_section";
const char *ElasticMaterial::RADIUS = "radius";
const char *ElasticMaterial::SIDE_LENGTH = "side_length";

ElasticMaterial::ElasticMaterial(const String &name,
                                 const Properties &conf,
                                 const Properties &props,
                                 const Properties &globdat) : Super(name, conf, props, globdat)
{
  configure(props);
  getConfig(conf);
}

ElasticMaterial::~ElasticMaterial()
{
}

void ElasticMaterial::configure(const Properties &props)
{
  Properties myProps = props.getProps(myName_);

  myProps.get(young_, YOUNGS_MODULUS);
  if (!myProps.find(shearMod_, SHEAR_MODULUS))
  {
    double nu;
    myProps.get(nu, POISSON_RATIO);
    shearMod_ = young_ / 2. / (nu + 1.);
  }

  areaMoment_.resize(2);

  if (!myProps.find(cross_section_, CROSS_SECTION))
  {
    myProps.get(area_, AREA);
    myProps.get(areaMoment_, AREA_MOMENT);
    if (areaMoment_.size() == 1)
    {
      areaMoment_.reshape(2);
      areaMoment_[1] = areaMoment_[0];
    }
    shearParam_ = 5. / 6.; // assume standard square cross-section
  }
  else if (cross_section_ == "square")
  {
    myProps.get(side_length_, SIDE_LENGTH);
    JEM_ASSERT2(side_length_.size() == 1, "A square has only one side!");
    side_length_.reshape(2);
    side_length_[1] = side_length_[0];

    area_ = pow(side_length_[0], 2);
    areaMoment_[0] = areaMoment_[1] = pow(side_length_[0], 4) / 12.;
    shearParam_ = 5. / 6.;

    cross_section_ = "rectangle";
  }
  else if (cross_section_ == "circle")
  {
    myProps.get(radius_, RADIUS);
    area_ = M_PI * pow(radius_, 2);
    areaMoment_ = M_PI * pow(radius_, 4) / 4.;
    shearParam_ = 9. / 10.;
  }
  else if (cross_section_ == "rectangle")
  {
    myProps.get(side_length_, SIDE_LENGTH);
    JEM_ASSERT2(side_length_.size() == 2,
                "A rectangle has only two sides!");
    area_ = jem::product(side_length_);

    areaMoment_[0] = pow(side_length_[1], 3) * side_length_[0] / 12.;
    areaMoment_[1] = pow(side_length_[0], 3) * side_length_[1] / 12.;

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

  calcMaterialStiff_();
  calcMaterialMass_();

  double inertiaCorrect;
  if (myProps.find(inertiaCorrect, "inertia_correct"))
    for (idx_t i = 3; i < 6; i++)
      materialM_(i, i) *= inertiaCorrect;

  jem::System::debug(myName_)
      << " ...Stiffness matrix of the material '" << myName_ << "':\n"
      << materialK_ << "\n";
  jem::System::debug(myName_)
      << " ...Inertia matrix of the material '" << myName_ << "':\n"
      << materialM_ << "\n";
}

void ElasticMaterial::getConfig(const Properties &conf) const
{
  Properties myConf = conf.getProps(myName_);

  myConf.set(YOUNGS_MODULUS, young_);
  myConf.set(SHEAR_MODULUS, shearMod_);
  myConf.set(SHEAR_FACTOR, shearParam_);

  myConf.set(AREA, area_);
  myConf.set(AREA_MOMENT, areaMoment_);
  myConf.set(POLAR_MOMENT, polarMoment_);

  if (cross_section_ == "rectangle")
  {
    myConf.set(CROSS_SECTION, cross_section_);
    myConf.set(SIDE_LENGTH, side_length_);
  }
  else if (cross_section_ == "circle")
  {
    myConf.set(CROSS_SECTION, cross_section_);
    myConf.set(RADIUS, radius_);
  }

  myConf.set(DENSITY, density_);
}

void ElasticMaterial::calcMaterialStiff_()
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

void ElasticMaterial::calcMaterialMass_()
{
  materialM_.resize(6, 6);
  materialM_ = 0.0;
  materialM_(0, 0) = density_ * area_;
  materialM_(1, 1) = density_ * area_;
  materialM_(2, 2) = density_ * area_;
  materialM_(3, 3) = 0;
  materialM_(4, 4) = 0;
  materialM_(5, 5) = density_ * polarMoment_;
}

Matrix ElasticMaterial::getMaterialStiff() const
{
  return materialK_.clone();
}

void ElasticMaterial::getStress(const Vector &stress, const Vector &strain) const
{
  stress = matmul(materialK_, strain);
}

void ElasticMaterial::getLumpedMaterialMass(const Matrix &M, const double l, const bool border) const
{
  double length = l;
  if (border)
    length = l / 2;
  M = materialM_.clone() * length;

  if (cross_section_ == "circle")
  {
    M(3, 3) = M(0, 0) * (3 * radius_ * radius_ + length * length) / 12.;
    M(4, 4) = M(0, 0) * (3 * radius_ * radius_ + length * length) / 12.;
  }
  else if (cross_section_ == "rectangle")
  {
    M(3, 3) = M(0, 0) * (side_length_[1] * side_length_[1] + length * length) / 12.;
    M(4, 4) = M(0, 0) * (side_length_[0] * side_length_[0] + length * length) / 12.;
  }

  if (border)
  {
    M(3, 3) += M(0, 0) * length / 2 * length / 2;
    M(4, 4) += M(0, 0) * length / 2 * length / 2;
  }
}

Ref<Material> ElasticMaterial::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<ElasticMaterial>(name, conf, props, globdat);
}

void ElasticMaterial::declare()
{
  MaterialFactory::declare(TYPE_NAME, &makeNew);
  MaterialFactory::declare(CLASS_NAME, &makeNew);
}
