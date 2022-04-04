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

#include <jem/util/Properties.h>

#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>

using jive::Properties;
using jive::String;
using jive::Ref;

using jive::app::Module;

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
};
