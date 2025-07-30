/**
 * @file ParaViewModule.h
 * @author Til Gärtner
 * @brief Module for exporting simulation data to ParaView format
 *
 * This module provides functionality to export mesh data, displacement fields,
 * and other simulation results to VTK/VTU format files that can be visualized
 * in ParaView. It supports multiple element groups, time series output, and
 * flexible data selection for post-processing visualization.
 *
 * @note The current implementation is only tested for line elements, other elements are implemented to the best of our knowledge.
 */

#pragma once

#include <jem/base/Array.h>
#include <jem/base/CString.h>
#include <jem/base/IllegalArgumentException.h>
#include <jem/base/Slice.h>
#include <jem/base/System.h>
#include <jem/io/FileName.h>
#include <jem/io/FileOpenException.h>
#include <jem/io/FileWriter.h>
#include <jem/io/PrintWriter.h>
#include <jem/util/ArrayBuffer.h>
#include <jem/util/Properties.h>

#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/app/Names.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/fem/NodeSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/Globdat.h>
#include <jive/util/ItemGroup.h>
#include <jive/util/ItemSet.h>
#include <jive/util/SparseTable.h>
#include <jive/util/XTable.h>

#include <filesystem>

using jem::ALL;
using jem::Array;
using jem::idx_t;
using jem::IllegalArgumentException;
using jem::Limits;
using jem::newInstance;
using jem::Ref;
using jem::SliceFrom;
using jem::SliceFromTo;
using jem::SliceTo;
using jem::String;
using jem::sum;
using jem::where;
using jem::io::endl;
using jem::io::FileFlags;
using jem::io::FileName;
using jem::io::FileOpenException;
using jem::io::FileWriter;
using jem::io::PrintWriter;
using jem::io::Writer;
using jem::util::ArrayBuffer;
using jem::util::Properties;

using jive::IdxVector;
using jive::Matrix;
using jive::StringVector;
using jive::Vector;
using jive::app::Module;
using jive::app::PropNames;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::fem::NodeSet;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::Function;
using jive::util::Globdat;
using jive::util::ItemGroup;
using jive::util::ItemSet;
using jive::util::SparseTable;
using jive::util::XTable;

/// @brief Module for exporting simulation data to ParaView format
/// @details This module exports finite element simulation results to VTK/VTU
/// format files for visualization in ParaView. Key features include:
///
/// - **Multi-group support**: Export different element groups with specific data sets
/// - **Time series output**: Generate time-dependent visualization files with PVD collection
/// - **Flexible data export**: Support for displacements, velocities, accelerations, and custom DOF data
/// - **Element compatibility**: Handles various element shapes with proper VTK cell type mapping
/// - **Node ordering**: Automatic conversion from GMSH to ParaView node ordering
/// - **Conditional sampling**: Configurable output frequency based on time or iteration criteria
///
/// The module supports both individual VTU files and ParaView collection (PVD) files
/// for time series visualization. Element and node data can be selectively exported
/// based on configuration, making it suitable for large-scale simulations where
/// storage efficiency is important.
///
/// @see [ParaView](https://www.paraview.org/)
/// @see [VTK file formats](https://vtk.org/wp-content/uploads/2015/04/file-formats.pdf)
class ParaViewModule : public Module
{
protected:
  /// @brief Structure containing element group export information
  /// @details Stores configuration for each element group to be exported,
  /// including shape information and data field specifications
  struct ElementInfo
  {
    String name;           ///< Name of the element group
    String shape;          ///< Element shape identifier
    StringVector elemData; ///< Element-based data fields to export
    StringVector nodeData; ///< Node-based data fields to export
    StringVector dispData; ///< Displacement component names
    StringVector dofData;  ///< Additional DOF data to export
  };

public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  static const char *SPACING;   ///< XML indentation spacing
  /// @}

  /// @brief Constructor
  /// @param name Module name (default: "paraView")
  explicit ParaViewModule(const String &name = "paraView");

  /// @brief Initialize the module
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  /// @details Configures output format, file type, element groups, data fields,
  /// and sampling conditions for ParaView export
  virtual Status init(const Properties &conf, const Properties &props,
                      const Properties &globdat) override;

  /// @brief Run the ParaView export
  /// @param globdat Global data container
  /// @return Module status
  /// @details Exports current simulation state to VTU file if sampling
  /// conditions are met, and updates PVD collection if enabled
  virtual Status run(const Properties &globdat) override;

