/**
 * @file Material.h
 * @author Til Gärtner
 * @brief Material class for representing different materials in the simulation.
 *
 */
#pragma once

#include <jem/base/Array.h>
#include <jem/base/NamedObject.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/util/XTable.h>

using jem::NamedObject;
using jem::util::Properties;
using jive::idx_t;
using jive::IdxVector;
using jive::Matrix;
using jive::String;
using jive::Vector;
using jive::util::XTable;

/**
 * @brief Abstract base class for material models.
 *
 * Defines interface for material behavior including stress-strain relationships,
 * stiffness matrices, and energy calculations. Materials are created via
 * MaterialFactory and support multiple integration points per element.
 *
 * @section Material Hierarchy
 * - ElasticRodMaterial: Linear elastic material
 * - ElastoPlasticRodMaterial: Elasto-plastic with hardening
 *
 * @author Til Gärtner
 * @see ElasticRodMaterial, ElastoPlasticRodMaterial, MaterialFactory
 */
class Material : public NamedObject
{
public:
  JEM_DECLARE_CLASS(Material, NamedObject);

  /// Property name for verbosity level control
  const static char *VERBOSITY_PROP;

  /**
   * @brief Constructs a new Material object.
   *
   * @param name Material instance name for identification
   * @param conf Configuration properties container for output
   * @param props Input properties for material parameters
   * @param globdat Global data container for simulation state
   */
  Material(const String &name,
           const Properties &conf,
           const Properties &props,
           const Properties &globdat);

  /**
   * @brief Configure the material with given properties.
   *
   * @param props Properties containing configuration parameters
   * @param globdat Global data container with simulation context
   */
  virtual void configure(const Properties &props, const Properties &globdat);

  /**
   * @brief Retrieve current material configuration.
   *
   * @param conf Properties object to store configuration in
   * @param globdat Global data container with simulation context
   */
  virtual void getConfig(const Properties &conf, const Properties &globdat) const;

  /**
   * @brief Compute stress vector from strain vector.
   *
   * Fundamental constitutive relationship defining material response to deformation.
   * All derived material classes must implement this method.
   *
   * @param[out] stress Computed stress vector at current state
   * @param[in] strain Input strain vector for stress computation
   *
   * @note For rod elements: 6-dimensional vectors (axial, shear, bending, torsion)
   */
  virtual void getStress(const Vector &stress, const Vector &strain) = 0;

