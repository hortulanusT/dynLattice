/**
 * @file SolidMaterial.h
 * @author Til Gärtner
 * @brief Common base class of the solid materials
 */

#pragma once
#include "materials/Material.h"

using jive::Ref;

/// @brief Common base class of the 3D solid materials.
///
/// Holds density, mass matrix and the energy bookkeeping, and provides the
/// stiffness input shared by all solids. Property: density.
///
/// @see ElasticSolidMaterial, HyperelasticSolidMaterial
class SolidMaterial : public Material
{
 public:
  static const char *DENSITY; ///< Density property key

  JEM_DECLARE_CLASS(SolidMaterial, Material);

  SolidMaterial(const String &name,
                const Properties &conf,
                const Properties &props,
                const Properties &globdat);

  virtual void configure(const Properties &props, const Properties &globdat) override;

  virtual void getConfig(const Properties &conf, const Properties &globdat) const override;

  virtual Matrix getMaterialMass() const override;

  virtual void getMaterialMass(const Matrix &mass, const idx_t &ielem, const idx_t &ip) const override;

  virtual void applyDeform() override;

  virtual void rejectDeform() override;

  virtual double getPotentialEnergy(const idx_t &ielem, const idx_t &ip) const override;

 protected:
  ~SolidMaterial();

  /// @brief Read the Lamé parameters of an isotropic material.
  ///
  /// Reads `young` and either `poisson_ratio` or `shear_modulus`.
  ///
  /// @param[in] myProps  Properties of the material
  /// @param[out] mu  Shear modulus
  /// @param[out] lambda  First Lamé parameter
  /// @throws IllegalInputException for non-physical values
  void getLame(const Properties &myProps, double &mu, double &lambda) const;

  /// @brief Build the 6x6 Voigt stiffness matrix from the user input.
  ///
  /// Voigt order: xx, yy, zz, yz, xz, xy with engineering shear strains.
  ///
  /// Property `symmetry` selects the input (default `isotropic`):
  /// - `isotropic`: young, poisson_ratio or shear_modulus
  /// - `transversely_isotropic`: young_1 (along the axis), young_2, poisson_12,
  ///   poisson_23, shear_12; the axis is `dir1`
  /// - `orthotropic`: young_1/2/3, poisson_12/13/23, shear_12/13/23;
  ///   the axes are `dir1`, `dir2` and their cross product
  ///
  /// `dir1` and `dir2` are given in the global frame (default: x and y axis)
  /// and are orthonormalised (Gram-Schmidt).
  ///
  /// If `C_11` is present, the stiffness is read directly in the global frame
  /// from `C_ij` (i <= j, symmetric) and `symmetry` is ignored.
  ///
  /// @param[in] myProps  Properties of the material
  /// @returns Stiffness matrix in the global frame
  /// @throws IllegalInputException for unknown symmetries or non-physical constants
  Matrix makeStiffness(const Properties &myProps) const;

  double density_; ///< Density

  Matrix materialM_; ///< Mass density matrix (density times identity, 3x3)

  Matrix energyPot_;  ///< Committed energy density, ipCount x elemCount
  Matrix currEnergy_; ///< Energy density of the current iteration
};
