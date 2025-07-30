/**
 * @file LenientNonlinModule.h
 * @author F.P. van der Meer, T. Gärtner
 * @brief Lenient nonlinear solver module with relaxed convergence criteria
 *
 * This module extends the standard nonlinear solver to accept non-converged
 * solutions when a maximum number of iterations is reached. This is useful
 * for problems where strict convergence may not be achievable but a reasonable
 * approximate solution is sufficient for continued analysis.
 *
 * Original implementation: F.P. van der Meer, April 2014
 * Modified: T. Gärtner, January 2025
 * - Accept solution if not converged directly
 * - Check maximum iteration limit before accepting
 */

#pragma once

#include <jem/util/Properties.h>

#include <jive/implict/Names.h>
#include <jive/implict/NonlinModule.h>
#include <jive/model/Model.h>

using jem::idx_t;
using jem::Ref;
using jem::String;

using jem::util::Properties;
using jive::app::Module;
using jive::implict::NonlinModule;
using jive::model::Model;

/// @brief Lenient nonlinear solver with relaxed convergence criteria
/// @details This module extends the standard NonlinModule to handle cases where
/// strict convergence cannot be achieved within the maximum iteration limit.
/// When a "no convergence" exception is thrown and the maximum number of iterations
/// has been reached, the module accepts the current solution and continues the
/// analysis. This is particularly useful for:
/// - Contact problems with convergence difficulties
/// - Analyses that need to continue despite local convergence issues
///
/// The module maintains all standard nonlinear solver functionality while
/// providing the flexibility to proceed with non-converged solutions when
/// necessary.
class LenientNonlinModule : public NonlinModule
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  /// @}

  JEM_DECLARE_CLASS(LenientNonlinModule, NonlinModule);
  /// @brief Constructor
  /// @param name Module name (default: "lenient")
  explicit LenientNonlinModule(const String &name = "lenient");

  /// @brief Factory method for creating new LenientNonlinModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new LenientNonlinModule instance
  static Ref<Module> makeNew(const String &name,
                             const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  /// @brief Register LenientNonlinModule type with ModuleFactory
  static void declare();

  /// @brief Initialize the module
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  /// @details Initializes the base NonlinModule and retrieves the maximum
  /// iteration limit for lenient convergence behavior
  virtual Status init(const Properties &conf,
                      const Properties &props,
                      const Properties &globdat);

  /// @brief Run the lenient nonlinear solver
  /// @param globdat Global data container
  /// @return Module status
  /// @details Executes the nonlinear solution process with lenient convergence
  /// criteria. If convergence fails after maxIter_ iterations, accepts the
  /// current solution and continues
  virtual Status run(const Properties &globdat);

  /// @brief Configure module parameters
  /// @param props Configuration properties
  /// @param globdat Global data container
  /// @details Delegates to base class configuration
  virtual void configure(const Properties &props,
                         const Properties &globdat);

  /// @brief Get current configuration
  /// @param conf Configuration properties (output)
  /// @param globdat Global data container
  /// @details Delegates to base class configuration retrieval
  virtual void getConfig(const Properties &conf,
                         const Properties &globdat) const;

protected:
  /// @brief Protected destructor
  virtual ~LenientNonlinModule();

private:
  /// @name Solver parameters
  /// @{
  idx_t maxIter_; ///< Maximum iterations before accepting non-converged solution
  /// @}
};
