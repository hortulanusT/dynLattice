/**
 * @file ElasticRodMaterial.h
 * @author Til Gärtner
 * @brief Linear elastic rod material implementation with cross-sectional properties
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

/// @brief Linear elastic rod material with configurable cross-sectional properties
class ElasticRodMaterial : public Material
{
public:
  /// @name Material type and property identifiers
  /// @{
  static const char *TYPE_NAME;
  static const char *YOUNGS_MODULUS; ///< Young's modulus property key
  static const char *SHEAR_MODULUS;  ///< Shear modulus property key
  static const char *POISSON_RATIO;  ///< Poisson's ratio property key
  static const char *AREA;           ///< Cross-sectional area property key
  static const char *DENSITY;        ///< Material density property key
  static const char *AREA_MOMENT;    ///< Area moment of inertia property key
  static const char *SHEAR_FACTOR;   ///< Shear correction factor property key
  static const char *POLAR_MOMENT;   ///< Polar moment of inertia property key
  static const char *CROSS_SECTION;  ///< Cross-section type property key (e.g., "square", "circle" "rectangle")
  static const char *RADIUS;         ///< Radius for circular sections property key
  static const char *SIDE_LENGTH;    ///< Side lengths for rectangular sections property key
  /// @}
  /// @name Element and edge properties
  /// @{
  static const char *N_ELEM;      ///< Number of elements property key
  static const char *EDGE_FACTOR; ///< Edge factor for lattice materials property key
  static const char *EDGE_ELEMS;  ///< Number of edge elements property key
  /// @}

  JEM_DECLARE_CLASS(ElasticRodMaterial, Material);

  /// @brief Constructor with configuration and global data
  ElasticRodMaterial(const String &name,
                     const Properties &conf,
                     const Properties &props,
                     const Properties &globdat);

  /// @brief Factory method for material creation
  static Ref<Material> makeNew(const String &name, const Properties &conf,
                               const Properties &props, const Properties &globdat);

  /// @brief Register material type with factory
  static void declare();

  /// @brief Configure material properties including cross-sectional geometry
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
  /// @brief Calculate material stiffness matrix
  void calcMaterialStiff_();
  /// @brief Calculate material mass matrix
  void calcMaterialMass_();

  /// @name Material properties
  /// @{
  double young_;      ///< Young's modulus
  double shearMod_;   ///< Shear modulus
  double shearParam_; ///< Shear parameter

  double edgeFact_; ///< Edge factor for lattice materials
  idx_t edgeElems_; ///< Number of edge elements
  idx_t nElem_;     ///< Total number of elements

  double area_;        ///< Cross-sectional area
  Vector areaMoment_;  ///< Area moments of inertia
  double polarMoment_; ///< Polar moment of inertia

  String cross_section_; ///< Cross-section type identifier
  double radius_;        ///< Radius for circular sections
  Vector side_length_;   ///< Side lengths for rectangular sections

  double density_; ///< Material density
  /// @}

  /// @name Computed matrices and state
  /// @{
  Matrix materialK_; ///< Material stiffness matrix
  Matrix materialM_; ///< Material mass matrix

  Cubix old_Strains_;  ///< Strains from last converged step
  Cubix curr_Strains_; ///< Strains from current iteration
  Matrix E_pot_;       ///< Potential energy storage

  String rodName_; ///< Material instance name
  /// @}
};
