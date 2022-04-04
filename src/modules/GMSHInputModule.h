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
#include <jem/util/Properties.h>
#include <jem/util/ArrayBuffer.h>

#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/fem/XNodeSet.h>
#include <jive/fem/XElementSet.h>
#include <jive/fem/ElementGroup.h>
#include <jive/util/Assignable.h>

using jive::idx_t;
using jive::Properties;
using jive::String;
using jive::Ref;
using jive::Vector;
using jive::IdxVector;

using jive::app::Module;
using jive::fem::XNodeSet;
using jive::fem::XElementSet;
using jive::fem::ElementGroup;
using jive::util::Assignable;

typedef jem::util::ArrayBuffer<idx_t> IdxBuffer;

/**
 * @brief Class GMSHInputModule to translate GMSH Geometry to JIVE-Data
 * 
 */
class GMSHInputModule : public Module
{
 public:

  typedef                 GMSHInputModule   Self;
  typedef                 Module            Super;
  
  static const char*      TYPE_NAME;
  static const char*      GEO_FILE;
  static const char*      ORDER;


  explicit                  GMSHInputModule

    ( const String&           name = "GMSHInput" );

  Status                    init

    ( const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat )      override;

  Status                    run

    ( const Properties&       globdat )      override;

  void                      shutdown

    ( const Properties&       globdat )      override;


  static Ref<Module>        makeNew

    ( const String&           name,
      const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  static void               declare       ();


 protected:

                            ~GMSHInputModule   ();

  void                      createMesh_ 
  
    ( const idx_t             order,
      const Properties&       globdat ) const;
};
