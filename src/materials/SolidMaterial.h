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

  /// @brief Voigt index of the tensor component (i,j).
  ///
  /// Voigt order: xx, yy, zz, yz, xz, xy
  static const idx_t VOIGT[3][3];

  /// @brief Read the Lamé parameters of an isotropic material.
  ///
  /// Reads `young` and either `poisson_ratio` or `shear_modulus`; giving both is
  /// an error, as the material would be over-determined.
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
  /// from `C_ij` (i <= j, symmetric). The diagonal entries `C_ii` are required,
  /// the off-diagonal entries default to zero, and the matrix has to be positive
  /// definite. Combining `C_ij` with `symmetry` or the elastic constants is an error.
  ///
  /// @param[in] myProps  Properties of the material
  /// @returns Stiffness matrix in the global frame
  /// @throws IllegalInputException for unknown symmetries or non-physical constants
  Matrix makeStiffness(const Properties &myProps) const;

  double density_; ///< Density

  Matrix materialM_; ///< Mass density matrix (density times identity, 3x3)

  Matrix energyPot_;  ///< Committed energy density, ipCount x elemCount
  Matrix currEnergy_; ///< Energy density of the current iteration

 private:
  /// @brief Read the stiffness directly from the entries `C_ij`, if given.
  ///
  /// @param[out] C  Stiffness matrix in the global frame (6x6)
  /// @param[in] myProps  Properties of the material
  /// @returns false if `C_11` is not present and nothing was read
  /// @throws IllegalInputException for a missing diagonal entry, a stiffness that is not
  ///         positive definite or additional elastic constants
  bool readDirectStiffness_(const Matrix &C, const Properties &myProps) const;

  /// @brief Read the material axes `dir1` and `dir2` (default: x and y axis).
  ///
  /// The axes are orthonormalised by Gram-Schmidt: \f$a_1 = d_1/|d_1|\f$,
  /// \f$a_2 = (d_2 - (d_2\cdot a_1)a_1)/|\cdot|\f$, \f$a_3 = a_1 \times a_2\f$.
  ///
  /// @param[out] R  Material axes as columns, given in the global frame (3x3)
  /// @param[in] myProps  Properties of the material
  /// @throws IllegalInputException for a zero direction or parallel directions
  void readOrientation_(const Matrix &R, const Properties &myProps) const;

  /// @brief Stiffness of an orthotropic material in its own axes.
  ///
  /// The normal part is the inverse of the compliance
  /// \f$S_{11} = 1/E_1\f$, \f$S_{12} = -\nu_{12}/E_1\f$, \f$S_{22} = 1/E_2\f$, ...,
  /// the shear part is diagonal with \f$G_{23}, G_{13}, G_{12}\f$.
  ///
  /// @throws IllegalInputException for non-positive moduli or a compliance that is not
  ///         positive definite
  Matrix orthotropicStiffness_(double E1, double E2, double E3,
                               double nu12, double nu13, double nu23,
                               double G12, double G13, double G23) const;

  /// @brief Rotate a stiffness into the global frame, \f$C \leftarrow T C T^T\f$.
  ///
  /// \f$T\f$ is the Voigt transformation of the stress for the axes `R`.
  ///
  /// @param[in,out] C  Stiffness matrix (6x6)
  /// @param[in] R  Material axes as columns (3x3)
  static void rotateStiffness_(const Matrix &C, const Matrix &R);

  /// @brief Test whether a symmetric 6x6 matrix is positive definite (Cholesky).
  static bool isPositiveDefinite_(const Matrix &C);

  static const idx_t VOIGT_I[6]; ///< Row index of the Voigt component
  static const idx_t VOIGT_J[6]; ///< Column index of the Voigt component
};
