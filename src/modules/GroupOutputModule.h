/**
 * @file GroupOutputModule.h
 * @author Til Gärtner
 * @brief Module for extracting and storing group-based output data
 *
 * This module extracts displacement, velocity, acceleration, load, and force response
 * data from specified node and element groups and stores them in the global
 * database for post-processing and analysis.
 */

#pragma once

#include <jem/base/Array.h>
#include <jem/util/Properties.h>

#include <jive/Array.h>
#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/app/Names.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/NodeGroup.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/StateVector.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Globdat.h>

#include "utils/helpers.h"
#include "utils/testing.h"

using jem::idx_t;
using jem::newInstance;
using jem::Ref;
using jem::String;
using jem::sum;
using jem::util::Properties;

using jive::IdxVector;
using jive::Matrix;
using jive::StringVector;
using jive::Vector;
using jive::app::Module;
using jive::app::PropNames;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::DofSpace;
using jive::util::Globdat;

/// @brief Module for extracting and storing group-based output data
/// @details This module extracts various state variables (displacement, velocity,
/// acceleration, load, and response forces) from specified node and element groups
/// and stores them in the global database. For node groups, it computes:
/// - Sum of loads and response forces across all DOFs in the group
/// - Average displacement, velocity, and acceleration across all nodes in the group
///
/// For element groups, it extracts data from all nodes associated with the elements
/// in the group. The module supports flexible DOF specification and automatically
/// handles state vector retrieval and group-based aggregation.
///
/// The extracted data is stored in the global variables database under group names
/// and can be accessed by other modules or for post-processing.
class GroupOutputModule : public Module
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  /// @}

  JEM_DECLARE_CLASS(GroupOutputModule, Module);

  /// @brief Constructor
  /// @param name Module name (default: "groupOutput")
  explicit GroupOutputModule(const String &name = "groupOutput");

  /// @brief Initialize the module
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  /// @details Configures node and element groups, DOF specifications, and
  /// translates DOF names to indices. Performs initial data extraction.
  virtual Status init(const Properties &conf, const Properties &props,
                      const Properties &globdat);

  /// @brief Extract and store group-based output data
  /// @param globdat Global data container
  /// @return Module status
  /// @details Extracts state variables from all configured node and element groups
  /// and stores aggregated results in the global variables database
  virtual Status run(const Properties &globdat);

  /// @brief Factory method for creating new GroupOutputModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new GroupOutputModule instance
  static Ref<Module> makeNew(const String &name, const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  /// @brief Register GroupOutputModule type with ModuleFactory
  static void declare();

protected:
  /// @brief Protected destructor
  virtual ~GroupOutputModule();

protected:
  /// @name Group configuration
  /// @{
  StringVector nodeGroups_; ///< Node groups for singular element force/displacement calculations
  StringVector elemGroups_; ///< Element groups for extent/strain/stress value calculations
  /// @}

  /// @name DOF configuration
  /// @{
  StringVector nodeDofNames_; ///< DOF names for node group calculations
  IdxVector nodeDofs_;        ///< DOF indices for node group calculations
  StringVector elemDofNames_; ///< DOF names for element group calculations
  IdxVector elemDofs_;        ///< DOF indices for element group calculations
  /// @}
};
