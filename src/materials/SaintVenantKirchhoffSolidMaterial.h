/**
 * @file SaintVenantKirchhoffSolidMaterial.h
 * @author Til Gärtner
 * @brief St. Venant-Kirchhoff solid (linear elastic law on the Green-Lagrange strain)
 */

#pragma once
#include "materials/HyperelasticSolidMaterial.h"

/// @brief St. Venant-Kirchhoff solid: S = C : E with E = (F^T F - I) / 2.
///
/// Accepts the same stiffness input as ElasticSolidMaterial, see SolidMaterial::makeStiffness().
/// Only suitable for moderate stretches, as it loses stiffness under strong compression.
///
/// @see HyperelasticSolidMaterial, ElasticSolidMaterial
class SaintVenantKirchhoffSolidMaterial : public HyperelasticSolidMaterial
{
 public:
  static const char *TYPE_NAME;

  JEM_DECLARE_CLASS(SaintVenantKirchhoffSolidMaterial, HyperelasticSolidMaterial);

  SaintVenantKirchhoffSolidMaterial(const String &name,
                                    const Properties &conf,
                                    const Properties &props,
                                    const Properties &globdat);

  static Ref<Material> makeNew(const String &name, const Properties &conf,
                               const Properties &props, const Properties &globdat);

  static void declare();

  virtual void configure(const Properties &props, const Properties &globdat) override;

  virtual void getConfig(const Properties &conf, const Properties &globdat) const override;

 protected:
  ~SaintVenantKirchhoffSolidMaterial();

  virtual double calcStress_(const Matrix &P, const Matrix &F) const override;

  virtual void calcTangent_(const Matrix &A, const Matrix &F) const override;

 private:
  /// Second Piola-Kirchhoff stress and energy density
  double calcSecondPK_(double (&S)[3][3], const Matrix &F) const;

  Matrix materialK_; ///< Voigt stiffness matrix (6x6)
};
