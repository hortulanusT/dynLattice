/**
 * @file GMSHInputModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @author Leon Riccius (l.riccius@tudelff.nl)
 * @brief A Module to import Geometry using the GMSH C++ API
 * @version 0.1
 * @date 2022-04-01
 *
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 *
 */

#pragma once

#include <gmsh.h>

#include <jem/base/CString.h>
#include <jem/base/Array.h>
#include <jem/base/System.h>
#include <jem/util/Properties.h>
#include <jem/util/ArrayBuffer.h>
#include <jem/util/HashDictionary.h>

#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/app/Names.h>
#include <jive/fem/XNodeSet.h>
#include <jive/fem/XElementSet.h>
#include <jive/fem/ElementGroup.h>
#include <jive/util/Assignable.h>
#include <jive/util/Globdat.h>

using jem::Array;
using jem::max;

using jive::ALL;
using jive::idx_t;
using jive::IdxMatrix;
using jive::IdxVector;
using jive::Matrix;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::Vector;

using jive::app::Module;
using jive::fem::ElementGroup;
using jive::fem::XElementSet;
using jive::fem::XNodeSet;
using jive::util::Assignable;

typedef jem::util::ArrayBuffer<idx_t> IdxBuffer;

static_assert(GMSH_API_VERSION_MAJOR == 4, "Wrong GMSH API version, please use 4.9.5");
static_assert(GMSH_API_VERSION_MINOR == 9, "Wrong GMSH API version, please use 4.9.5");
static_assert(GMSH_API_VERSION_PATCH == 5, "Wrong GMSH API version, please use 4.9.5");

/**
 * @brief Class GMSHInputModule to translate GMSH Geometry to JIVE-Data
 *
 */
class GMSHInputModule : public Module
{
public:
  typedef GMSHInputModule Self;
  typedef Module Super;

  static const char *TYPE_NAME;
  static const char *ORDER;
  static const char *MESH_DIM;
  static const char *SAVE_MSH;
  static const char *STORE_TANGENTS;
  static const char *ENTITY_NAMES[4];
  static const char *ONELAB_PROPS;
  static const char *VERBOSE;

  explicit GMSHInputModule

      (const String &name = "GMSHInput");

  Status init

      (const Properties &conf,
       const Properties &props,
       const Properties &globdat) override;

  Status run

      (const Properties &globdat) override;

  void shutdown

      (const Properties &globdat) override;

  static Ref<Module> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  static void declare();

protected:
  ~GMSHInputModule();

  /**
   * @brief opens a file, reads in the geometry and generates a mesh from it
   *
   * @param geoFile name of the .geo file
   * @param order order of the mesh (1=linear elements, 2=quadratic elements, ...)
   */
  void openMesh_

      (const String &geoFile,
       const idx_t order);

  /**
   * @brief prepares the onllab properties for the mesh, that are given in the settings
   *
   * @param onelabProps Onelab Properties to set
   */
  void prepareOnelab_

      (const Properties &onelabProps);

  /**
   * @brief poplulates the nodes stored in the global databse
   *
   * @param dim dimension of the embedding space (i.e. number of coordinates per node)
   */
  void createNodes_

      (const idx_t dim);

  /**
   * @brief populates the elements stored in the global database
   *
   * @param globdat global database
   * @param offset offset for the numbering, e.g. between the JIVE and the GMSH lists
   */
  void createElems_

      (const Properties &globdat,
       const idx_t offset = 1);

  void storeTangents_

      (const Properties &globdat,
       const idx_t offset = 1);

private:
  Assignable<XNodeSet> nodes_;
  IdxVector nodeIDs_;
  Assignable<XElementSet> elements_;
  IdxMatrix entities_;
  bool verbose_;
};
