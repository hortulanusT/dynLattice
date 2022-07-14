/**
 * @file CSVOutputModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Module to enable the Standard Output Module to Output CSV Files
 * @version 0.1
 * @date 2022-06-22
 * 
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 * 
 */
#pragma once

#include <jive/app/ModuleFactory.h>
#include <jive/app/OutputModule.h>
#include <jive/util/utilities.h>

using jem::newInstance;

using jive::Ref;
using jive::String;
using jive::Properties;
using jive::app::Module;
using jive::app::OutputModule;
using jive::app::DataPrinter;

class CSVOutputModule : public OutputModule
{
 public:

  JEM_DECLARE_CLASS       ( CSVOutputModule, OutputModule );

  static const char*        TYPE_NAME;

  explicit                  CSVOutputModule

    ( const String&               name = "csvOutput",
      const Ref<DataPrinter>      printer = nullptr  );

  static Ref<Module>        makeNew

    ( const String&           name,
      const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  static void               declare       ();


 protected:

  virtual                  ~CSVOutputModule   ();
};