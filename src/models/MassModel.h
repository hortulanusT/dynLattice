/**
 * @file MassModel.h
 * @author Til Gärtner
 * @brief Mass model for adding concentrated masses to node groups
 */

#pragma once

#include <jem/base/Class.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/fem/NodeGroup.h>
#include <jive/fem/NodeSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>

using jem::Array;
using jive::idx_t;
using jive::IdxVector;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::StringVector;
using jive::Vector;
using jive::algebra::MatrixBuilder;
using jive::fem::DofSpace;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::util::Assignable;

/// @brief Model for adding concentrated masses to specified node groups
/// @details Applies concentrated mass values to node groups and DOF types,
/// contributing to the global mass matrix. Useful for modeling additional
/// masses, lumped inertias, or point masses attached to the structure.
class MassModel : public Model
{
public:
  JEM_DECLARE_CLASS(MassModel, Model);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;        ///< Model type name
  static const char *TOTAL_MASS_PROP;  ///< Total mass values
  static const char *NODE_GROUPS_PROP; ///< Node group names
  static const char *DOFS_PROP;        ///< DOF type names
  /// @}

  /// @}

  /// @brief Constructor with configuration and properties
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit MassModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Handle model actions for mass matrix assembly
  /// @param action Action name to execute
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat) override;

  /// @brief Factory method for creating new MassModel instances
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return Reference to new MassModel instance
  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Register MassModel type with ModelFactory
  static void declare();

protected:
  virtual ~MassModel();

private:
  /// @name Mass configuration
  /// @{
  Vector totalMass_;            ///< Mass values for each group
  StringVector nodeGroupNames_; ///< Node group names
  StringVector dofNames_;       ///< DOF type names
  /// @}

  /// @name System components
  /// @{
  Ref<DofSpace> dofs_;                      ///< Degree of freedom space
  IdxVector dofTypes_;                      ///< DOF type indices
  Array<Assignable<NodeGroup>> nodeGroups_; ///< Node groups
  /// @}
};
