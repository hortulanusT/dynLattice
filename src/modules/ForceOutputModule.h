/**
 * @file ForceOutputModule.h
 * @author Til Gärtner
 * @brief Module for writing force vectors to output files
 */

#pragma once

#include <jem/base/Array.h>
#include <jem/base/CString.h>
#include <jem/base/Class.h>
#include <jem/io/FileWriter.h>
#include <jem/io/GzipFileWriter.h>
#include <jem/io/PrintWriter.h>
#include <jem/util/Properties.h>

#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/app/Names.h>
#include <jive/fem/NodeSet.h>
#include <jive/femodel/Names.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/Globdat.h>
#include <jive/util/utilities.h>

using jem::newInstance;
using jem::io::FileWriter;
using jem::io::GzipFileWriter;
using jem::io::PrintWriter;
using jem::io::Writer;
using jem::numeric::Function;

using jive::BoolVector;
using jive::idx_t;
using jive::IdxVector;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::StringVector;
using jive::Vector;

using jive::app::Module;
using jive::app::PropNames;
using jive::fem::NodeSet;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::FuncUtils;

/// @brief Module for outputting internal, external, and gyroscopic forces
/// @details Writes force vectors to output files during simulation for post-processing
/// and analysis. Supports selective output of internal forces, external forces,
/// and gyroscopic forces with configurable sampling conditions.
class ForceOutputModule : public Module
{
public:
  JEM_DECLARE_CLASS(ForceOutputModule, Module);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  static const char *INT_PROP;  ///< Internal force output property
  static const char *EXT_PROP;  ///< External force output property
  static const char *GYRO_PROP; ///< Gyroscopic force output property
  static const char *NODE_PROP; ///< Node set property
  /// @}

  /// @brief Constructor
  /// @param name Module name
  explicit ForceOutputModule(const String &name = "ForceOutput");

  /// @brief Initialize the module
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Module status
  virtual Status init(const Properties &conf,
                      const Properties &props,
                      const Properties &globdat) override;

  /// @brief Run the force output operation
  /// @param globdat Global data container
  /// @return Module status
  virtual Status run(const Properties &globdat) override;

  /// @brief Shutdown the module and close output files
  /// @param globdat Global data container
  virtual void shutdown(const Properties &globdat) override;

  /// @brief Factory method for creating new ForceOutputModule instances
  /// @param name Module name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified module properties
  /// @param globdat Global data container
  /// @return Reference to new ForceOutputModule instance
  static Ref<Module> makeNew(const String &name,
                             const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  /// @brief Register ForceOutputModule type with ModuleFactory
  static void declare();

protected:
  /// @brief Protected destructor
  virtual ~ForceOutputModule();

private:
  /// @brief Write file header with column names
  /// @param time Include time column flag
  void writeHeader_(const bool time = false);

  /// @brief Write data line to output file
  /// @param data Force data vector
  /// @param step Time step number
  /// @param type Force type identifier
  /// @param time Current simulation time
  void writeLine_(const Vector &data,
                  const idx_t step,
                  const String type,
                  const double time = -1.) const;

private:
  /// @name System components
  /// @{
  Ref<DofSpace> dofs_; ///< Degree of freedom space
  Ref<Model> model_;   ///< Root of the model tree
  /// @}

  /// @name Output configuration
  /// @{
  IdxVector dofsOut_; ///< DOF indices for output
  bool intOut_;       ///< Output internal forces flag
  bool extOut_;       ///< Output external forces flag
  bool gyroOut_;      ///< Output gyroscopic forces flag
  /// @}

  /// @name Output management
  /// @{
  bool started_;             ///< Output started flag
  Ref<PrintWriter> output_;  ///< Output writer
  Ref<Function> sampleCond_; ///< Sampling condition function
  /// @}
};
