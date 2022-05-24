/**
 * @file ParaViewModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Implements the export of mesh data to ParaView
 * @version 0.1
 * @date 2021-10-25
 * 
 * @copyright Copyright (C) 2021 TU Delft. All rights reserved.
 * 
 */

#pragma once

#include <jem/base/Array.h>
#include <jem/base/System.h>
#include <jem/base/Slice.h>
#include <jem/base/IllegalArgumentException.h>
#include <jem/base/CString.h>
#include <jem/util/Properties.h>
#include <jem/io/FileWriter.h>
#include <jem/io/PrintWriter.h>
#include <jem/io/FileName.h>
#include <jem/io/FileOpenException.h>

#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/app/Names.h>
#include <jive/util/XTable.h>
#include <jive/util/SparseTable.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/ItemSet.h>
#include <jive/util/ItemGroup.h>
#include <jive/util/Globdat.h>
#include <jive/util/FuncUtils.h>
#include <jive/fem/NodeSet.h>
#include <jive/fem/ElementSet.h>
#include <jive/fem/ElementGroup.h>
#include <jive/model/Model.h>
#include <jive/model/StateVector.h>
#include <jive/model/Actions.h>

#include <filesystem>

#include "utils/testing.h"
#include "utils/helpers.h"

using jem::Ref;
using jem::newInstance;
using jem::where;
using jem::Limits;
using jem::String;
using jem::SliceTo;
using jem::SliceFrom;
using jem::IllegalArgumentException;
using jem::util::Properties;
using jem::io::endl;
using jem::io::Writer;
using jem::io::FileWriter;
using jem::io::PrintWriter;
using jem::io::FileName;
using jem::io::FileOpenException;

using jive::StringVector;
using jive::app::Module;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::XTable;
using jive::util::SparseTable;
using jive::util::ItemSet;
using jive::util::ItemGroup;
using jive::util::Globdat;
using jive::util::Function;
using jive::fem::NodeSet;
using jive::fem::ElementSet;
using jive::fem::ElementGroup;
using jive::model::Model;
using jive::model::StateVector;
using jive::model::Actions;

class ParaViewModule : public Module
{
 protected:
  struct elInfo
  {
    String        name;
    String        shape;
    StringVector  elemData;
    StringVector  nodeData;
    StringVector  dispData;
    StringVector  dofData;
  };

 public:

  static const char*         TYPE_NAME;
  static const char*         SPACING;

  /**
   * @brief Construct a new Para View Module object
   * 
   * @param name name of the module
   */
  explicit                   ParaViewModule
    ( const String&            name = "paraView");

  virtual Status             init

    ( const Properties&        conf,
      const Properties&        props,
      const Properties&        globdat      ) override;

  virtual Status             run

    ( const Properties&  globdat            ) override;

  virtual void                shutdown

    ( const Properties&       globdat       ) override;

  static Ref<Module>         makeNew

    ( const String&      name,
      const Properties&  conf,
      const Properties&  props,
      const Properties&  globdat            );

  static void                declare       ();

  /**
   * @brief converts a Shape-Name to a VTK compatible number
   * 
   * @param name Shape-Name
   * @return VTK Cell Number
   */
  static idx_t               nameToVTKNum

    ( const String&      name               );

  /**
   * @brief converts a gmsh Node Order to a ParaView Node order
   * 
   * @param elNodes Vector with the jive indizes
   * @param name Shape-Name
   * @return correctly ordered Vector for ParaView Purposes
   */
  static IdxVector            gmsh2ParaNodeOrder

    ( const IdxVector    elNodes,
      const String&      name               );

 private:
  /**
   * @brief writes data to paraView compatible File
   * 
   * @param fileName Name of the file to write to
   * @param globdat Global Property file
   */
  void                      writeFile_

    ( const String&                 fileName,
      const Properties&             globdat );

  /**
   * @brief writes mesh info to file
   * 
   * @param file PrintWriter for the correspoding file
   * @param points NodeSet with all the information regarding the points
   * @param cells ElemenSet with all the information regarding the elements 
   * @param group ElemenGroup from which the information should be extracted
   * @param disp StateVector 0
   * @param velo StateVector 1
   * @param acce StateVector 2
   * @param dofs DofSpace with all available information
   * @param model root for the modeltree
   * @param globdat Global Properties Database
   * @param info the info of the group 
   */
  void                      writePiece_

  ( const Ref<PrintWriter>&         file,
    const Assignable<NodeSet>&      points,
    const Assignable<ElementSet>&   cells,
    const Assignable<ElementGroup>& group,
    const Vector&                   disp,
    const Vector&                   velo,
    const Vector&                   acce,
    const Ref<DofSpace>&            dofs,
    const Ref<Model>&               model,
    const Properties&               globdat,
    const elInfo&                   info   );

  /**
   * @brief writes given data to file
   * 
   * @param data matrix containing all the data
   * @param type number format of the data 
   * @param name name of the data
   */
  void                      writeDataArray_

  ( const Ref<PrintWriter>&       file,
    const Matrix&                 data,
    const String&                 type,
    const String&                 name   );

  void                      writeDataArray_

  ( const Ref<PrintWriter>&       file,
    const Ref<XTable>&            data,
    const IdxVector&              rows,
    const String&                 type,
    const String&                 name   );

  void                      writeDataArray_

  ( const Ref<PrintWriter>&       file,
    const Vector&                 data,
    const String&                 type,
    const String&                 name   );

 private:
  String            nameFormat_;
  String            fileType_;
  StringVector      elemSets_;
  Array<elInfo>     setInfo_;
  Ref<Function>     sampleCond_;
  Ref<PrintWriter>  pvd_printer_;
};