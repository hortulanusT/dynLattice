/**
 * @file PBCGroupOutputModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Wrapper Class for some default PBC Outputs
 * @version 0.1
 * @date 2021-12-06
 * 
 * @copyright Copyright (C) 2021 TU Delft. All rights reserved.
 * 
 */

#include "GroupOutputModule.h"
#include "PBCGroupInputModule.h"

#include <jem/base/Slice.h>
#include <jem/util/ArrayBuffer.h>
#include <jive/app/SampleModule.h>

using jem::util::ArrayBuffer;
using jive::app::SampleModule;

class PBCGroupOutputModule : public GroupOutputModule
{
 public:

  typedef PBCGroupOutputModule   Self;
  typedef GroupOutputModule      Super;

  static const char*             TYPE_NAME;
  static const char*             CHILD_NAME;

  explicit                  PBCGroupOutputModule

    ( const String&           name   = "pbcGroupOutput" );

  virtual Status            init

    ( const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  virtual Status            run 
    ( const Properties&       globdat );

  virtual void              shutdown 
    ( const Properties&       globdat );

  static Ref<Module>        makeNew

    ( const String&           name,
      const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  static void         declare ();  
 
 protected:
  String              getHeader_ () const;

  StringVector        getDataSets_ () const;

 protected:
  Ref<SampleModule>   child_;
};