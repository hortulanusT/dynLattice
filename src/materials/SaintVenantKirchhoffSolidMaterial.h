/**
 * @file SaintVenantKirchhoffSolidMaterial.h
 * @author Til Gärtner
 * @brief St. Venant-Kirchhoff solid (linear elastic law on the Green-Lagrange strain)
 */

#pragma once
#include "materials/HyperelasticSolidMaterial.h"

/// @brief St. Venant-Kirchhoff solid: \f$S = C : E\f$ with \f$E = (F^T F - I) / 2\f$.
///
/// Same stiffness input as ElasticSolidMaterial, see SolidMaterial::makeStiffness_().
/// Loses stiffness under strong compression.
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
  /// @brief Second Piola-Kirchhoff stress \f$S = C : E\f$, returns the energy density \f$\frac12 E : C : E\f$.
  double calcSecondPK_(const Matrix &S, const Matrix &F) const;

  Matrix materialK_; ///< Voigt stiffness matrix (6x6)

  /// Work array of calcStress_ and calcTangent_, overwritten in every call (see utils/helpers.h).
  /// Not thread safe.
  mutable Matrix secondPKWork_; ///< Second Piola-Kirchhoff stress, 3 x 3
};
