/**
 * @file MilneDeviceModule.h
 * @author Til Gärtner
 * @brief Milne device predictor-corrector method for explicit time integration
 *
 * This module implements Milne's device, a predictor-corrector algorithm that
 * combines Adams-Bashforth predictor steps with Adams-Moulton corrector steps
 * for explicit time integration with improved stability and accuracy.
 */
#pragma once

#include "modules/ExplicitModule.h"

/// @brief Module implementing Milne's device predictor-corrector method
/// @details This module implements Milne's device, a predictor-corrector scheme
/// that combines Adams-Bashforth predictor steps with Adams-Moulton corrector
/// steps for explicit time integration. The method proceeds in three phases:
///
/// 1. **Predictor step**: Uses Adams-Bashforth method to predict state at next time
/// 2. **Corrector step**: Uses Adams-Moulton method to correct the prediction
/// 3. **Step size adaptation**: Evaluates solution quality for potential step size control
///
/// The predictor-corrector approach provides better stability and accuracy
/// compared to simple explicit methods while maintaining computational efficiency.
/// The module supports both first-order (Euler implicit) and second-order
/// (trapezoidal rule) Adams-Moulton correctors.
///
/// @see [Milne (1926)](https://doi.org/10.1080/00029890.1926.11986619)
class MilneDeviceModule : public ExplicitModule
{
public:
  JEM_DECLARE_CLASS(MilneDeviceModule, ExplicitModule);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  /// @}

  /// @brief Constructor
  /// @param name Module name (default: "MilneDevice")
  explicit MilneDeviceModule(const String &name = "MilneDevice");

  /// @brief Solve using Milne's device predictor-corrector method
  /// @param info Solver information (output)
  /// @param globdat Global data container
  /// @details Performs one time step using predictor-corrector algorithm:
  /// 1. Predict velocities and displacements using Adams-Bashforth
  /// 2. Correct using Adams-Moulton with updated forces
  /// 3. Evaluate solution quality for step size adaptation
  virtual void solve(const Properties &info, const Properties &globdat);

  /// @brief Factory method for creating new MilneDeviceModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new MilneDeviceModule instance
  static Ref<Module> makeNew(const String &name, const Properties &conf,
                             const Properties &props, const Properties &globdat);

  /// @brief Register MilneDeviceModule type with ModuleFactory
  static void declare();

  /// @brief Destructor
  virtual ~MilneDeviceModule();

protected:
  /// @brief Update forces for corrector step
  /// @param fint Internal force vector (modified)
  /// @param fext External force vector
  /// @param globdat Global data container
  /// @return Resulting force vector (external - internal)
  /// @details In the corrector step, only internal forces change due to
  /// updated displacement field, so external forces remain constant
  Vector updForce(const Vector &fint,
                  const Vector &fext,
                  const Properties &globdat);

private:
  /// @name Adams-Moulton corrector methods
  /// @{

  /// @brief Adams-Moulton 2-step corrector (Trapezoidal rule)
  /// @param delta_y Change in state variable (output)
  /// @param f_pre Force at previous time step
  /// @param f_cur Force at current time step
  /// @details Implements trapezoidal rule: Δy = Δt/2 * (f_prev + f_cur)
  inline void AMupdate_(const Vector &delta_y, const Vector &f_pre,
                        const Vector &f_cur) const;

  /// @brief Adams-Moulton 1-step corrector (Euler implicit)
  /// @param delta_y Change in state variable (output)
  /// @param f_pre Force at previous time step
  /// @details Implements backward Euler: Δy = Δt * f_prev
  inline void AMupdate_(const Vector &delta_y, const Vector &f_pre) const;
  /// @}
};

//=======================================================================
//   Inline implementations
//=======================================================================

inline void MilneDeviceModule::AMupdate_(const Vector &delta_y,
                                         const Vector &f_pre,
                                         const Vector &f_cur) const
{
  delta_y = dtime_ / 2. * (1. * f_pre + 1. * f_cur);
}

inline void MilneDeviceModule::AMupdate_(const Vector &delta_y,
                                         const Vector &f_pre) const
{
  delta_y = dtime_ * f_pre;
}
