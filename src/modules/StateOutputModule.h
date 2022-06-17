/**
 * @file StateOutputModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Simple Module to write the state vectors to some Output file
 * @version 0.1
 * @date 2022-06-03
 * 
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 * 
 */

#pragma once

#include <jem/base/CString.h>
#include <jem/base/Array.h>
#include <jem/io/FileWriter.h>
#include <jem/io/GzipFileWriter.h>
#include <jem/io/PrintWriter.h>
#include <jem/util/Properties.h>

#include <jive/util/utilities.h>
#include <jive/util/Assignable.h>
#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/app/Names.h>
#include <jive/fem/NodeSet.h>
#include <jive/femodel/Names.h>
#include <jive/model/StateVector.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/Globdat.h>

using jem::newInstance;
using jem::io::Writer;
using jem::io::FileWriter;
using jem::io::GzipFileWriter;
using jem::io::PrintWriter;
using jem::numeric::Function;

using jive::idx_t;
using jive::Ref;
using jive::String;
using jive::Properties;
using jive::Vector;
using jive::IdxVector;
using jive::BoolVector;
using jive::StringVector;

using jive::util::Assignable;
using jive::app::Module;
using jive::fem::NodeSet;
using jive::util::DofSpace;
using jive::util::FuncUtils;
using jive::model::StateTag;


class StateOutputModule : public Module
{
 public:

  JEM_DECLARE_CLASS       ( StateOutputModule, Module );

  static const char*        TYPE_NAME;
  static const char*        STATE_PROP[3];
  static const char*        NODE_PROP;


  explicit                  StateOutputModule

    ( const String&           name = "stateOutput" );

  virtual Status            init

    ( const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat )      override;

  virtual Status            run

    ( const Properties&       globdat )      override;

  virtual void              shutdown

    ( const Properties&       globdat )      override;


  static Ref<Module>        makeNew

    ( const String&           name,
      const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  static void               declare       ();


 protected:

  virtual                  ~StateOutputModule   ();

 private:

  void                     writeHeader_ 
  
    ( const bool            time = false );

  void                     writeLine_

    ( const Vector&         data,
      const idx_t           step,
      const StateTag        state,
      const double          time = -1. ) const;


 private:  
  Ref<DofSpace>           dofs_;
  IdxVector               dofsOut_;
  BoolVector              stateOut_;

  bool                    started_;
  Ref<PrintWriter>        output_;
  Ref<Function>           sampleCond_; 
};