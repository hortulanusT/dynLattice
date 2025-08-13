/**
 * @file GMSHInputModule.h
 * @author Til Gärtner
 * @brief Module for importing geometry using the GMSH C++ API
 *
 * This module provides geometry import capabilities using GMSH mesh generator
 * with support for various element types and mesh orders.
 */

#pragma once

#include <filesystem>
#include <gmsh.h>

#include <jem/base/Array.h>
#include <jem/base/CString.h>
#include <jem/base/Class.h>
#include <jem/base/System.h>
#include <jem/util/ArrayBuffer.h>
#include <jem/util/HashDictionary.h>
#include <jem/util/Properties.h>

#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/app/Names.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/XElementSet.h>
#include <jive/fem/XNodeSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/DenseTable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/Globdat.h>
#include <jive/util/XTable.h>

using jem::Array;
using jem::max;
using jem::newInstance;

using jive::ALL;
using jive::idx_t;
using jive::IdxMatrix;
using jive::IdxVector;
using jive::Matrix;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::StringVector;
using jive::Vector;

using jive::app::Module;
using jive::fem::ElementGroup;
using jive::fem::XElementSet;
using jive::fem::XNodeSet;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::Function;
using jive::util::Globdat;

typedef jem::util::ArrayBuffer<idx_t> IdxBuffer;

static_assert(GMSH_API_VERSION_MAJOR == 4, "Wrong GMSH API version, please use 4.9.5");
static_assert(GMSH_API_VERSION_MINOR == 9, "Wrong GMSH API version, please use 4.9.5");
static_assert(GMSH_API_VERSION_PATCH == 5, "Wrong GMSH API version, please use 4.9.5");

/// @brief Module for importing geometry and mesh data using GMSH API
/// @details Translates GMSH geometry files to JIVE data structures with support
/// for various element orders, mesh dimensions, and output formats. Requires
/// GMSH API version 4.9.5 for compatibility.
class GMSHInputModule : public Module
{
public:
  JEM_DECLARE_CLASS(GMSHInputModule, Module);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;       ///< Module type name
  static const char *ORDER;           ///< Mesh order property
  static const char *MESH_DIM;        ///< Mesh dimension property
  static const char *SAVE_MSH;        ///< Save MSH file property
  static const char *STORE_TANGENTS;  ///< Store tangents property
  static const char *ENTITY_NAMES[4]; ///< Entity names array
  static const char *ONELAB_PROPS;    ///< ONELAB properties
  static const char *VERBOSE;         ///< Verbose output property
  static const char *OUT_FILE;        ///< Output file property
  static const char *OUT_TABLES;      ///< Output tables property
  /// @}

  /// @brief Constructor
  /// @param name Module name
  explicit GMSHInputModule(const String &name = "GMSHInput");

  /// @brief Initialize the module
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  Status init(const Properties &conf,
              const Properties &props,
              const Properties &globdat) override;

  /// @brief Run mesh import process
  /// @param globdat Global data container
  /// @return Module status
  Status run(const Properties &globdat) override;

  /// @brief Shutdown the module
  /// @param globdat Global data container
  void shutdown(const Properties &globdat) override;

  /// @brief Factory method for creating new GMSHInputModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new GMSHInputModule instance
  static Ref<Module> makeNew(const String &name,
                             const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  /// @brief Register GMSHInputModule type with ModuleFactory
  static void declare();

protected:
  /// @brief Protected destructor
  ~GMSHInputModule();

  /// @brief Open geometry file and generate mesh
  /// @param geoFile Name of the .geo file
  /// @param order Mesh order (1=linear, 2=quadratic, etc.)
  void openMesh_(const String &geoFile, const idx_t order);

  /// @brief Prepare ONELAB properties for mesh generation
  /// @param onelabProps ONELAB properties to set
  void prepareOnelab_(const Properties &onelabProps);

  /// @brief Populate nodes in global database
  /// @param dim Embedding space dimension (coordinates per node)
  void createNodes_(const idx_t dim);

  /// @brief Populate elements in global database
  /// @param globdat Global database
  void createElems_(const Properties &globdat);

  /// @brief Store tangent vectors in global database
  /// @param globdat Global database
  void storeTangents_(const Properties &globdat);

  /// @brief Write output file for GMSH post-processing
  /// @param globdat Global database
  void writeOutFile_(const Properties &globdat) const;

private:
  /// @name GMSH-JIVE mapping
  /// @{
  Assignable<XNodeSet> nodes_;                               ///< Node set
  std::unordered_map<std::size_t, idx_t> gmshToJiveNodeMap_; ///< GMSH to JIVE node mapping
  Assignable<XElementSet> elements_;                         ///< Element set
  std::unordered_map<std::size_t, idx_t> gmshToJiveElemMap_; ///< GMSH to JIVE element mapping
  IdxMatrix entities_;                                       ///< Entity matrix
  /// @}

  /// @name Output configuration
  /// @{
  bool verbose_;             ///< Verbose output flag
  bool writeOutput_;         ///< Write output flag
  Ref<Function> sampleCond_; ///< Sampling condition function
  String outFile_;           ///< Output file name
  String outExt_;            ///< Output file extension
  StringVector outTables_;   ///< Output table names
  idx_t nodeView_;           ///< Node view index
  idx_t elemView_;           ///< Element view index
  /// @}
};
