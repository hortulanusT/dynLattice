/**
 * @file LeapFrogModule.h
 * @author Til Gärtner
 * @brief Leap-frog time integration algorithm for explicit dynamics
 *
 * This module implements the leap-frog time integration scheme for explicit
 * dynamic analysis. The leap-frog method is a second-order accurate symplectic
 * integrator that maintains energy conservation properties.
 *
 * @note this is an old and largely untested module, use with caution
 */
#pragma once

#include "modules/ExplicitModule.h"

/// @brief Module for leap-frog time integration
/// @details Implements the leap-frog time integration scheme for explicit
/// dynamic analysis. This method uses a staggered time stepping approach where:
/// - Positions are computed at integer time steps (n, n+1, n+2, ...)
/// - Velocities are computed at half time steps (n+1/2, n+3/2, ...)
/// - Accelerations are computed at integer time steps
///
/// The leap-frog method is symplectic and second-order accurate, making it
/// well-suited for long-time dynamic simulations where energy conservation
/// is important. It inherits basic explicit integration functionality from
/// ExplicitModule. It has no error estimate and runs with the fixed step deltaTime.
/// The initial velocity is at t = 0, so the first step is a half kick; from then on velocities are stored at the
/// half steps: E_kin lags E_pot by half a step.
/// @see [Leap-frog integration](https://en.wikipedia.org/wiki/Leapfrog_integration)
class LeapFrogModule : public ExplicitModule
{
 public:
  JEM_DECLARE_CLASS(LeapFrogModule, ExplicitModule);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  /// @}

  /// @brief Constructor
  /// @param[in] name  Module name (default: "leapFrog")
  explicit LeapFrogModule(const String &name = "leapFrog");

  /// @brief Solve using leap-frog integration scheme
  /// @param[out] info  Solver information (output)
  /// @param[in] globdat  Global data container
  /// @details Performs one time step using the leap-frog algorithm:
  /// 1. Compute forces and accelerations at current time
  /// 2. Update velocities using current and previous accelerations
  /// 3. Update positions using updated velocities
  virtual void solve(const Properties &info, const Properties &globdat) override;

  /// @brief Initialise as ExplicitModule, then fix the step size to deltaTime
  /// @param[out] conf  Actually used configuration properties (output)
  /// @param[in] props  User-specified module properties
  /// @param[in] globdat  Global data container
  /// @return Module status
  virtual Status init(const Properties &conf, const Properties &props, const Properties &globdat) override;

  /// @brief Configure as ExplicitModule, then fix the step size to deltaTime
  /// @param[in] props  User-specified module properties
  /// @param[in] globdat  Global data container
  virtual void configure(const Properties &props, const Properties &globdat) override;

  /// @brief Commit as ExplicitModule; the first committed step ends the half kick
  /// @param[in] globdat  Global data container
  /// @return Whether the step was accepted
  virtual bool commit(const Properties &globdat) override;

  /// @brief Factory method for creating new LeapFrogModule instances
  /// @param[in] name  Module name
  /// @param[out] conf  Actually used configuration properties (output)
  /// @param[in] props  User-specified module properties
  /// @param[in] globdat  Global data container
  /// @return Reference to new LeapFrogModule instance
  static Ref<Module> makeNew(const String &name, const Properties &conf,
                             const Properties &props, const Properties &globdat);

  /// @brief Register LeapFrogModule type with ModuleFactory
  static void declare();

 protected:
  /// @brief Protected destructor
  virtual ~LeapFrogModule();

 private:
  bool firstStep_; ///< Until the first step of this module is committed: its velocity update is a half kick
};
