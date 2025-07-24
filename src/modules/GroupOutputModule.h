/**
 * @file GroupOutputModule.h
 * @author Til Gärtner
 * @brief module that writes information into globdat
 *
 *
 */

#pragma once

#include <jem/base/Array.h>
#include <jem/util/Properties.h>

#include <jive/Array.h>
#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/app/Names.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/NodeGroup.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/StateVector.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Globdat.h>

#include "utils/helpers.h"
#include "utils/testing.h"

using namespace jive_helpers;

using jem::idx_t;
using jem::newInstance;
using jem::Ref;
using jem::String;
using jem::sum;
using jem::util::Properties;

using jive::IdxVector;
using jive::Matrix;
using jive::StringVector;
using jive::Vector;
using jive::app::Module;
using jive::app::PropNames;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::DofSpace;
using jive::util::Globdat;

/**
 * This module adds certain values to the globdat database
 * and updates them after each and every timestep.
 * @brief module to add desired values to the globdat
 *
 */
class GroupOutputModule : public Module
{
public:
  static const char *TYPE_NAME;

  /**
   * @brief Construct a new Load Extent Module object
   *
   * @param name name of the module
   */
  explicit GroupOutputModule(const String &name = "groupOutput");

  /**
   * @brief initalizes the module with the given values
   *
   * @param conf configuration properties, that will be reported
   * @param props configuration properteis, that will be applied
   * @param globdat global database
   * @returns status code
   */
  virtual Status init(const Properties &conf, const Properties &props,
                      const Properties &globdat);

  virtual Status run(const Properties &globdat);

  static Ref<Module> makeNew(const String &name, const Properties &conf,
                             const Properties &props,
                             const Properties &globdat);

  static void declare();

protected:
  StringVector nodeGroups_;   ///< groups of nodes used to calculate the
                              ///< singular element forces/displacements
  StringVector elemGroups_;   ///< groups of elements used to calculate the
                              ///< extent/strain/stress values
  StringVector nodeDofNames_; ///< vector containing the names of the dofs
                              ///< used for singular calculations
  IdxVector nodeDofs_;        ///< vector containing the ids of the dofs used for
                              ///< the singular caluclations
  StringVector elemDofNames_; ///< vector containing the names of the dofs
                              ///< used for aggregate calculations
  IdxVector elemDofs_;        ///< vector containing the ids of the dofs used for
                              ///< the aggregate calculations
};
