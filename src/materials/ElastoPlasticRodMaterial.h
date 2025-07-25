/**
 * @file ElastoPlasticRodMaterial.h
 * @author Til Gärtner
 * @brief Elasto-plastic rod material with yield conditions and hardening
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

/// @brief Elasto-plastic rod material implementing yield conditions with isotropic and kinematic hardening
class ElastoPlasticRodMaterial : public ElasticRodMaterial
{
public:
  /// @name Plasticity property identifiers
  /// @{
  static const char *TYPE_NAME;
  static const char *YIELD_PROP;       ///< Yield condition function
  static const char *YIELD_DERIV_PROP; ///< Yield condition derivatives
  static const char *ISO_HARD_PROP;    ///< Isotropic hardening parameters
  static const char *KIN_HARD_PROP;    ///< Kinematic hardening parameters
  /// @}

  JEM_DECLARE_CLASS(ElastoPlasticRodMaterial, ElasticRodMaterial);

  /// @brief Constructor with plasticity configuration
  ElastoPlasticRodMaterial(const String &name,
                           const Properties &conf,
                           const Properties &props,
                           const Properties &globdat);

  /// @brief Factory method for plastic material creation
  static Ref<Material> makeNew(const String &name, const Properties &conf,
                               const Properties &props, const Properties &globdat);

  /// @brief Register plastic material type with factory
  static void declare();

  virtual void configure(const Properties &props, const Properties &globdat) override;

  virtual void getConfig(const Properties &conf, const Properties &globdat) const override;

  /// @brief Get hardening values from hardening parameters
  virtual void getHardVals(const Vector &hardVals, const Vector &hardParams) const;

  /// @brief Plastic stress computation using convex cutting plane algorithm
  /// @param stress Calculated stress vector (output)
  /// @param strain Input strain vector
  /// @param ielem Element index
  /// @param ip Integration point index
  /// @see [Computational Inelasticity](https://doi.org/10.1007/b98904) Box 3.6
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
  /// @name Plasticity algorithm components
  /// @{
  Ref<Function> yieldCond_;         ///< Yield condition function
  FuncUtils::FuncArray yieldDeriv_; ///< Derivatives of yield condition
  idx_t maxIter_;                   ///< Max iterations for stress update
  double precision_;                ///< Convergence tolerance for stress update
  /// @}

  /// @name Function argument organization
  /// @{
  idx_t argCount_;         ///< Total arguments to yield condition
  jem::Slice stress_part_; ///< Stress portion of arguments
  jem::Slice hard_part_;   ///< Hardening portion of arguments
  /// @}

  /// @name Hardening and plastic state
  /// @{
  Matrix materialH_; ///< Hardening matrix

  Cubix old_hardParams_;  ///< Hardening parameters from last converged step
  Cubix curr_hardParams_; ///< Current hardening parameters

  Cubix old_plastStrains_;  ///< Plastic strains from last converged step
  Cubix curr_plastStrains_; ///< Current plastic strains

  Matrix curr_deltaFlow_; ///< Current plastic flow increment
  Matrix E_diss_;         ///< Dissipated energy storage
  Matrix E_hardPot_;      ///< Hardening potential energy storage
  /// @}
};
