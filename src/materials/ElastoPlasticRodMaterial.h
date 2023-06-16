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
#include <jem/util/StringUtils.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
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
using jive::util::FuncUtils;
using jive::util::joinNames;

class ElastoPlasticRodMaterial : public ElasticRodMaterial
{
public:
  static const char *TYPE_NAME;
  static const char *YIELD_PROP;
  static const char *DOF_PROP;

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

  Matrix getConsistentStiff(const Vector &stress) const;

protected:
  ~ElastoPlasticRodMaterial();

protected:
  StringVector dofNames_;
  Ref<Function> yieldCond_;
};