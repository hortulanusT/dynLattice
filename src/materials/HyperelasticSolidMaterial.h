/**
 * @file HyperelasticSolidMaterial.h
 * @author Til Gärtner
 * @brief Base class of the finite-strain (hyperelastic) solid materials
 */

#pragma once
#include "materials/SolidMaterial.h"

/// @brief Base class of the finite-strain solids, driven by the deformation gradient F.
///
/// A law provides calcStress_() and calcTangent_().
///
/// @see SolidMaterial, NeoHookeanSolidMaterial, SaintVenantKirchhoffSolidMaterial
class HyperelasticSolidMaterial : public SolidMaterial
{
 public:
  JEM_DECLARE_CLASS(HyperelasticSolidMaterial, SolidMaterial);

  HyperelasticSolidMaterial(const String &name,
                            const Properties &conf,
                            const Properties &props,
                            const Properties &globdat);

  using Material::getMaterialStiff;
  using Material::getStress;

  /// @throws IllegalOperationException always, only F input is supported
  virtual void getStress(const Vector &stress, const Vector &strain) override;

  virtual void getStress(const Matrix &stress, const Matrix &F, const idx_t &ielem, const idx_t &ip, const bool inelastic = true) override;

  /// @throws IllegalOperationException always, the tangent depends on F
  virtual Matrix getMaterialStiff() const override;

  virtual void getMaterialStiff(const Matrix &stiff, const Matrix &F, const idx_t &ielem, const idx_t &ip) const override;

 protected:
  ~HyperelasticSolidMaterial();

  /// @brief Compute the first Piola-Kirchhoff stress.
  /// @param[out] P  Stress (3x3), may be the same array as F
  /// @param[in] F  Deformation gradient (3x3)
  /// @returns Energy density at F
  virtual double calcStress_(const Matrix &P, const Matrix &F) const = 0;

  /// @brief Compute the tangent dP/dF.
  /// @param[out] A  Tangent (9x9), A(i + 3 J, k + 3 L) = dP_iJ / dF_kL
  /// @param[in] F  Deformation gradient (3x3)
  virtual void calcTangent_(const Matrix &A, const Matrix &F) const = 0;
};
