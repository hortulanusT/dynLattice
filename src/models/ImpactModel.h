/**
 * @file ImpactModel.h
 * @author Til Gärtner
 * @brief Impact model for updating acceleration based on response forces and impactor weights
 */

#pragma once

#include <jem/base/Class.h>

#include <jem/base/System.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/fem/NodeGroup.h>
#include <jive/fem/NodeSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/util/Assignable.h>
#include <jive/util/Constraints.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Globdat.h>

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
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::DofSpace;
using jive::util::Globdat;

/// @brief Model for impact boundary conditions with weighted acceleration updates
/// @details Implements impact loading by updating acceleration based on response forces
/// and specified impactor weights for different node groups and DOF types.
class ImpactModel : public Model
{
public:
  JEM_DECLARE_CLASS(ImpactModel, Model);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;    ///< Model type name
  static const char *NODES_PROP;   ///< Node groups property
  static const char *DOF_PROP;     ///< DOF types property
  static const char *WEIGHTS_PROP; ///< Impactor weights property
  /// @}

  /// @brief Constructor with configuration and properties
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit ImpactModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Handle model actions for impact boundary conditions
  /// @param action Action name to execute
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat) override;

  /// @brief Factory method for creating new ImpactModel instances
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return Reference to new ImpactModel instance
  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Register ImpactModel type with ModelFactory
  static void declare();

private:
  /// @name System components
  /// @{
  Ref<DofSpace> dofs_;        ///< Degree of freedom space
  Ref<Constraints> cons_;     ///< Constraint manager
  Assignable<NodeSet> nodes_; ///< Node set
  /// @}

  /// @name Impact configuration
  /// @{
  StringVector nodeGroups_; ///< Node group names
  StringVector dofNames_;   ///< DOF type names
  Vector weights_;          ///< Impactor weights for each group/DOF
  /// @}
};
