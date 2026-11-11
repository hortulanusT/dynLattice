/**
 * @file NeoHookeanSolidMaterial.h
 * @author Til Gärtner
 * @brief Compressible Neo-Hookean solid
 */

#pragma once
#include "materials/HyperelasticSolidMaterial.h"

#include <jem/base/Tuple.h>

/// @brief Compressible isotropic Neo-Hookean solid.
///
/// Energy density: \f$\psi = \frac{\mu}{2}(I_1 - 3) - \mu \ln J + \frac{\lambda}{2}(\ln J)^2\f$
/// with \f$I_1 = \mathrm{tr}(F^T F)\f$ and \f$J = \det F\f$
///
/// First Piola-Kirchhoff stress: \f$P = \mu (F - F^{-T}) + \lambda \ln J \, F^{-T}\f$
///
/// Properties: young and either poisson_ratio or shear_modulus; density. Isotropic only:
/// `symmetry` and `C_ij` are rejected.
/// For small strains it reduces to isotropic linear elasticity.
///
/// @see HyperelasticSolidMaterial
class NeoHookeanSolidMaterial : public HyperelasticSolidMaterial
{
 public:
  static const char *TYPE_NAME;

  JEM_DECLARE_CLASS(NeoHookeanSolidMaterial, HyperelasticSolidMaterial);

  NeoHookeanSolidMaterial(const String &name,
                          const Properties &conf,
                          const Properties &props,
                          const Properties &globdat);

  static Ref<Material> makeNew(const String &name, const Properties &conf,
                               const Properties &props, const Properties &globdat);

  static void declare();

  virtual void configure(const Properties &props, const Properties &globdat) override;

  virtual void getConfig(const Properties &conf, const Properties &globdat) const override;

 protected:
  ~NeoHookeanSolidMaterial();

  virtual double calcStress_(const Matrix &P, const Matrix &F) const override;

  virtual void calcTangent_(const Matrix &A, const Matrix &F) const override;

 private:
  /// @brief Inverse of F, returns the determinant \f$J = \det F\f$.
  /// @param[out] Finv  Inverse of F
  /// @param[in] F  Deformation gradient (3x3)
  /// @throws IllegalInputException if J is not positive and finite
  double invertF_(jem::Tuple<double, GLOB_RANK, GLOB_RANK> &Finv, const Matrix &F) const;

  double mu_;     ///< Shear modulus
  double lambda_; ///< First Lamé parameter
};
