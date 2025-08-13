/**
 * @file PBCGroupOutputModule.h
 * @author Til Gärtner
 * @brief Specialized output module for periodic boundary condition analysis
 *
 * Extends GroupOutputModule to provide automatic strain and stress output
 * for periodic boundary condition simulations with predefined PBC-specific
 * node groups and data extraction.
 */
#pragma once

#include "GroupOutputModule.h"
#include "PBCGroupInputModule.h"

#include <jem/base/Slice.h>
#include <jem/util/ArrayBuffer.h>
#include <jive/app/Names.h>
#include <jive/app/SampleModule.h>

using jem::SliceTo;
using jem::util::ArrayBuffer;
using jive::app::PropNames;
using jive::app::SampleModule;

/// @brief Specialized output module for periodic boundary condition analysis
/// @details Extends GroupOutputModule with automatic configuration for PBC
/// simulations. Provides strain and stress extraction from boundary node groups
/// and calculates displacement gradients and stress tensors for homogenization.
class PBCGroupOutputModule : public GroupOutputModule
{
public:
  /// @name Type definitions
  /// @{
  typedef PBCGroupOutputModule Self; ///< Self type alias
  typedef GroupOutputModule Super;   ///< Base class type alias
  /// @}

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;  ///< Module type name
  static const char *CHILD_NAME; ///< Child sampling module name
  /// @}

  /// @brief Constructor
  /// @param name Module name (default: "pbcGroupOutput")
  explicit PBCGroupOutputModule(const String &name = "pbcGroupOutput");

  /// @brief Initialize PBC output module
  /// @param conf Configuration properties (output)
  /// @param props User properties
  /// @param globdat Global data container
  /// @return Module status
  virtual Status init(const Properties &conf, const Properties &props,
                      const Properties &globdat);

  /// @brief Run PBC output extraction
  /// @param globdat Global data container
  /// @return Module status
  virtual Status run(const Properties &globdat);

  /// @brief Shutdown and finalize output
  /// @param globdat Global data container
  virtual void shutdown(const Properties &globdat);

  /// @brief Factory method for creating new instances
  /// @param name Module name
  /// @param conf Configuration properties (output)
  /// @param props User properties
  /// @param globdat Global data container
  /// @return New module instance
  static Ref<Module> makeNew(const String &name, const Properties &conf,
                             const Properties &props, const Properties &globdat);

  /// @brief Register module type with factory
  static void declare();

  /// @brief Generate strain/stress data set expressions
  /// @param dim Spatial dimension
  /// @param strains Include strain measures
  /// @param stresses Include stress measures
  /// @param dofNames DOF names for coordinate directions
  /// @return Vector of data set expressions
  static StringVector getDataSets(const idx_t dim,
                                  const bool strains = false,
                                  const bool stresses = false,
                                  const StringVector &dofNames = {
                                      "dx", "dy", "dz"});

protected:
  /// @brief Generate CSV header string
  /// @param existing Existing header content
  /// @return Complete header with PBC-specific columns
  String getHeader_(String existing) const;

  /// @brief Generate data set expressions
  /// @param existing Existing data sets
  /// @return Complete data sets with PBC-specific expressions
  StringVector getDataSets_(StringVector existing) const;

protected:
  /// @name Child modules
  /// @{
  Ref<SampleModule> child_; ///< Child sampling module for data output
  /// @}
};
