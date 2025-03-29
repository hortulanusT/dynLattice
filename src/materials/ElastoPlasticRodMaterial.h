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
#include <jive/implict/Names.h>
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
  static const char *YIELD_DERIV_PROP;
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

  virtual void getHardVals(const Vector &hardVals, const Vector &hardParams) const;

  /**
   * Calculates the stress for a given strain using the convex cutting plane algorithm.
   * Compare Simo/Hughes Computational Inelasticity Box 3.6
   *
   * @param stress The calculated stress will be stored in this vector.
   * @param strain The strain vector for which the stress needs to be calculated.
   * @param ielem The index of the element.
   * @param ip The index of the integration point.
   */
  virtual void getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic = true) override;

  virtual void apply_deform() override;

  virtual void reject_deform() override;

  virtual void getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const override;

  virtual double getDissipatedEnergy(const idx_t &ielem, const idx_t &ip) const override;

  virtual double getHardeningPotential(const idx_t &ielem, const idx_t &ip) const override;

  virtual double getPotentialEnergy(const idx_t &ielem, const idx_t &ip) const override;

protected:
  ~ElastoPlasticRodMaterial();

protected:
  Ref<Function> yieldCond_;         // yield condition
  FuncUtils::FuncArray yieldDeriv_; // derivative of the yield condition
  idx_t maxIter_;                   // maximum iterations in the stress update
  double precision_;                // tolerance for the stress update

  idx_t argCount_;         // number of arguments to yield condition
  jem::Slice stress_part_; // slice for the stress part of the arguments
  jem::Slice hard_part_;   // slice for the hardening part of the arguments

  Matrix materialH_; // hardening factors

  Cubix old_hardParams_;  // last converged load step
  Cubix curr_hardParams_; // last inner converged solution

  Cubix old_plastStrains_;  // last converged load step
  Cubix curr_plastStrains_; // last inner converged solution

  Matrix curr_deltaFlow_; // last inner converged solution
  Matrix E_diss_;         // dissipated energy
  Matrix E_hardPot_;      // hardening potential
};