  /**
   * @brief Compute stress with element and integration point context.
   *
   * @param[out] stress Computed stress vector
   * @param[in] strain Input strain vector
   * @param[in] ielem Element index for state tracking
   * @param[in] ip Integration point index
   * @param[in] inelastic Enable inelastic behavior (default: true)
   *
   * @note Default delegates to getStress(stress, strain)
   * ElastoPlasticRodMaterial overrides for plastic state management
   */
  virtual inline void getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic = true);

  /**
   * @brief Get the material stiffness matrix.
   *
   * Computes the material stiffness matrix relating stress to strain.
   * This matrix represents the tangent modulus of the material at the current state.
   *
   * @returns Material stiffness matrix
   */
  virtual Matrix getMaterialStiff() const = 0;

  /**
   * @brief Get the material stiffness matrix for specific element and integration point.
   *
   * Enhanced version that allows state-dependent stiffness computation.
   *
   * @param ielem Element index for state tracking
   * @param ip Integration point index within the element
   * @returns Material stiffness matrix for the specified location
   *
   * @note Default implementation delegates to getMaterialStiff()
   */
  virtual inline Matrix getMaterialStiff(const idx_t &ielem, const idx_t &ip) const;

  /**
   * @brief Get the material mass matrix per unit length.
   *
   * Computes the material mass density matrix used in dynamic analysis.
   * The translational part of the mass matrix is typically diagonal with density values.
   *
   * @returns Mass matrix per unit length
   */
  virtual Matrix getMaterialMass() const = 0;

  /**
   * @brief Get the material mass matrix for specific element and integration point.
   *
   * Enhanced version for element-specific mass computation.
   *
   * @param ielem Element index for potential state tracking
   * @param ip Integration point index within the element
   * @returns Material mass matrix for the specified location
   */
  virtual inline Matrix getMaterialMass(const idx_t &ielem, const idx_t &ip) const;

  /**
   * @brief Get the lumped mass matrix for a given element length.
   *
   * Computes a lumped (diagonal) mass matrix for the given element length.
   *
   * @param l Element length
   * @returns Lumped mass matrix for the element
   */
  virtual Matrix getLumpedMass(const double l) const;

  /**
   * @brief Get the lumped mass matrix for specific element and length.
   *
   * Element-specific version of lumped mass computation.
   *
   * @param l Element length
   * @param ielem Element index for potential customization
   * @returns Lumped mass matrix for the specified element
   */
  virtual inline Matrix getLumpedMass(const double l, const idx_t &ielem) const;

  /**
   * @brief Apply computed deformation to the material state.
   *
   * This method commits the current deformation state to the material's internal
   * variables. It's typically called at the end of a successful load step to
   * update the material's permanent state.
   */
  virtual void applyDeform() = 0;

  /**
   * @brief Reject the current deformation and revert to previous state.
   *
   * This method reverts the material to its state before the current load step.
   * It's called when a load step fails to converge or needs to be repeated.
   */
  virtual void rejectDeform() = 0;

  /**
   * @brief Populate strain table with material-specific data.
   *
   * This method fills an XTable with strain-related data for output or
   * post-processing purposes. The specific data depends on the material type.
   *
   * @param name Name identifier for the table data
   * @param strain_table Output table to populate with strain data
   * @param items Index vector specifying which items to include
   * @param weights Weight vector for data averaging or integration
   */
  virtual void getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const = 0;

  /**
   * @brief Get the dissipated energy at a specific integration point.
   *
   * Computes the energy dissipated through inelastic processes at the
   * specified element and integration point location.
   *
   * @param ielem Element index
   * @param ip Integration point index within the element
   * @returns Total dissipated energy at the specified location
   */
  virtual double getDissipatedEnergy(const idx_t &ielem, const idx_t &ip) const = 0;

  /**
   * @brief Get the potential energy at a specific integration point.
   *
   * Computes the stored elastic energy at the specified element and
   * integration point location.
   *
   * @param ielem Element index
   * @param ip Integration point index within the element
   * @returns Elastic potential energy at the specified location
   * @note 0 for purely elastic materials
   */
  virtual double getPotentialEnergy(const idx_t &ielem, const idx_t &ip) const = 0;

  /**
   * @brief Get the hardening potential energy at a specific integration point.
   *
   * Computes the energy stored in hardening variables for plasticity models.
   * This represents the energy associated with the evolution of the yield surface.
   *
   * @param ielem Element index
   * @param ip Integration point index within the element
   * @returns Hardening potential energy at the specified location
   */
  virtual double getHardeningPotential(const idx_t &ielem, const idx_t &ip) const = 0;

  /**
   * @brief Get the context string for error reporting and debugging.
   *
   * @returns Context string identifying this material instance
   * @note 0 for purely elastic materials
   */
  virtual String getContext() const override;

protected:
  /**
   * @brief Destroy the Material object
   *
   * Virtual destructor ensures proper cleanup of derived classes.
   * Each material type can clean up its specific resources.
   */
  virtual ~Material();

  /// @brief Verbosity level for material output and debugging
  idx_t verbosity_;
};

/// @brief Default element-specific stress computation
void Material::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic)
{
  getStress(stress, strain);
}

/// @brief Default element-specific stiffness matrix
Matrix Material::getMaterialStiff(const idx_t &ielem, const idx_t &ip) const
{
  return getMaterialStiff();
}

/// @brief Default element-specific mass matrix
Matrix Material::getMaterialMass(const idx_t &ielem, const idx_t &ip) const
{
  return getMaterialMass();
}

/// @brief Default element-specific lumped mass
Matrix Material::getLumpedMass(const double l, const idx_t &ielem) const
{
  return getLumpedMass(l);
}
