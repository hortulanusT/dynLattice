/**
 * @file CSVOutputModule.h
 * @author Til Gärtner
 * @brief CSV output module for data export in comma-separated values format
 */

#pragma once

#include <jem/base/Class.h>
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

/// @brief Module for outputting data in CSV format
/// @details Extends the standard OutputModule to enable CSV file output using
/// a CSVDataPrinter. Provides comma-separated values format for data analysis
/// and post-processing in external tools like Excel, MATLAB, or Python.
/// @see CSVDataPrinter
class CSVOutputModule : public OutputModule
{
public:
  JEM_DECLARE_CLASS(CSVOutputModule, OutputModule);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  /// @}

  /// @brief Constructor
  /// @param name Module name
  /// @param printer Data printer for CSV output (will be always a new CSVDataPrinter Instance)
  explicit CSVOutputModule(const String &name = "csvOutput",
                           const Ref<DataPrinter> printer = nullptr);

  /// @brief Factory method for creating new CSVOutputModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new CSVOutputModule instance
  static Ref<Module> makeNew(const String &name,
                             const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  /// @brief Register CSVOutputModule type with ModuleFactory
  static void declare();

protected:
  /// @brief Protected destructor
  virtual ~CSVOutputModule();
};
