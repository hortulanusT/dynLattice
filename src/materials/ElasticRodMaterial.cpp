#include "materials/ElasticRodMaterial.h"
#include "utils/testing.h"
#include <jem/base/ClassTemplate.h>

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

ElasticRodMaterial::ElasticRodMaterial(const String &name,
                                       const Properties &conf,
                                       const Properties &props,
                                       const Properties &globdat) : Super(name, conf, props, globdat)
{
  configure(props);
  getConfig(conf);
}

ElasticRodMaterial::~ElasticRodMaterial()
{
}

void ElasticRodMaterial::configure(const Properties &props)
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

void ElasticRodMaterial::getConfig(const Properties &conf) const
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

Matrix ElasticRodMaterial::getLumpedMass(double l) const
{
  Matrix M = Matrix(getMaterialMass() * l);

  M(3, 3) += area_ * density_ * pow(l, 3) / 12.;
  M(4, 4) += area_ * density_ * pow(l, 3) / 12.;

  return M;
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
