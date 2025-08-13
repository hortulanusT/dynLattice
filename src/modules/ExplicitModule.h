/**
 * @file ExplicitModule.h
 * @author Til Gärtner
 * @brief Base class for explicit time integration methods
 *
 * This module provides the foundation for explicit time integration schemes
 * with support for rotational degrees of freedom and adaptive time stepping.
 */

#pragma once

#include "utils/helpers.h"
#include <jem/base/ArithmeticException.h>
#include <jem/base/Array.h>
#include <jem/base/Class.h>
#include <jem/base/ClassTemplate.h>
#include <jem/base/System.h>
#include <jem/base/array/operators.h>
#include <jem/util/Event.h>
#include <jem/util/Properties.h>
#include <jem/util/PropertyException.h>
#include <jive/algebra/AbstractMatrix.h>
#include <jive/algebra/DiagMatrixObject.h>
#include <jive/algebra/FlexMatrixBuilder.h>
#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/implict/Names.h>
#include <jive/implict/SolverInfo.h>
#include <jive/implict/SolverModule.h>
#include <jive/implict/utilities.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/StateVector.h>
#include <jive/solver/Solver.h>
#include <jive/solver/SolverParams.h>
#include <jive/solver/declare.h>
#include <jive/solver/utilities.h>
#include <jive/util/Constraints.h>
#include <jive/util/DenseTable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/Globdat.h>
#include <jive/util/ItemSet.h>
#include <jive/util/XTable.h>

using jem::idx_t;
using jem::newInstance;
using jem::numeric::Function;

using jive::IdxMatrix;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::StringVector;
using jive::Vector;
using jive::algebra::AbstractMatrix;
using jive::algebra::DiagMatrixObject;
using jive::algebra::FlexMatrixBuilder;
using jive::app::Module;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::implict::newSolverParams;
using jive::implict::SolverInfo;
using jive::implict::SolverModule;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::solver::newSolver;
using jive::solver::Solver;
using jive::util::Constraints;
using jive::util::DenseTable;
using jive::util::DofSpace;
using jive::util::FuncUtils;
using jive::util::Globdat;
using jive::util::ItemSet;
using jive::util::XTable;

using jive_helpers::expVec;
using jive_helpers::logMat;

//-----------------------------------------------------------------------
//   class ExplicitModule
//-----------------------------------------------------------------------

/// @brief Base class for explicit time integration schemes
/// @details Provides foundation for explicit solvers with support for rotational DOFs,
/// adaptive time stepping, and both lumped and consistent mass matrices. Handles
/// special integration of SO(3) rotational degrees of freedom using exponential maps.
class ExplicitModule : public SolverModule
{
public:
  /// @brief Mass matrix formulation modes
  enum MassMode
  {
    LUMPED,    ///< Lumped mass matrix
    CONSISTENT ///< Consistent mass matrix
  };

  JEM_DECLARE_CLASS(ExplicitModule, SolverModule);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;  ///< Module type name
  static const char *STEP_COUNT; ///< Step count property
  static const char *SO3_DOFS;   ///< SO(3) DOF types property
  static const char *LEN_SCALE;  ///< Length scale property
  /// @}

  /// @brief Initialize the module
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  virtual Status init(const Properties &conf,
                      const Properties &props,
                      const Properties &globdat) override;

  /// @brief Shutdown the module
  /// @param globdat Global data container
  virtual void shutdown(const Properties &globdat);

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

  /// @brief Solve current time step (pure virtual)
  /// @param info Solver information
  /// @param globdat Global data container
  virtual void solve(const Properties &info,
                     const Properties &globdat) = 0;

  /// @brief Cancel current solution attempt
  /// @param globdat Global data container
  virtual void cancel(const Properties &globdat) override;

  /// @brief Compute next step size and commit solution
  /// @param globdat Global data container
  /// @return true if step can be accepted
  virtual bool commit(const Properties &globdat) override;

  /// @brief Set convergence precision
  /// @param eps Convergence tolerance
  virtual void setPrecision(double eps) override;

  /// @brief Get current convergence precision
  /// @return Current convergence tolerance
  virtual double getPrecision() const override;

