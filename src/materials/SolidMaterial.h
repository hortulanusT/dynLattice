/**
 * @file SolidMaterial.h
 * @author Til Gärtner
 * @brief Common base class of the solid materials
 */

#pragma once
#include "materials/Material.h"

#include <jem/base/Tuple.h>

using jive::Ref;

/// @brief Common base class of the 3D solid materials.
///
/// Holds density, mass matrix, energy bookkeeping and the Lamé parameters (getLame_()).
/// Laws defined by a stiffness matrix C (ElasticSolidMaterial, SaintVenantKirchhoffSolidMaterial)
/// also use readStiffness_() and writeStiffness_().
///
/// Properties: density (default 0); ipCount and elemCount (required, set by the model).
///
/// Use in a model:
/// - `ipCount` is the largest number of integration points of any element. The energy of
///   the points an element does not have stays 0.
/// - ElasticSolidMaterial takes the Voigt strain vector and returns the stress. The hyperelastic
///   materials take the deformation gradient F and return P and dP/dF. The other form throws.
/// - getStress() with `ielem` and `ip` stores the energy density, applyDeform() commits it and
///   rejectDeform() restores the committed one.
/// - The lumped mass of an element is getLumpedMass(volume): density times volume on the diagonal.
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

  /// @name Sizes
  /// Plain `int`, not `idx_t`: they are sizes of arrays and tuples, which `idx_t` (a class in jem) cannot be.
  /// @{
  static const int STRAIN_COUNT = 6; ///< Number of stress and strain components (Voigt)
  static const int GLOB_RANK = 3;    ///< Global rank, i.e. the number of spatial dimensions
  /// @}

  /// @brief Voigt index of the tensor component (i,j), order xx, yy, zz, yz, xz, xy.
  static const idx_t VOIGT[GLOB_RANK][GLOB_RANK];

  /// @brief Lamé parameters from `young` and either `poisson_ratio` or `shear_modulus` (not both).
  /// @param[in] myProps  Properties of the material
  /// @param[out] mu  Shear modulus
  /// @param[out] lambda  First Lamé parameter
  /// @throws IllegalInputException for non-physical or over-determined input
  void getLame_(const Properties &myProps, double &mu, double &lambda) const;

  /// @brief Build the 6x6 stiffness from the user input.
  ///
  /// Voigt order xx, yy, zz, yz, xz, xy with engineering shear strains.
  /// `symmetry` (default `isotropic`) selects the constants:
  /// - `isotropic`: young and either poisson_ratio or shear_modulus
  /// - `transversely_isotropic`: young_1 (along dir1), young_2, poisson_12, poisson_23, shear_12
  /// - `orthotropic`: young_1/2/3, poisson_12/13/23, shear_12/13/23
  ///
  /// The material axes follow from `dir1` and `dir2` in the global frame (default x and y axis;
  /// dir2 is made perpendicular to dir1).
  ///
  /// Alternatively the entries `C_ij` (i <= j) give the stiffness in the global frame. The diagonal
  /// is required, the other entries default to 0, and C has to be positive definite. They cannot be
  /// combined with `symmetry`, `young`, `poisson_ratio`, `shear_modulus` or `young_1`.
  ///
  /// @param[in] myProps  Properties of the material
  /// @returns Stiffness matrix in the global frame
  /// @throws IllegalInputException for unknown symmetries or invalid input
  Matrix makeStiffness_(const Properties &myProps) const;

  /// @brief Read the stiffness with makeStiffness_(). At verbosity > 0 it is written to the debug output.
  /// @param[in] props  Properties of all materials
  /// @param[in] globdat  Global data container
  /// @returns Stiffness matrix in the global frame (6x6)
  Matrix readStiffness_(const Properties &props, const Properties &globdat) const;

  /// @brief Write the stiffness as the entries `C_ij` (i <= j) into the configuration.
  ///
  /// makeStiffness_() reads them back.
  ///
  /// @param[out] conf  Configuration of all materials
  /// @param[in] stiff  Stiffness matrix in the global frame (6x6)
  void writeStiffness_(const Properties &conf, const Matrix &stiff) const;

  double density_; ///< Density

  Matrix materialM_; ///< Mass density matrix (density times identity, 3x3)

  Matrix energyPot_;  ///< Committed energy density, ipCount x elemCount
  Matrix currEnergy_; ///< Energy density of the current iteration

 private:
  /// @brief Read the stiffness from the entries `C_ij`, if `C_11` is given.
  /// @param[out] C  Stiffness matrix in the global frame (6x6)
  /// @param[in] myProps  Properties of the material
  /// @returns false if `C_11` is not given and nothing was read
  /// @throws IllegalInputException for a missing diagonal entry, a matrix that is not positive
  ///         definite or additional elastic constants
  bool readDirectStiffness_(const Matrix &C, const Properties &myProps) const;

  /// @brief Read the material axes from `dir1` and `dir2` (Gram-Schmidt).
  ///
  /// \f$a_1 = d_1/|d_1|\f$, \f$a_2 = (d_2 - (d_2\cdot a_1)a_1)/|\cdot|\f$, \f$a_3 = a_1 \times a_2\f$.
  /// If `dir2` is not given it is the y axis, or the x axis if that is parallel to `dir1`.
  ///
  /// @param[out] R  Material axes as columns in the global frame (3x3)
  /// @param[in] myProps  Properties of the material
  /// @throws IllegalInputException for directions without 3 components, a zero or parallel directions
  void readOrientation_(const Matrix &R, const Properties &myProps) const;

  /// @brief Stiffness of an orthotropic material in its own axes.
  ///
  /// The normal part is the inverse of the compliance (\f$S_{11} = 1/E_1\f$,
  /// \f$S_{12} = -\nu_{12}/E_1\f$, ...), the shear part is diagonal with
  /// \f$G_{23}, G_{13}, G_{12}\f$.
  ///
  /// @throws IllegalInputException for non-positive moduli or a compliance that is not positive definite
  Matrix orthotropicStiffness_(double E1, double E2, double E3,
                               double nu12, double nu13, double nu23,
                               double G12, double G13, double G23) const;

  /// @brief Rotate a stiffness into the global frame, \f$C \leftarrow T C T^T\f$.
  ///
  /// \f$T\f$ is the Voigt transformation of the stress for the material axes `R`.
  ///
  /// @param[in,out] C  Stiffness matrix (6x6)
  /// @param[in] R  Material axes as columns (3x3)
  static void rotateStiffness_(const Matrix &C, const Matrix &R);

  static const idx_t VOIGT_I[STRAIN_COUNT]; ///< Tensor row index of each Voigt component
  static const idx_t VOIGT_J[STRAIN_COUNT]; ///< Tensor column index of each Voigt component
};
