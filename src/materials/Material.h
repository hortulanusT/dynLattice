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

/// @brief Abstract base class for material models.
///
/// Defines interface for material behavior including stress-strain relationships,
/// stiffness matrices, and energy calculations. Materials are created via
/// MaterialFactory and support multiple integration points per element.
///
/// @section Material Hierarchy
/// - ElasticRodMaterial: Linear elastic rod
/// - ElastoPlasticRodMaterial: Elasto-plastic with hardening
/// - SolidMaterial: Base of the solids
///   - ElasticSolidMaterial: Small-strain linear elastic solid
///   - HyperelasticSolidMaterial: Base of the finite-strain solids
///     (SaintVenantKirchhoffSolidMaterial, NeoHookeanSolidMaterial)
///
/// Rods and small-strain solids take a strain Vector, finite-strain solids take
/// the deformation gradient F as Matrix. Calling a material with the form it
/// does not support throws an exception.
///
/// @author Til Gärtner
/// @see ElasticRodMaterial, ElastoPlasticRodMaterial, ElasticSolidMaterial, HyperelasticSolidMaterial, MaterialFactory
class Material : public NamedObject
{
 public:
  JEM_DECLARE_CLASS(Material, NamedObject);

  /// Property name for verbosity level control
  const static char *VERBOSITY_PROP;

  /// @brief Constructs a new Material object.
  ///
  /// @param[in] name  Material instance name for identification
  /// @param[out] conf  Configuration properties container for output
  /// @param[in] props  Input properties for material parameters
  /// @param[in] globdat  Global data container for simulation state
  Material(const String &name,
           const Properties &conf,
           const Properties &props,
           const Properties &globdat);

  /// @brief Configure the material with given properties.
  ///
  /// @param[in] props  Properties containing configuration parameters
  /// @param[in] globdat  Global data container with simulation context
  /// @throws Derived classes may throw exceptions for invalid parameters
  virtual void configure(const Properties &props, const Properties &globdat);

  /// @brief Retrieve current material configuration.
  ///
  /// @param[out] conf  Properties object to store configuration in
  /// @param[in] globdat  Global data container with simulation context
  virtual void getConfig(const Properties &conf, const Properties &globdat) const;

  /// @brief Compute stress vector from strain vector.
  ///
  /// @param[out] stress  Computed stress vector at current state
  /// @param[in] strain  Input strain vector for stress computation
  ///
  /// @note Rods: 6 generalized components (shear, axial, bending, torsion).
  ///       Solids: 6 Voigt components (xx, yy, zz, yz, xz, xy).
  virtual void getStress(const Vector &stress, const Vector &strain) = 0;

