/**
 * @file FollowerLoadModel.h
 * @author Til Gärtner
 * @brief Follower load model for direction-dependent loading
 */

#pragma once

#include <jem/base/Class.h>
#include <jem/util/Properties.h>

#include <jive/Array.h>
#include <jive/fem/NodeGroup.h>
#include <jive/fem/NodeSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>

#include "utils/helpers.h"
#include "utils/testing.h"

using jem::newInstance;
using jem::Ref;
using jem::String;
using jem::System;
using jem::util::Properties;

using jive::StringVector;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::Assignable;
using jive::util::DofSpace;

using jive_helpers::expVec;
using jive_helpers::eye;

/// @brief Model for applying follower loads that rotate with nodal rotations
/// @details Implements direction-dependent loading where force direction is updated
/// based on nodal rotational degrees of freedom. Useful for pressure loads and
/// other loads that maintain fixed orientation relative to deformed geometry.
class FollowerLoadModel : public Model
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;
  static const char *NODES_PROP;   ///< Node group property
  static const char *START_PROP;   ///< Initial force direction
  static const char *DOF_PROP;     ///< Force DOF types
  static const char *ROT_DOF_PROP; ///< Rotational DOF types
  /// @}

  JEM_DECLARE_CLASS(FollowerLoadModel, Model);

  /// @brief Constructor with configuration and properties
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit FollowerLoadModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Handle model actions (INIT, ADVANCE, GET_EXT_VECTOR)
  /// @param action Action name to execute
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat);

  /// @brief Factory method for creating new FollowerLoadModel instances
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return Reference to new FollowerLoadModel instance
  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Register FollowerLoadModel type with ModelFactory
  static void declare();

private:
  /// @brief Compute external force vector with current rotations
  /// @param fext External force vector to modify
  /// @param scale Load scale factor
  /// @param globdat Global data container
  void get_ext_vec_(const Vector &fext,
                    const double scale,
                    const Properties &globdat) const;

  /// @brief Update rotation matrices based on current displacements
  /// @param d Current displacement vector
  /// @details Computes rotation matrices from rotational DOF values using exponential map
  void advance_rots_(const Vector &d) const;

  /// @name System components
  /// @{
  Assignable<NodeSet> nodes_;   ///< Node set for boundary conditions
  String nameGroup_;            ///< Node group name
  Assignable<NodeGroup> group_; ///< Target node group
  Ref<DofSpace> dofs_;          ///< Degree of freedom space
  /// @}

  /// @name DOF management
  /// @{
  StringVector forceDOFs_; ///< Force DOF type names
  IdxVector iForceDOFs_;   ///< Force DOF type indices
  StringVector rotDOFs_;   ///< Rotational DOF type names
  IdxVector iRotDOFs_;     ///< Rotational DOF type indices
  /// @}

  /// @name Load parameters
  /// @{
  Vector orgDir_; ///< Original force direction vector
  Cubix rotMats_; ///< Rotation matrices for each node
  /// @}
};