  /// @brief Shutdown and finalize ParaView export
  /// @param globdat Global data container
  /// @details Writes final PVD collection file for time series visualization
  virtual void shutdown(const Properties &globdat) override;

  /// @brief Factory method for creating new ParaViewModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new ParaViewModule instance
  static Ref<Module> makeNew(const String &name, const Properties &conf,
                             const Properties &props, const Properties &globdat);

  /// @brief Register ParaViewModule type with ModuleFactory
  static void declare();

  /// @brief Convert element shape name to VTK cell type number
  /// @param name Element shape name (e.g., "Triangle", "Tetrahedron")
  /// @return VTK cell type identifier
  /// @details Maps JIVE element shape names to corresponding VTK cell type
  /// numbers for proper visualization in ParaView
  static idx_t nameToVTKNum(const String &name);

  /// @brief Convert GMSH node ordering to ParaView node ordering
  /// @param elNodes Vector with JIVE node indices
  /// @param name Element shape name
  /// @return Correctly ordered node vector for ParaView
  /// @details Different finite element codes use different node ordering
  /// conventions. This function converts from GMSH ordering to ParaView ordering
  static IdxVector gmsh2ParaNodeOrder(const IdxVector elNodes, const String &name);

private:
  /// @name File output methods
  /// @{

  /// @brief Write VTU file with simulation data
  /// @param fileName Output file name
  /// @param globdat Global data container
  /// @details Main file writing function that coordinates the export of all
  /// configured element groups and their associated data to a VTU file
  void writeFile_(const String &fileName, const Properties &globdat);

  /// @brief Write mesh piece for specific element group
  /// @param file Output file writer
  /// @param points Node set containing point information
  /// @param cells Element set containing cell information
  /// @param group Element group to export
  /// @param disp Displacement state vector
  /// @param velo Velocity state vector
  /// @param acce Acceleration state vector
  /// @param dofs DOF space with degree of freedom information
  /// @param model Model reference for data extraction
  /// @param globdat Global data container
  /// @param info Element group configuration information
  /// @details Writes the mesh geometry and associated data for a single element
  /// group in VTK unstructured grid format
  void writePiece_(const Ref<PrintWriter> &file, const Assignable<NodeSet> &points,
                   const Assignable<ElementSet> &cells,
                   const Assignable<ElementGroup> &group, const Vector &disp,
                   const Vector &velo, const Vector &acce, const Ref<DofSpace> &dofs,
                   const Ref<Model> &model, const Properties &globdat,
                   const ElementInfo &info);

  /// @brief Write data array from matrix
  /// @param file Output file writer
  /// @param data Matrix containing data values
  /// @param type VTK data type string
  /// @param name Data array name
  /// @details Writes matrix data in VTK DataArray format with proper formatting
  void writeDataArray_(const Ref<PrintWriter> &file, const Matrix &data,
                       const String &type, const String &name);

  /// @brief Write data array from XTable
  /// @param file Output file writer
  /// @param data XTable containing data values
  /// @param rows Row indices to extract
  /// @param type VTK data type string
  /// @param name Data array name
  /// @details Writes XTable data for specified rows in VTK DataArray format
  void writeDataArray_(const Ref<PrintWriter> &file, const Ref<XTable> &data,
                       const IdxVector &rows, const String &type, const String &name);

  /// @brief Write data array from vector
  /// @param file Output file writer
  /// @param data Vector containing data values
  /// @param type VTK data type string
  /// @param name Data array name
  /// @details Writes vector data in VTK DataArray format
  void writeDataArray_(const Ref<PrintWriter> &file, const Vector &data,
                       const String &type, const String &name);
  /// @}

private:
  /// @name Configuration parameters
  /// @{
  String nameFormat_;          ///< Output file name format string
  String fileType_;            ///< Output file type (e.g., "vtu")
  StringVector elemSets_;      ///< Names of element groups to export
  Array<ElementInfo> setInfo_; ///< Configuration for each element group
  Ref<Function> sampleCond_;   ///< Condition function for output sampling
  Ref<Function> sampleInfo_;   ///< Information function for sampling
  /// @}

  /// @name Output tracking
  /// @{
  idx_t out_num_;                       ///< Current output file number
  bool pvd_print_;                      ///< Flag for PVD collection file generation
  String pvd_name_;                     ///< Name of PVD collection file
  ArrayBuffer<double> pvd_time_buffer_; ///< Time values for PVD collection
  ArrayBuffer<String> pvd_name_buffer_; ///< File names for PVD collection
  /// @}
};
