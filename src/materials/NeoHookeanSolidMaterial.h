/**
 * @file NeoHookeanSolidMaterial.h
 * @author Til Gärtner
 * @brief Compressible Neo-Hookean solid
 */

#pragma once
#include "materials/HyperelasticSolidMaterial.h"

/// @brief Compressible isotropic Neo-Hookean solid.
///
/// Energy density: psi = mu/2 (I1 - 3) - mu ln J + lambda/2 (ln J)^2
///
/// First Piola-Kirchhoff stress: P = mu (F - F^-T) + lambda ln J F^-T
///
/// Properties: young, poisson_ratio or shear_modulus, density.
/// For small strains the law reduces to isotropic linear elasticity.
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
  /// @throws IllegalInputException if det(F) is not positive
  double checkedDet_(const Matrix &F) const;

  /// Inverse of F for a given determinant
  static void invert_(double (&Finv)[3][3], const Matrix &F, double det);

  double mu_;     ///< Shear modulus
  double lambda_; ///< First Lamé parameter
};
