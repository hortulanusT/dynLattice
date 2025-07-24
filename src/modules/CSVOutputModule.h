/**
 * @file CSVOutputModule.h
 * @author Til Gärtner
 * @brief Module to enable the Standard Output Module to Output CSV Files
 *
 *
 */
#pragma once

#include <jive/app/ModuleFactory.h>
#include <jive/app/OutputModule.h>
#include <jive/util/utilities.h>

using jem::newInstance;

using jive::Properties;
using jive::Ref;
using jive::String;
using jive::app::DataPrinter;
using jive::app::Module;
using jive::app::OutputModule;

class CSVOutputModule : public OutputModule
{
public:
  JEM_DECLARE_CLASS(CSVOutputModule, OutputModule);

  static const char *TYPE_NAME;

  explicit CSVOutputModule

      (const String &name = "csvOutput",
       const Ref<DataPrinter> printer = nullptr);

  static Ref<Module> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  static void declare();

protected:
  virtual ~CSVOutputModule();
};
