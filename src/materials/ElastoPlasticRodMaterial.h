/**
 * @file ElasticRodMaterial.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief simple implementation of a elaso-plastic rod
 * @version 0.1
 * @date 2023-05-31
 *
 * @copyright Copyright (C) 2023 TU Delft. All rights reserved.
 *
 */

#pragma once
#include "materials/ElasticRodMaterial.h"
#include "materials/MaterialFactory.h"
#include "utils/helpers.h"
#include <jem/numeric/algebra/utilities.h>
#include <jem/util/PropertyException.h>
#include <jem/util/StringUtils.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/utilities.h>

using jem::idx_t;
using jem::newInstance;
using jem::numeric::dotProduct;
using jem::numeric::Function;
using jive::Cubix;
using jive::Ref;
using jive::Vector;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::util::DofSpace;
using jive::util::FuncUtils;
using jive::util::joinNames;
using jive::util::XTable;

class ElastoPlasticRodMaterial : public ElasticRodMaterial
{
public:
  static const char *TYPE_NAME;
  static const char *YIELD_PROP;
  static const char *ISO_HARD_PROP;
  static const char *KIN_HARD_PROP;

  JEM_DECLARE_CLASS(ElastoPlasticRodMaterial, ElasticRodMaterial);

  ElastoPlasticRodMaterial(const String &name,
                           const Properties &conf,
                           const Properties &props,
                           const Properties &globdat);

  static Ref<Material> makeNew(const String &name, const Properties &conf,
                               const Properties &props, const Properties &globdat);

  static void declare();

  virtual void configure(const Properties &props, const Properties &globdat) override;

  virtual void getConfig(const Properties &conf, const Properties &globdat) const override;

  virtual inline void getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip) const override;

  virtual double calc_inelast_corr(const Vector &strain, const idx_t &ielem, const idx_t &ip) override;

  virtual void apply_inelast_corr() override;

  virtual void reject_inelast_corr() override;

  virtual void getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const override;

  virtual double getDisspiatedEnergy() const override;

protected:
  ~ElastoPlasticRodMaterial();

protected:
  Ref<Function> yieldCond_; // yield condition
  idx_t argCount_;          // number of arguments to yield condition
  double E_diss_;           // dissipated energy

  Matrix isoParams_;

  Matrix kinFacts_;
  Cubix kinParams_;

  Cubix plastStrains_;
  Cubix oldStrains_;

  Cubix currStrains_;
  Cubix intUpdate_;
};

void ElastoPlasticRodMaterial::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip) const
{
  stress = matmul(materialK_, Vector(strain - plastStrains_[ielem][ip]));
}
