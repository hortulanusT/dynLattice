/**
 * @file PBCGroupInputModule.h
 * @author Frans van der Meer
 * @brief Module for generating periodic boundary condition node groups
 *
 * Automatically creates node groups for periodic boundary conditions by
 * identifying boundary nodes on faces, edges, and corners of rectangular domains.
 */

#pragma once

#include <jive/app/Module.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/fem/NodeSet.h>
#include <jive/util/Assignable.h>
#include <jive/util/Constraints.h>
#include <jive/util/XDofSpace.h>

#include "GroupInputModule.h"

/// @brief Module for automatic periodic boundary condition node group generation
/// @details Extends GroupInputModule to automatically create node groups for
/// periodic boundary conditions. Identifies boundary nodes on domain faces,
/// edges, and corners based on coordinate analysis and generates appropriate
/// node groups for PBC implementation.
class PBCGroupInputModule : public GroupInputModule
{
public:
  /// @name Type definitions
  /// @{
  typedef PBCGroupInputModule Self; ///< Self type alias
  typedef GroupInputModule Super;   ///< Base class type alias
  /// @}

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;       ///< Module type name
  static const char *EDGES[6];        ///< Edge group names (xmin, xmax, etc.)
  static const char *CORNERS[4];      ///< Corner group names
  static const char *DUPEDNODES_PROP; ///< Duplicated nodes property name
  static const char *NGROUPS_PROP;    ///< Node groups property name
  /// @}

  /// @brief Constructor
  /// @param name Module name (default: "pbcGroupInput")
  explicit PBCGroupInputModule(const String &name = "pbcGroupInput");

  /// @brief Initialize PBC node group generation
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  virtual Status init(const Properties &conf, const Properties &props,
                      const Properties &globdat);

  /// @brief Factory method for creating new instances
  /// @param name Module name
  /// @param conf Configuration properties (output)
  /// @param props User properties
  /// @param globdat Global data container
  /// @return New module instance
  static Ref<Module> makeNew(const String &name, const Properties &conf,
                             const Properties &props, const Properties &globdat);

  /// @brief Register module type with factory
  static void declare();

protected:
  /// @brief Protected destructor
  virtual ~PBCGroupInputModule();

  /// @brief Prepare properties for PBC group generation
  /// @param myProps Module properties to configure
  void prepareProps_(const Properties &myProps) const;

  /// @brief Sort boundary nodes into master/slave pairs
  /// @param islaves Slave node indices
  /// @param imasters Master node indices
  /// @param nodes Node set
  /// @param globdat Global data container
  /// @param ix Coordinate direction index
  void sortBoundaryNodes_(const IdxVector &islaves, const IdxVector &imasters,
                          const NodeSet &nodes, const Properties &globdat,
                          const idx_t ix) const;

protected:
  /// @name Configuration parameters
  /// @{
  idx_t rank_;               ///< Spatial dimension of the domain
  double small_;             ///< Tolerance for coordinate comparison
  bool edges_;               ///< Generate edge node groups
  bool corners_;             ///< Generate corner node groups
  Properties groupSettings_; ///< Group configuration settings
  String dupedNodeGroup_;    ///< Name of duplicated node group
  /// @}
};
