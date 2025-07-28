/**
 * @file AdaptiveStepModule.h
 * @author Til Gärtner
 * @author Frans P. van der Meer
 * @brief Adaptive time stepping module for nonlinear analysis with plasticity
 *
 * This module implements adaptive time stepping with a NonlinModule for
 * adapting to explicit plasticity stepping.
 */

#pragma once

#include <cmath>
#include <jem/base/Class.h>
#include <jem/base/ClassTemplate.h>
#include <jem/base/StringObject.h>
#include <jem/util/Properties.h>
#include <jive/app/ModuleFactory.h>
#include <jive/implict/Names.h>
#include <jive/implict/NonlinModule.h>
#include <jive/implict/SolverInfo.h>
#include <jive/implict/SolverModule.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/Names.h>
#include <jive/model/StateVector.h>
#include <jive/solver/SolverException.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/Globdat.h>
#include <jive/util/utilities.h>

using jem::newInstance;
using jive::idx_t;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::app::Module;
using jive::implict::NonlinModule;
using jive::implict::SolverInfo;
using jive::implict::SolverModule;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::DofSpace;
using jive::util::FuncUtils;
using jive::util::Globdat;

//-----------------------------------------------------------------------
//   class AdaptiveStepModule
//-----------------------------------------------------------------------

/// @brief Module for adaptive time stepping in nonlinear analysis
/// @details Implements adaptive time step control by monitoring convergence behavior
/// and adjusting load increments automatically. Particularly useful for problems
/// with plasticity where step size needs to adapt to material nonlinearity.
class AdaptiveStepModule : public SolverModule
{
public:
  JEM_DECLARE_CLASS(AdaptiveStepModule, SolverModule);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  /// @}

  /// @brief Constructor with optional nonlinear solver
  /// @param name Module name
  /// @param solver Nonlinear solver module
  explicit AdaptiveStepModule(const String &name = "AdaptiveStep",
                              Ref<NonlinModule> solver = nullptr);

  /// @brief Initialize the module
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  virtual Status init(const Properties &conf,
                      const Properties &props,
                      const Properties &globdat) override;

  /// @brief Configure the module from properties
  /// @param props User-specified module properties
  /// @param globdat Global data container
  virtual void configure(const Properties &props,
                         const Properties &globdat) override;

  /// @brief Get current module configuration
  /// @param props Actually used configuration properties (output)
  /// @param globdat Global data container
  virtual void getConfig(const Properties &props,
                         const Properties &globdat) const override;

  /// @brief Advance to next time step
  /// @param globdat Global data container
  virtual void advance(const Properties &globdat) override;

  /// @brief Solve the current step with adaptive control
  /// @param info Solver information
  /// @param globdat Global data container
  virtual void solve(const Properties &info,
                     const Properties &globdat) override;

  /// @brief Cancel current solution attempt
  /// @param globdat Global data container
  virtual void cancel(const Properties &globdat) override;

  /// @brief Commit current solution
  /// @param globdat Global data container
  /// @return true if commit was successful
  virtual bool commit(const Properties &globdat) override;

  /// @brief Set convergence precision
  /// @param eps Convergence tolerance
  virtual inline void setPrecision(double eps) override;

  /// @brief Get current convergence precision
  /// @return Current convergence tolerance
  virtual inline double getPrecision() const override;

  /// @brief Factory method for creating new AdaptiveStepModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new AdaptiveStepModule instance
  static Ref<Module> makeNew(const String &name,
                             const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  /// @brief Register AdaptiveStepModule type with ModuleFactory
  static void declare();

protected:
  /// @brief Protected destructor
  virtual ~AdaptiveStepModule();

private:
  /// @name Solver components
  /// @{
  Ref<NonlinModule> solver_; ///< Nonlinear solver module
  Ref<Model> model_;         ///< Root of the model tree
  Ref<DofSpace> dofs_;       ///< Degree of freedom space
  /// @}

  /// @name Load control parameters
  /// @{
  double oldLoadScale_; ///< Previous load scale factor
  double loadScale_;    ///< Current load scale factor
  double incr_;         ///< Current load increment
  double minIncr_;      ///< Minimum allowed increment
  double maxIncr_;      ///< Maximum allowed increment
  double incrFact_;     ///< Increment increase factor
  double decrFact_;     ///< Increment decrease factor
  /// @}
};

//-----------------------------------------------------------------------
//   inline definitions
//-----------------------------------------------------------------------

/// @brief Set convergence precision for the underlying solver
/// @param eps Convergence tolerance
void AdaptiveStepModule::setPrecision(double eps)
{
  solver_->setPrecision(eps);
}

/// @brief Get current convergence precision from the underlying solver
/// @return Current convergence tolerance
double AdaptiveStepModule::getPrecision() const
{
  return solver_->getPrecision();
}
