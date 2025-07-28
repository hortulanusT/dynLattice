/**
 * @file InitLoadModel.h
 * @author Til Gärtner
 * @brief Initial loading model for setting displacement and velocity conditions
 */

#pragma once

#include <jem/base/Class.h>
#include <jem/util/Properties.h>

#include <jive/fem/NodeGroup.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>

using jem::idx_t;
using jem::newInstance;
using jive::IdxVector;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::StringVector;
using jive::Vector;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::Assignable;
using jive::util::DofSpace;

/// @brief Model for setting initial displacement and velocity conditions
/// @details Applies initial displacement and velocity values to specified node groups
/// and DOF types. Used for initialization of dynamic simulations with non-zero
/// starting conditions.
class InitLoadModel : public Model
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;   ///< Model type name
  static const char *DISP_GROUPS; ///< Displacement node groups
  static const char *DISP_DOFS;   ///< Displacement DOF types
  static const char *DISP_VALS;   ///< Displacement values
  static const char *VELO_GROUPS; ///< Velocity node groups
  static const char *VELO_DOFS;   ///< Velocity DOF types
  static const char *VELO_VALS;   ///< Velocity values
  /// @}

  JEM_DECLARE_CLASS(InitLoadModel, Model);

  /// @brief Constructor with configuration and properties
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit InitLoadModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Handle model actions for initial loading
  /// @param action Action name to execute
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat);

  /// @brief Factory method for creating new InitLoadModel instances
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return Reference to new InitLoadModel instance
  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Register InitLoadModel type with ModelFactory
  static void declare();

private:
  /// @brief Initialize model and apply initial conditions
  /// @param globdat Global data container
  void init_

      (const Properties &globdat);

  /// @name Displacement initialization
  /// @{
  StringVector dgroups_; ///< Displacement node group names
  StringVector ddofs_;   ///< Displacement DOF type names
  Vector dvals_;         ///< Displacement values
  /// @}

  /// @name Velocity initialization
  /// @{
  StringVector vgroups_; ///< Velocity node group names
  StringVector vdofs_;   ///< Velocity DOF type names
  Vector vvals_;         ///< Velocity values
  /// @}
};