  // static Ref<Module> makeNew

  //     (const String &name, const Properties &conf,
  //      const Properties &props, const Properties &globdat);

  // static void declare();

protected:
  /// @brief Protected constructor
  /// @param name Module name
  explicit ExplicitModule(const String &name = "");

  /// @brief Protected destructor
  virtual ~ExplicitModule();

  /// @brief Update mass matrix
  /// @param globdat Global data container
  void updateMass_(const Properties &globdat);

  /// @brief invalidate_ current state
  void invalidate_();

  /// @brief Adams-Bashforth 2-step update
  /// @param delta_y Displacement increment
  /// @param f_cur Current force vector
  /// @param f_old Previous force vector
  inline void ABupdate(const Vector &delta_y,
                       const Vector &f_cur,
                       const Vector &f_old) const;

  /// @brief Adams-Bashforth 1-step update (explicit Euler)
  /// @param delta_y Displacement increment
  /// @param f_cur Current force vector
  inline void ABupdate(const Vector &delta_y,
                       const Vector &f_cur) const;

  /// @brief Update displacement vectors with optional SO(3) handling
  /// @param y_new New displacement vector
  /// @param y_old Old displacement vector
  /// @param delta_y Displacement increment
  /// @param rot Flag for rotational DOFs
  void updateVec(const Vector &y_new,
                 const Vector &y_old,
                 const Vector &delta_y,
                 const bool rot = false);

  /// @brief Get acceleration and return resulting force vector
  /// @param a Acceleration vector (output)
  /// @param cons Constraint manager
  /// @param fres Resulting force vector
  /// @param globdat Global data container
  void getAcce(const Vector &a,
               const Ref<Constraints> &cons,
               const Vector &fres,
               const Properties &globdat);

  /// @brief Get force vector
  /// @param fint Internal force vector
  /// @param fext External force vector
  /// @param globdat Global data container
  /// @return Resulting force vector (external - internal)
  Vector getForce(const Vector &fint,
                  const Vector &fext,
                  const Properties &globdat);

  /// @brief Get solution quality measure
  /// @param y_pre Predicted solution
  /// @param y_cor Corrected solution
  /// @return Quality measure
  double getQuality(const Vector &y_pre,
                    const Vector &y_cor);

protected:
  /// @name Module state
  /// @{
  bool valid_; ///< State validity flag
  /// @}

  /// @name Time stepping parameters
  /// @{
  double dtime_;    ///< Current time step size
  double prec_;     ///< Precision tolerance
  double minDtime_; ///< Minimum time step size
  double maxDtime_; ///< Maximum time step size
  double saftey_;   ///< Safety factor for step size control
  double incrFact_; ///< Step size increase factor
  double decrFact_; ///< Step size decrease factor
  /// @}

  /// @name Integration parameters
  /// @{
  MassMode mode_;   ///< Mass matrix mode
  idx_t order_;     ///< Integration order
  double lenScale_; ///< Characteristic length scale
  /// @}

  /// @name DOF management
  /// @{
  Ref<Function> updCond_; ///< Update condition function
  Vector massInv_;        ///< Inverse mass matrix
  IdxVector dofsSO3_;     ///< SO(3) DOF type indices
  IdxMatrix rdofs_;       ///< Rotational DOF mapping
  /// @}

  /// @name System components
  /// @{
  Ref<Model> model_;      ///< Root of the model tree
  Ref<DofSpace> dofs_;    ///< Degree of freedom space
  Ref<Constraints> cons_; ///< Constraint manager
  Ref<Solver> solver_;    ///< Linear solver
  /// @}
};

//-----------------------------------------------------------------------
//   inline definitions
//-----------------------------------------------------------------------

inline void ExplicitModule::ABupdate(const Vector &delta_y,
                                     const Vector &f_cur,
                                     const Vector &f_old) const
{
  delta_y = dtime_ / 2. * (3. * f_cur - 1. * f_old);
}

inline void ExplicitModule::ABupdate(const Vector &delta_y,
                                     const Vector &f_cur) const
{
  delta_y = dtime_ * f_cur;
}
