/**
 * @file ElasticSolidMaterial.h
 * @author Til Gärtner
 * @brief Small-strain linear elastic solid (isotropic, transversely isotropic, orthotropic)
 */

#pragma once
#include "materials/SolidMaterial.h"

/// @brief Small-strain linear elastic 3D solid.
///
/// Strain and stress are Voigt vectors (xx, yy, zz, yz, xz, xy) with engineering
/// shear strains. The stiffness input is described in SolidMaterial::makeStiffness().
///
/// @see SolidMaterial, SaintVenantKirchhoffSolidMaterial
class ElasticSolidMaterial : public SolidMaterial
{
 public:
  static const char *TYPE_NAME;

  JEM_DECLARE_CLASS(ElasticSolidMaterial, SolidMaterial);

  ElasticSolidMaterial(const String &name,
                       const Properties &conf,
                       const Properties &props,
                       const Properties &globdat);

  static Ref<Material> makeNew(const String &name, const Properties &conf,
                               const Properties &props, const Properties &globdat);

  static void declare();

  virtual void configure(const Properties &props, const Properties &globdat) override;

  virtual void getConfig(const Properties &conf, const Properties &globdat) const override;

  using Material::getMaterialStiff;
  using Material::getStress;

  virtual void getStress(const Vector &stress, const Vector &strain) override;

  virtual void getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic = true) override;

  virtual Matrix getMaterialStiff() const override;

  virtual void getMaterialStiff(const Matrix &stiff) const override;

  virtual void getMaterialStiff(const Matrix &stiff, const idx_t &ielem, const idx_t &ip) const override;

 protected:
  ~ElasticSolidMaterial();

  Matrix materialK_; ///< Voigt stiffness matrix (6x6)
};
