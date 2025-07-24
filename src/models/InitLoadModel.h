/**
 * @file InitLoadModel.h
 * @author Til Gärtner
 * @brief InitLoadModel for initial loads
 *
 *
 */

#pragma once

#include <jem/util/Properties.h>

#include <jive/fem/NodeGroup.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>

using jem::idx_t;
using jem::newInstance;
using jive::IdxVector;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::StringVector;
using jive::Vector;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::Assignable;
using jive::util::DofSpace;

class InitLoadModel : public Model
{
public:
  static const char *TYPE_NAME;
  static const char *DISP_GROUPS;
  static const char *DISP_DOFS;
  static const char *DISP_VALS;
  static const char *VELO_GROUPS;
  static const char *VELO_DOFS;
  static const char *VELO_VALS;

  explicit InitLoadModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat);

  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  static void declare();

private:
  void init_

      (const Properties &globdat);

  StringVector dgroups_;
  StringVector ddofs_;
  Vector dvals_;
  StringVector vgroups_;
  StringVector vdofs_;
  Vector vvals_;
};
