/**
 * @file ElastoPlasticRodMaterial.h
 * @author Til Gärtner
 * @brief Elasto-plastic rod material with yield conditions and hardening
 */

#pragma once
#include "materials/ElasticRodMaterial.h"
#include <jem/numeric/algebra/utilities.h>
#include <jive/util/FuncUtils.h>

// Forward declarations
class MaterialFactory;

using jem::idx_t;
using jem::numeric::Function;
using jive::Ref;
using jive::String;
using jive::util::FuncUtils;

/// @brief Elasto-plastic rod material implementing yield conditions with isotropic and kinematic hardening
/// @see [Corresponding Paper](https://doi.org/10.1007/s00466-024-02572-3)
class ElastoPlasticRodMaterial : public ElasticRodMaterial
{
 public:
  /// @name Plasticity property identifiers
  /// @{
  static const char *TYPE_NAME;
  static const char *YIELD_PROP;       ///< Yield condition function
  static const char *YIELD_DERIV_PROP; ///< Yield condition derivatives
  static const char *YIELD_TYPE_PROP;  ///< Yield condition type (see YieldType)
  static const char *YIELD_STRENGTH_PROP; ///< Strengths c_i of the super-ellipse yield condition
  static const char *YIELD_EXPONENT_PROP; ///< Exponents p_i of the super-ellipse yield condition
  static const char *ISO_HARD_PROP;    ///< Isotropic hardening parameters
  static const char *KIN_HARD_PROP;    ///< Kinematic hardening parameters
  /// @}

  /// @brief Yield condition implementations, selected by the property `yieldType`
  enum YieldType
  {
    EXPRESSION,   ///< "Expression": interpreted function `yieldCond` (default)
    SUPER_ELLIPSE ///< "SuperEllipse": native Lamé curve, see getSuperEllipse_
  };

  JEM_DECLARE_CLASS(ElastoPlasticRodMaterial, ElasticRodMaterial);

  /// @brief Constructor with plasticity configuration
  /// @param[in] name  Material instance name
  /// @param[out] conf  Actually used configuration properties
  /// @param[in] props  User-specified material properties
  /// @param[in] globdat  Global data container
  ElastoPlasticRodMaterial(const String &name,
                           const Properties &conf,
                           const Properties &props,
                           const Properties &globdat);

  /// @brief Factory method for plastic material creation
  /// @param[in] name  Material instance name
  /// @param[out] conf  Actually used configuration properties
  /// @param[in] props  User-specified material properties
  /// @param[in] globdat  Global data container
  /// @return Reference to the created material
  static Ref<Material> makeNew(const String &name, const Properties &conf,
                               const Properties &props, const Properties &globdat);

  /// @brief Register plastic material type with factory
  static void declare();

  /// @brief Configure plasticity parameters and yield conditions
  ///
  /// @param[in] props  Properties containing configuration parameters
  /// @param[in] globdat  Global data container with simulation context
  /// @throws jem::util::PropertyException if yield function is not provided
  virtual void configure(const Properties &props, const Properties &globdat) override;

  virtual void getConfig(const Properties &conf, const Properties &globdat) const override;

  /// @brief Get hardening values from hardening parameters
  /// @param[out] hardVals  Computed hardening values
  /// @param[in] hardParams  Hardening parameters
  virtual void getHardVals(const Vector &hardVals, const Vector &hardParams) const;

  /// @brief Elastic stress computation based on elastic rod material model
  /// @throws jem::Exception as this method should not be called directly for elasto-plastic materials
  virtual void getStress(const Vector &stress, const Vector &strain) override;

