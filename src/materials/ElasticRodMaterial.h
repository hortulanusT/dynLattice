/**
 * @file ElasticRodMaterial.h
 * @author Til Gärtner
 * @brief simple implementation of the linear elastic material laws
 *
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
#include <jive/util/DofSpace.h>
#include <jive/util/ObjectConverter.h>
#include <math.h>

using jem::idx_t;
using jem::newInstance;
using jem::numeric::matmul;
using jive::IdxVector;
using jive::Ref;
using jive::Vector;
using jive::util::DofSpace;
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
  static const char *N_ELEM;
  static const char *EDGE_FACTOR;
  static const char *EDGE_ELEMS;

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

  virtual Matrix getMaterialStiff() const override;

  virtual Matrix getMaterialStiff(const idx_t &ielem, const idx_t &ip) const override;

  virtual Matrix getMaterialMass() const override;

  virtual Matrix getMaterialMass(const idx_t &ielem, const idx_t &ip) const override;

  virtual Matrix getLumpedMass(const double l) const override;

  virtual Matrix getLumpedMass(const double l, const idx_t &ielem) const override;

  virtual void getStress(const Vector &stress, const Vector &strain) override;

  virtual void getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic = false) override;

  virtual void getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const override;

  virtual void apply_deform() override;

  virtual void reject_deform() override;

  virtual double getDissipatedEnergy(const idx_t &ielem, const idx_t &ip) const override;

  virtual double getPotentialEnergy(const idx_t &ielem, const idx_t &ip) const override;

  virtual double getHardeningPotential(const idx_t &ielem, const idx_t &ip) const override;

protected:
  ~ElasticRodMaterial();

protected:
  void calcMaterialStiff_();
  void calcMaterialMass_();

  double young_;
  double shearMod_;
  double shearParam_;

  double edgeFact_;
  idx_t edgeElems_;
  idx_t nElem_;

  double area_;
  Vector areaMoment_;
  double polarMoment_;

  String cross_section_;
  double radius_;
  Vector side_length_;

  double density_;

  Matrix materialK_;
  Matrix materialM_;

  Cubix old_Strains_;  // last converged load step
  Cubix curr_Strains_; // current load step iteration
  Matrix E_pot_;       // potential energy

  String rodName_;
};