  /// @brief Compute stress with element and integration point context.
  ///
  /// @param[out] stress  Computed stress vector
  /// @param[in] strain  Input strain vector
  /// @param[in] ielem  Element index for state tracking
  /// @param[in] ip  Integration point index
  /// @param[in] inelastic  Enable inelastic behavior (default: true)
  ///
  /// @note Default delegates to getStress(stress, strain)
  /// ElastoPlasticRodMaterial overrides for plastic state management
  virtual void getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic = true);

  /// @brief Compute the first Piola-Kirchhoff stress from the deformation gradient.
  ///
  /// @param[out] stress  First Piola-Kirchhoff stress P (3x3)
  /// @param[in] F  Deformation gradient (3x3)
  /// @param[in] ielem  Element index for state tracking
  /// @param[in] ip  Integration point index
  /// @param[in] inelastic  Enable inelastic behavior (default: true)
  ///
  /// @note Default implementation throws
  virtual void getStress(const Matrix &stress, const Matrix &F, const idx_t &ielem, const idx_t &ip, const bool inelastic = true);

  /// @brief Get the material stiffness matrix (tangent modulus relating stress to strain).
  ///
  /// @returns Material stiffness matrix
  virtual Matrix getMaterialStiff() const = 0;

  /// @brief Write the material stiffness matrix into an existing matrix (no allocation, for hot code)
  /// @param[out] stiff  Material stiffness matrix, overwritten
  /// @note Default implementation copies getMaterialStiff()
  virtual void getMaterialStiff(const Matrix &stiff) const;

  /// @brief Write the material stiffness matrix for an element and integration point into an existing matrix
  /// @param[out] stiff  Material stiffness matrix, overwritten
  /// @param[in] ielem  Element index
  /// @param[in] ip  Integration point index
  /// @note Default implementation copies getMaterialStiff()
  virtual void getMaterialStiff(const Matrix &stiff, const idx_t &ielem, const idx_t &ip) const;

  /// @brief Write the tangent dP/dF for a deformation gradient into an existing matrix
  ///
  /// The 9x9 tangent is stiff(i + 3*J, k + 3*L) = dP_iJ / dF_kL.
  ///
  /// @param[out] stiff  Tangent, overwritten
  /// @param[in] F  Deformation gradient (3x3)
  /// @param[in] ielem  Element index
  /// @param[in] ip  Integration point index
  /// @note Default implementation throws
  virtual void getMaterialStiff(const Matrix &stiff, const Matrix &F, const idx_t &ielem, const idx_t &ip) const;

  /// @brief Get the material mass matrix.
  ///
  /// @returns Mass matrix per unit length (rods) or per unit volume (solids)
  virtual Matrix getMaterialMass() const = 0;

  /// @brief Write the material mass matrix for an element and integration point into an existing matrix
  /// @param[out] mass  Material mass matrix, overwritten
  /// @param[in] ielem  Element index
  /// @param[in] ip  Integration point index
  /// @note Default implementation copies getMaterialMass()
  virtual void getMaterialMass(const Matrix &mass, const idx_t &ielem, const idx_t &ip) const;

  /// @brief Get the lumped mass matrix of a whole element.
  ///
  /// The default is getMaterialMass() * size; rods add rotary inertia terms.
  ///
  /// @param[in] size  Element length for rods, element volume for solids
  /// @returns Lumped mass matrix for the element
  virtual Matrix getLumpedMass(const double size) const;

  /// @brief Get the lumped mass matrix of a specific element.
  ///
  /// @param[in] size  Element length for rods, element volume for solids
  /// @param[in] ielem  Element index for potential customization
  /// @returns Lumped mass matrix for the specified element
  virtual Matrix getLumpedMass(const double size, const idx_t &ielem) const;

  /// @brief Apply computed deformation to the material state.
  ///
  /// This method commits the current deformation state to the material's internal
  /// variables. It's typically called at the end of a successful load step to
  /// update the material's permanent state.
  virtual void applyDeform() = 0;

  /// @brief Reject the current deformation and revert to previous state.
  ///
  /// This method reverts the material to its state before the current load step.
  /// It's called when a load step fails to converge or needs to be repeated.
  virtual void rejectDeform() = 0;

  /// @brief Populate strain table with material-specific data.
  ///
  /// This method fills an XTable with strain-related data for output or
  /// post-processing purposes. The specific data depends on the material type.
  ///
  /// @param[out] strain_table  Output table to populate with strain data
  /// @param[in,out] weights  Weight vector for data averaging or integration
  /// @param[in] name  Name identifier for the table data
  /// @param[in] items  Index vector specifying which items to include
  /// @note Default implementation only warns that the table is not supported
  virtual void getTable(XTable &strain_table, const Vector &weights, const String &name, const IdxVector &items) const;

  /// @brief Get the dissipated energy at a specific integration point.
  ///
  /// @param[in] ielem  Element index
  /// @param[in] ip  Integration point index within the element
  /// @returns Total dissipated energy at the specified location
  /// @note Default implementation returns 0 (no dissipation)
  /// @note Contract: must return the value of the last committed state, i.e. it has
  ///       to be stored in applyDeform(). SpecialCosseratRodModel::getEnergies_ reads
  ///       it right after applyDeform() without re-evaluating strains/stresses.
  virtual double getDissipatedEnergy(const idx_t &ielem, const idx_t &ip) const;

  /// @brief Get the stored elastic energy at a specific integration point.
  ///
  /// @param[in] ielem  Element index
  /// @param[in] ip  Integration point index within the element
  /// @returns Elastic potential energy at the specified location
  /// @note Contract: must return the value of the last committed state, i.e. it has
  ///       to be stored in applyDeform(). SpecialCosseratRodModel::getEnergies_ reads
  ///       it right after applyDeform() without re-evaluating strains/stresses.
  virtual double getPotentialEnergy(const idx_t &ielem, const idx_t &ip) const = 0;

  /// @brief Get the energy stored in hardening variables at a specific integration point.
  ///
  /// @param[in] ielem  Element index
  /// @param[in] ip  Integration point index within the element
  /// @returns Hardening potential energy at the specified location
  /// @note Default implementation returns 0 (no hardening)
  virtual double getHardeningPotential(const idx_t &ielem, const idx_t &ip) const;

  /// @brief Get the context string for error reporting and debugging.
  ///
  /// @returns Context string identifying this material instance
  virtual String getContext() const override;

 protected:
  virtual ~Material();

  /// @brief Verbosity level for material output and debugging
  idx_t verbosity_;
};