  /// @brief Plastic stress computation using convex cutting plane algorithm
  /// @details Implements iterative return mapping with yield condition evaluation
  /// @param[out] stress  Calculated stress vector
  /// @param[in] strain  Input strain vector
  /// @param[in] ielem  Element index
  /// @param[in] ip  Integration point index
  /// @param[in] inelastic  Whether to compute the inelastic stress update
  /// @see [Computational Inelasticity](https://doi.org/10.1007/b98904) Box 3.6
  virtual void getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic = true) override;

  virtual void applyDeform() override;

  virtual void rejectDeform() override;

  virtual void getTable(XTable &strain_table, const Vector &weights, const String &name, const IdxVector &items) const override;

  virtual double getDissipatedEnergy(const idx_t &ielem, const idx_t &ip) const override;

  virtual double getHardeningPotential(const idx_t &ielem, const idx_t &ip) const override;

  virtual double getPotentialEnergy(const idx_t &ielem, const idx_t &ip) const override;

 protected:
  /// @brief Destroy the elasto-plastic rod material
  ~ElastoPlasticRodMaterial();

  /// @brief Value of the yield condition
  /// @param[in] args  Stresses, [isotropic hardening stress], [kinematic hardening stresses]
  double getYield_(const Vector &args) const;

  /// @brief Gradient of the yield condition with respect to args
  /// @param[out] grad  Gradient
  /// @param[in] args  Stresses, [isotropic hardening stress], [kinematic hardening stresses]
  void getYieldGrad_(const Vector &grad, const Vector &args) const;

  /// @brief Super-ellipse yield condition f = sum_i |s_i/(c_i-h_i)|^p_i - (1+h_0)
  /// @details Kinematic hardening h_i shifts the strengths c_i (Herrnböck et al., Comput. Mech. 2023,
  /// Eq. 39), isotropic hardening h_0 acts on the sum (Herrnböck et al., Comput. Mech. 2021, Eq. 47).
  double getSuperEllipse_(const Vector &args) const;

  /// @brief Gradient of the super-ellipse yield condition (zero stress derivative for |s_i| < EPS)
  void getSuperEllipseGrad_(const Vector &grad, const Vector &args) const;

 protected:
  /// @name Plasticity algorithm components
  /// @{
  Ref<Function> yieldCond_;         ///< Yield condition function
  FuncUtils::FuncArray yieldDeriv_; ///< Derivatives of yield condition
  idx_t maxIter_;                   ///< Max iterations for stress update
  double precision_;                ///< Convergence tolerance for stress update
  YieldType yieldType_;             ///< Selected yield condition implementation
  bool isoHard_;                    ///< Isotropic hardening stress is a yield function argument
  bool kinHard_;                    ///< Kinematic hardening stresses are yield function arguments
  Vector yieldStrength_;            ///< Strengths c_i of the super-ellipse yield condition
  Vector yieldExponent_;            ///< Exponents p_i of the super-ellipse yield condition
  /// @}

  /// @name Function argument organization
  /// @{
  idx_t dofCount_; ///< Degrees of freedom in stress/strain vectors
  idx_t argCount_; ///< Total arguments to yield condition
  /// @}

  /// @name Hardening and plastic state
  /// @{
  Matrix materialH_; ///< Hardening matrix

  Cubix oldHardParams_;  ///< Hardening parameters from last converged step
  Cubix currHardParams_; ///< Current hardening parameters

  Cubix oldPlastStrains_;  ///< Plastic strains from last converged step
  Cubix currPlastStrains_; ///< Current plastic strains

  /// @name Work arrays of getStress
  /// Created once in configure and overwritten in every call, so that getStress
  /// does not have to create new arrays for every integration point
  /// (see the note in utils/helpers.h). Not thread safe.
  /// @{
  Vector plastStrain_;   ///< Plastic strain iterate
  Vector hardParams_;    ///< Hardening parameter iterate
  Vector hardStress_;    ///< Hardening stresses
  Vector elasticStrain_; ///< Elastic strain
  Vector yieldArgs_;     ///< Arguments of the yield condition
  Vector yieldGrad_;     ///< Gradient of the yield condition
  /// @}

  Matrix currDeltaFlow_; ///< Current plastic flow increment
  Matrix energyDiss_;    ///< Dissipated energy storage
  Matrix energyHardPot_; ///< Hardening potential energy storage
  /// @}
};
