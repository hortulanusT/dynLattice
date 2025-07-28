/**
 * @file DirichletModel.h
 * @author Frans P. van der Meer
 * @brief Dirichlet boundary condition model for displacement control
 */

#pragma once

#include <jem/io/Writer.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/fem/NodeSet.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/Names.h>
#include <jive/util/Assignable.h>
#include <jive/util/Constraints.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Globdat.h>

using jem::Float;
using jem::idx_t;
using jem::IllegalInputException;
using jem::newInstance;
using jem::NIL;
using jem::Ref;
using jem::SliceFrom;
using jem::String;
using jem::System;
using jem::io::Writer;
using jem::util::Properties;
using jive::IdxVector;
using jive::StringVector;
using jive::Vector;
using jive::fem::NodeSet;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::DofSpace;
using jive::util::Globdat;

//-----------------------------------------------------------------------
//   class DirichletModel
//-----------------------------------------------------------------------

/// @brief Model for applying Dirichlet boundary conditions with displacement control
/// @details Implements displacement-based boundary conditions using rate, increment, or load scale methods.
/// Supports multiple node groups with different displacement factors and DOF types.
class DirichletModel : public Model
{
public:
  /// @brief Displacement control methods
  enum Method
  {
    RATE,      ///< Displacement rate control
    INCREMENT, ///< Displacement increment control
    LOADSCALE  ///< Load scale factor control
  };

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;

  static const char *MAX_DISP_PROP;  ///< Maximum displacement value
  static const char *DISP_INCR_PROP; ///< Displacement increment
  static const char *DISP_RATE_PROP; ///< Displacement rate
  static const char *INIT_DISP_PROP; ///< Initial displacement
  static const char *NODES_PROP;     ///< Node groups
  static const char *DOF_PROP;       ///< Degree of freedom types
  static const char *FACTORS_PROP;   ///< Displacement factors
  static const char *LOADED_PROP;    ///< Loaded node group index
  /// @}

  JEM_DECLARE_CLASS(DirichletModel, Model);

  /// @brief Constructor with optional child model
  /// @param name Model name (default: "arclen")
  /// @param child Optional child model reference
  explicit DirichletModel

      (const String &name = "arclen", const Ref<Model> &child = NIL);

  /// @brief Configure Dirichlet boundary conditions from properties
  /// @param props User-specified configuration properties
  /// @param globdat Global data container
  virtual void configure

      (const Properties &props, const Properties &globdat);

  /// @brief Get current configuration
  /// @param conf Actually used configuration properties (output)
  /// @param globdat Global data container
  virtual void getConfig

      (const Properties &conf, const Properties &globdat) const;

  /// @brief Handle model actions (INIT, GET_CONSTRAINTS, ADVANCE, etc.)
  /// @param action Action name to execute
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction

      (const String &action, const Properties &params,
       const Properties &globdat);

  /// @brief Factory method for creating new DirichletModel instances
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return Reference to new DirichletModel instance
  static Ref<Model> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  /// @brief Register DirichletModel type with ModelFactory
  static void declare();

protected:
  virtual ~DirichletModel();

protected:
  /// @brief Initialize model state and DOF mappings
  /// @param globdat Global data container with node set and DOF space
  void init_

      (const Properties &globdat);

  /// @brief Advance to next time step
  /// @param globdat Global data container
  /// @details Updates displacement scale factor and logs progress
  void advance_

      (const Properties &globdat);

  /// @brief Apply displacement constraints to system
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @details Iterates through node groups and applies scaled displacement constraints
  void applyConstraints_

      (const Properties &params, const Properties &globdat);

  /// @brief Check if current state should be committed
  /// @param params Action parameters containing termination flags
  /// @param globdat Global data container
  /// @details Terminates computation if displacement exceeds maximum value
  void checkCommit_

      (const Properties &params, const Properties &globdat);

  /// @brief Commit current displacement state
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @details Stores converged displacement scale factor
  void commit_

      (const Properties &params, const Properties &globdat);

  /// @brief Set time step size for displacement control
  /// @param params Parameters containing step size information
  /// @details Calculates displacement increment based on control method
  void setDT_

      (const Properties &params);

private:
  /// @name Core system components
  /// @{
  Ref<DofSpace> dofs_;        ///< Degree of freedom space
  Ref<Constraints> cons_;     ///< Constraint manager
  Assignable<NodeSet> nodes_; ///< Node set for boundary conditions
  /// @}

  /// @name DOF management
  /// @{
  idx_t ngroups_;   ///< Number of node groups
  IdxVector idofs_; ///< DOF indices for constraints
  /// @}

  /// @name Displacement control state
  /// @{
  double dispScale0_; ///< Previous displacement scale factor
  double dispScale_;  ///< Current displacement scale factor
  double dispIncr_;   ///< Current displacement increment
  double dispRate_;   ///< Displacement rate
  Method method_;     ///< Displacement control method
  /// @}

  /// @name Boundary condition specification
  /// @{
  StringVector nodeGroups_; ///< Node group names
  StringVector dofTypes_;   ///< DOF type names for each group
  Vector factors_;          ///< Displacement factors for each group
  /// @}

  /// @name Configuration parameters
  /// @{
  double dispIncr0_;    ///< Initial displacement increment
  double maxDispVal_;   ///< Maximum absolute displacement value
  double initDisp_;     ///< Initial displacement value
  Properties lbcProps_; ///< Boundary condition properties
  String varName_;      ///< Variable name for load factor
  /// @}
};
