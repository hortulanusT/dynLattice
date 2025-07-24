/**
 * @file PBCGroupOutputModule.h
 * @author Til Gärtner
 * @brief Wrapper Class for some default PBC Outputs
 *
 *
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

class PBCGroupOutputModule : public GroupOutputModule
{
public:
  typedef PBCGroupOutputModule Self;
  typedef GroupOutputModule Super;

  static const char *TYPE_NAME;
  static const char *CHILD_NAME;

  explicit PBCGroupOutputModule

      (const String &name = "pbcGroupOutput");

  virtual Status init

      (const Properties &conf, const Properties &props,
       const Properties &globdat);

  virtual Status run(const Properties &globdat);

  virtual void shutdown(const Properties &globdat);

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

  /**
   * @brief get the strings describing different strain and stress
   * measures
   *
   * @param dim dimension for which to get the strain and stress measures
   * @param strains whether to return strain measures
   * @param stresses whether to return stress measures
   * @param dofNames names of the dofs to use
   * @returns Vector with strings for the expressions
   */
  static StringVector getDataSets(const idx_t dim,
                                  const bool strains = false,
                                  const bool stresses = false,
                                  const StringVector &dofNames = {
                                      "dx", "dy", "dz"});

protected:
  String getHeader_(String existing) const;

  StringVector getDataSets_(StringVector existing) const;

protected:
  Ref<SampleModule> child_;
};
