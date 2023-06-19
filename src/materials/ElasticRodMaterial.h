/**
 * @file ElasticRodMaterial.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief simple implementation of the linear elastic material laws
 * @version 0.1
 * @date 2023-05-31
 *
 * @copyright Copyright (C) 2023 TU Delft. All rights reserved.
 *
 */

#pragma once
#include "materials/Material.h"
#include "materials/MaterialFactory.h"
#include "utils/helpers.h"
#include <jem/base/Array.h>
#include <jem/base/IllegalInputException.h>
#include <jem/base/System.h>
#include <jem/numeric/algebra/matmul.h>
#include <jem/util/StringUtils.h>
#include <math.h>

using jem::idx_t;
using jem::newInstance;
using jem::numeric::matmul;
using jive::IdxVector;
using jive::Ref;
using jive::Vector;
using jive::util::XTable;

class ElasticRodMaterial : public Material
{
public:
  static const char *TYPE_NAME;
  static const char *YOUNGS_MODULUS;
  static const char *SHEAR_MODULUS;
  static const char *POISSON_RATIO;
  static const char *AREA;
  static const char *DENSITY;
  static const char *AREA_MOMENT;
  static const char *SHEAR_FACTOR;
  static const char *POLAR_MOMENT;
  static const char *CROSS_SECTION;
  static const char *RADIUS;
  static const char *SIDE_LENGTH;

  JEM_DECLARE_CLASS(ElasticRodMaterial, Material);

  ElasticRodMaterial(const String &name,
                     const Properties &conf,
                     const Properties &props,
                     const Properties &globdat);

  static Ref<Material> makeNew(const String &name, const Properties &conf,
                               const Properties &props, const Properties &globdat);

  static void declare();

  virtual void configure(const Properties &props, const Properties &globdat) override;

  virtual void getConfig(const Properties &conf, const Properties &globdat) const override;

  virtual inline Matrix getMaterialStiff() const override;

  virtual inline Matrix getMaterialMass() const override;

  virtual Matrix getLumpedMass(double l) const override;

  virtual inline void getStress(const Vector &stress, const Vector &strain) const override;

  virtual inline void getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const override;

  virtual inline void update(const Vector &strain, const idx_t &ielem, const idx_t &ip) override;

protected:
  ~ElasticRodMaterial();

protected:
  void calcMaterialStiff_();
  void calcMaterialMass_();

  double young_;
  double shearMod_;
  double shearParam_;

  double area_;
  Vector areaMoment_;
  double polarMoment_;

  String cross_section_;
  double radius_;
  Vector side_length_;

  double density_;

  Matrix materialK_;
  Matrix materialM_;

  String rodName_;
};

Matrix ElasticRodMaterial::getMaterialStiff() const
{
  return materialK_;
}

Matrix ElasticRodMaterial::getMaterialMass() const
{
  return materialM_;
}

void ElasticRodMaterial::getStress(const Vector &stress, const Vector &strain) const
{
  stress = matmul(materialK_, strain);
}

void ElasticRodMaterial::getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const
{
  WARN(name + " not supported by this material");
}

void ElasticRodMaterial::update(const Vector &strain, const idx_t &ielem, const idx_t &ip)
{
}