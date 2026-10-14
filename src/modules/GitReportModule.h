/**
 * @file GitReportModule.h
 * @author Til Gärtner
 * @brief Module for reporting Git version information to log files
 */

#pragma once

#ifndef GIT_HASH
#error "Please provide the githash using -DGIT_HASH!"
#endif

#include <jem/base/Class.h>
#include <jem/base/System.h>
#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>

using jive::Properties;
using jive::Ref;
using jive::String;
using jive::app::Module;
using jive::app::ModuleFactory;

/// @brief Module for reporting Git version information at simulation start
/// @details Prints Git hash and repository status to log files for reproducibility
/// tracking. Requires GIT_HASH to be defined during compilation.
class GitReportModule : public Module
{
 public:
  JEM_DECLARE_CLASS(GitReportModule, Module);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  /// @}

  /// @brief Constructor
  /// @param[in] name  Module name
  explicit GitReportModule(const String &name = "GitReport");

  /// @brief Initialize the module
  /// @param[out] conf  Actually used configuration properties (output)
  /// @param[in] props  User-specified module properties
  /// @param[in] globdat  Global data container
  /// @return Module status
  virtual Status init(const Properties &conf,
                      const Properties &props,
                      const Properties &globdat) override;

  /// @brief Run Git reporting
  /// @param[in] globdat  Global data container
  /// @return Module status
  virtual Status run(const Properties &globdat) override;

  /// @brief Shutdown the module
  /// @param[in] globdat  Global data container
  virtual void shutdown(const Properties &globdat) override;

  /// @brief Factory method for creating new GitReportModule instances
  /// @param[in] name  Module name
  /// @param[out] conf  Actually used configuration properties (output)
  /// @param[in] props  User-specified module properties
  /// @param[in] globdat  Global data container
  /// @return Reference to new GitReportModule instance
  static Ref<Module> makeNew(const String &name,
                             const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  /// @brief Register GitReportModule type with ModuleFactory
  static void declare();

 protected:
  /// @brief Protected destructor
  virtual ~GitReportModule();
};
