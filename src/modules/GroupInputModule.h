/**
 * @file GroupInputModule.h
 * @author Frans P. van der Meer
 * @brief Module for generating node and element groups from input files
 */

#pragma once

#include <jem/base/Class.h>
#include <jive/app/Module.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/fem/NodeSet.h>
#include <jive/util/Assignable.h>
#include <jive/util/Constraints.h>
#include <jive/util/Globdat.h>
#include <jive/util/XDofSpace.h>

using jem::ALL;
using jem::Array;
using jem::Error;
using jem::idx_t;
using jem::max;
using jem::maxOf;
using jem::min;
using jem::newInstance;
using jem::Ref;
using jem::String;
using jem::System;
using jem::Tuple;
using jem::util::Properties;

using jive::IdxVector;
using jive::Matrix;
using jive::StringVector;
using jive::Vector;
using jive::app::Module;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::fem::NodeSet;
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::Globdat;
using jive::util::XDofSpace;

//-----------------------------------------------------------------------
//   class GroupInputModule
//-----------------------------------------------------------------------

/// @brief Module for generating node and element groups from input specifications
/// @details Creates NodeGroups and ElementGroups based on geometric criteria
/// such as coordinate ranges, minimum/maximum values, and element restrictions.
/// Supports complex group definitions with partial element selections.
class GroupInputModule : public Module
{
public:
  JEM_DECLARE_CLASS(GroupInputModule, Module);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;   ///< Module type name
  static const char *NODE_GROUPS; ///< Node groups property
  static const char *ELEM_GROUPS; ///< Element groups property
  /// @}

  /// @brief Constructor
  /// @param name Module name
  explicit GroupInputModule(const String &name = "groupInput");

  /// @brief Initialize the module and create groups
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  virtual Status init(const Properties &conf,
                      const Properties &props,
                      const Properties &globdat);

