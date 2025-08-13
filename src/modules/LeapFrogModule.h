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
/// ExplicitModule.
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
  /// @param name Module name (default: "leapFrog")
  explicit LeapFrogModule(const String &name = "leapFrog");

  /// @brief Solve using leap-frog integration scheme
  /// @param info Solver information (output)
  /// @param globdat Global data container
  /// @details Performs one time step using the leap-frog algorithm:
  /// 1. Compute forces and accelerations at current time
  /// 2. Update velocities using current and previous accelerations
  /// 3. Update positions using updated velocities
  virtual void solve(const Properties &info, const Properties &globdat) override;

  /// @brief Factory method for creating new LeapFrogModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new LeapFrogModule instance
  static Ref<Module> makeNew(const String &name, const Properties &conf,
                             const Properties &props, const Properties &globdat);

  /// @brief Register LeapFrogModule type with ModuleFactory
  static void declare();

protected:
  /// @brief Protected destructor
  virtual ~LeapFrogModule();
};
