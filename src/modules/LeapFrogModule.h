/**
 * @file LeapFrogModule.h
 * @author Til Gärtner
 * @brief Leap-frog time integration algorithm for explicit dynamics
 *
 * Leap-frog in its velocity-Verlet form, with fixed time steps.
 */
#pragma once

#include "modules/ExplicitModule.h"

/// @brief Module for leap-frog time integration
/// @details Leap-frog in its velocity-Verlet form: the positions are those of leap-frog, but displacements,
/// velocities and accelerations are all stored at the full step, t_n:
/// - kick: v_{n+1/2} = v_n + dt/2 a_n
/// - drift: u_{n+1} = u_n + dt v_{n+1/2} (rotations: exp(dt w_{n+1/2}) exp(u_n))
/// - a_{n+1} at u_{n+1} and t_{n+1}; the gyroscopic force in it uses v_{n+1/2} + dt/2 a_n for v_{n+1}
/// - kick: v_{n+1} = v_{n+1/2} + dt/2 a_{n+1}
///
/// Second order, also for the rotations and with time-dependent loads; one force evaluation per step.
/// a_n is kept in the third state vector. It has no error estimate and runs with the fixed step deltaTime.
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
  /// @details One step kick-drift-kick, see the class description.
  virtual void solve(const Properties &info, const Properties &globdat) override;

  /// @brief Initialise as ExplicitModule (which fixes the step through configure)
  /// @param[out] conf  Actually used configuration properties (output)
  /// @param[in] props  User-specified module properties
  /// @param[in] globdat  Global data container
  /// @return Module status
  virtual Status init(const Properties &conf, const Properties &props, const Properties &globdat) override;

  /// @brief Configure as ExplicitModule, then fix the step size to deltaTime
  /// @param[in] props  User-specified module properties
  /// @param[in] globdat  Global data container
  virtual void configure(const Properties &props, const Properties &globdat) override;

  /// @brief Cancel as ExplicitModule; the acceleration is evaluated again at the restored state
  /// @param[in] globdat  Global data container
  virtual void cancel(const Properties &globdat) override;

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
  /// @brief The stored acceleration no longer belongs to the state (dof space changed)
  void invalidateAcce_();

  bool accValid_; ///< The acceleration in the third state vector is a_n of the committed state
};