  /// @brief Factory method for creating new GroupInputModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new GroupInputModule instance
  static Ref<Module> makeNew(const String &name,
                             const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  /// @brief Register GroupInputModule type with ModuleFactory
  static void declare();

protected:
  virtual ~GroupInputModule();

private:
  /// @brief Number of nodes in the mesh
  idx_t numNodes_;

  /// @brief Configure module from properties
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  void configure_(const Properties &conf,
                  const Properties &props,
                  const Properties &globdat);

  /// @brief Create all node groups from global data
  /// @param globdat Global data container with group definitions
  void makeNodeGroups_(const Properties &globdat);

  //=======================================================================
  //   class NGroup_
  //=======================================================================

  /// @brief Helper class for generating node groups from input specifications
  /// @details Creates NodeGroups based on geometric criteria such as coordinate
  /// ranges, minimum/maximum values, and element restrictions. Supports
  /// complex group definitions with partial element selections.
  class NGroup_ : public Object
  {
  public:
    /// @brief Constructor
    explicit NGroup_();

    /// @brief Destructor
    ~NGroup_();

    /// @brief Configure node group from properties
    /// @param conf Actually used configuration properties (output)
    /// @param props Group specification properties
    /// @param name Group name
    void configure(const Properties &conf,
                   const Properties &props,
                   const String &name);

    /// @brief Generate the node group and store in global data
    /// @param globdat Global data container
    void makeGroup(const Properties &globdat);

    /// @name Class constants
    /// @{
    static const char *X_NAMES[3]; ///< Coordinate dimension names ["x", "y", "z"]
    static const double PI;        ///< Pi constant for angular calculations
    /// @}

    StringVector knownTypes; ///< Known coordinate selection types

    /// @brief Coordinate checking modes
    enum Check
    {
      NONE,  ///< No checking in this dimension
      VALUE, ///< Check for specific value
      BOUNDS ///< Check within bounds
    };

  private:
    /// @brief Find candidate nodes from element restrictions
    /// @param elems Element set to search
    /// @param globdat Global data container
    void findCandidates_(const ElementSet &elems,
                         const Properties &globdat);

    /// @brief Find nodes matching coordinate criteria
    /// @param nodes Node set to search
    void findNodes_(const NodeSet &nodes);

    /// @brief Store the generated node group
    /// @param nodes Node set containing the nodes
    /// @param globdat Global data container
    void store_(const NodeSet &nodes,
                const Properties &globdat);

    /// @name Configuration parameters
    /// @{
    idx_t rank_;    ///< Spatial dimension
    bool all_;      ///< Select all nodes flag
    double eps_;    ///< Tolerance for coordinate matching
    double angle_;  ///< Angle tolerance for geometric operations
    double radius_; ///< Radius for circular/spherical selections
    /// @}

    /// @brief Coordinate checking modes for each dimension
    Array<Check> doX_;

    /// @brief Coordinate values for each dimension
    Tuple<Vector, 3> xvals_;

    /// @brief Coordinate types for each dimension ("min", "max", "value")
    Tuple<String, 3> xtype_;

    /// @brief Coordinate bounds matrix
    Matrix xbounds_;

    /// @name Group data
    /// @{
    IdxVector inodes_;          ///< Selected node indices
    String myName_;             ///< Group name
    StringVector restrictTo_;   ///< Element groups to restrict to
    IdxVector restrictPartial_; ///< Partial element node indices
    /// @}
  };

  //=======================================================================
  //   class EGroup_
  //=======================================================================

  /// @brief Helper class for generating element groups from input specifications
  /// @details Creates ElementGroups based on geometric criteria such as coordinate
  /// ranges, minimum/maximum values. Supports element-based geometric selections
  /// for grouping elements by spatial location.
  class EGroup_ : public Object
  {
  public:
    /// @brief Constructor
    explicit EGroup_();

    /// @brief Destructor
    ~EGroup_();

    /// @brief Configure element group from properties
    /// @param conf Actually used configuration properties (output)
    /// @param props Group specification properties
    /// @param name Group name
    void configure(const Properties &conf,
                   const Properties &props,
                   const String &name);

    /// @brief Generate the element group and store in global data
    /// @param globdat Global data container
    void makeGroup(const Properties &globdat);

    /// @name Class constants
    /// @{
    static const char *X_NAMES[3]; ///< Coordinate dimension names ["x", "y", "z"]
    static const double PI;        ///< Pi constant for angular calculations
    /// @}

    StringVector knownTypes; ///< Known coordinate selection types

    /// @brief Coordinate checking modes
    enum Check
    {
      NONE,  ///< No checking in this dimension
      VALUE, ///< Check for specific value
      BOUNDS ///< Check within bounds
    };

  private:
    /// @brief Find elements matching coordinate criteria
    /// @param elems Element set to search
    /// @param globdat Global data container
    void findElems_(const ElementSet &elems,
                    const Properties &globdat);

    /// @brief Store the generated element group
    /// @param elems Element set containing the elements
    /// @param globdat Global data container
    void store_(const ElementSet &elems,
                const Properties &globdat);

    /// @name Configuration parameters
    /// @{
    idx_t rank_; ///< Spatial dimension
    bool all_;   ///< Select all elements flag
    double eps_; ///< Tolerance for coordinate matching
    /// @}

    /// @brief Coordinate checking modes for each dimension
    Array<Check> doX_;

    /// @brief Coordinate values for each dimension
    Tuple<Vector, 3> xvals_;

    /// @brief Coordinate types for each dimension ("min", "max", "value")
    Tuple<String, 3> xtype_;

    /// @brief Completeness requirement for each dimension
    Tuple<bool, 3> completely_;

    /// @brief Coordinate bounds matrix
    Matrix xbounds_;

    /// @name Group data
    /// @{
    IdxVector ielems_; ///< Selected element indices
    String myName_;    ///< Group name
    String parent_;    ///< Parent group name for hierarchical groups
    /// @}
  };
};
