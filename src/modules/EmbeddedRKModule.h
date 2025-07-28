/**
 * @file EmbeddedRKModule.h
 * @author Til Gärtner
 * @brief Embedded Runge-Kutta methods for explicit time integration
 *
 * This module implements embedded Runge-Kutta algorithms with adaptive
 * step size control for explicit time integration of dynamic systems.
 *
 * @note this is an old and largely untested module, use with caution
 */

#pragma once

#include "modules/ExplicitModule.h"
#include <jem/base/Class.h>

/// @brief Module for embedded Runge-Kutta time integration methods
/// @details Implements embedded Runge-Kutta schemes including Bogacki-Shampine (ODE23)
/// and Dormand-Prince (ODE45) methods with adaptive step size control. Features
/// special handling for rotational DOFs using Runge-Kutta-Munthe-Kaas (RKMK) methods.
/// @see [ODE23](https://en.wikipedia.org/wiki/Bogacki%E2%80%93Shampine_method)
/// @see [ODE45](https://en.wikipedia.org/wiki/Dormand%E2%80%93Prince_method)
/// @see [RKMK 1998] (https://doi.org/10.1007/BF02510919)
/// @see [RKMK 1999] (https://doi.org/10.1016/S0168-9274(98)00030-0)
class EmbeddedRKModule : public ExplicitModule
{
public:
  JEM_DECLARE_CLASS(EmbeddedRKModule, ExplicitModule);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  /// @}

  /// @brief Constructor
  /// @param name Module name
  explicit EmbeddedRKModule(const String &name = "EmbeddedRK");

  /// @brief Initialize the module
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  virtual Status init(const Properties &conf,
                      const Properties &props,
                      const Properties &globdat) override;

  /// @brief Solve using embedded Runge-Kutta method
  /// @param info Solver information
  /// @param globdat Global data container
  virtual void solve(const Properties &info,
                     const Properties &globdat) override;

  /// @brief Factory method for creating new EmbeddedRKModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new EmbeddedRKModule instance
  static Ref<Module> makeNew(const String &name,
                             const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  /// @brief Register EmbeddedRKModule type with ModuleFactory
  static void declare();

  /// @brief Protected destructor
  virtual ~EmbeddedRKModule();

private:
  /// @brief Initialize the Butcher tableau for Bogacki-Shampine method (ODE23)
  /// @details Sets up the coefficients for the 3rd-order embedded method
  /// equivalent to MATLAB's ode23 solver
  void initODE23_();

  /// @brief Initialize the Butcher tableau for Dormand-Prince method (ODE45)
  /// @details Sets up the coefficients for the 5th-order embedded method
  /// equivalent to MATLAB's ode45 solver
  void initODE45_();

  /// @brief Correct rotation values for RKMK method updates
  /// @param uncorrected Uncorrected displacement vector
  /// @param delta Displacement increment
  /// @details Implements Runge-Kutta-Munthe-Kaas method for rotation manifolds
  /// @see [Munthe-Kaas (1998)](https://doi.org/10.1007/BF02510919)
  void correctDisp_(const Vector &uncorrected, const Vector &delta);

  /// @brief Calculate Bernoulli coefficient for exponential map corrections
  /// @param i Coefficient index
  /// @return Bernoulli coefficient value
  double bernoulliCoeff_(const idx_t i);

  /// @brief Compute adjoint operator for Lie group integration
  /// @param point Current point on manifold
  /// @param about Reference point
  /// @param iterate Iteration number
  /// @return Adjoint matrix
  Matrix adjoint_(const Matrix &point,
                  const Matrix &about,
                  const idx_t iterate);

  /// @brief Compute inverse derivative of exponential map
  /// @param res Result matrix (output)
  /// @param point Current point
  /// @param about Reference point
  void invDerivExpMap_(const Matrix &res,
                       const Matrix &point,
                       const Matrix &about);

private:
  /// @name Runge-Kutta parameters
  /// @{
  String kind_;     ///< RK method type (ODE23 or ODE45)
  idx_t order_;     ///< Integration order
  idx_t butchSize_; ///< Butcher tableau size
  Matrix a_;        ///< Butcher tableau A matrix
  Vector b_;        ///< Butcher tableau b vector
  bool fsal_;       ///< First Same As Last property
  Vector c_;        ///< Butcher tableau c vector
  /// @}
};
