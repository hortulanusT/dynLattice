/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *
 * This class implements (plane) symmetry BC Model
 *
 * Author: T. Gaertner
 * Date: Feburary 2025
 *
 */
#pragma once

#include <jem/util/ArrayBuffer.h>
#include <jem/util/Properties.h>

#include <jive/Array.h>
#include <jive/fem/NodeGroup.h>
#include <jive/fem/NodeSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/Names.h>
#include <jive/util/Assignable.h>
#include <jive/util/Constraints.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Globdat.h>

#include "modules/PBCGroupInputModule.h"
#include "utils/helpers.h"
#include "utils/testing.h"

using jem::util::Properties;
using jive::BoolMatrix;
using jive::IdxMatrix;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::DofSpace;
using jive::util::Globdat;

typedef jem::util::ArrayBuffer<idx_t> IdxBuffer;

class SymBCModel : public Model
{
public:
  static const char *TYPE_NAME;
  static const char *DOF_OPPO_NAMES_PROP;
  static const char *DOF_EQUAL_NAMES_PROP;
  static const char *SURFACES_PROP;

  explicit SymBCModel

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  virtual bool takeAction

      (const String &action, const Properties &params,
       const Properties &globdat);

  static Ref<Model> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

private:
  void init_

      (const Properties &globdat);

  void setConstraints_();

private:
  Assignable<NodeSet> nodes_;
  Ref<DofSpace> dofs_;
  Ref<Constraints> cons_;
  StringVector dofOppoNames_;
  StringVector dofEqualNames_;
  StringVector surfaceNames_;
  IdxVector mainOppoDofs_;
  IdxVector secondaryOppoDofs_;
  IdxVector mainEqualDofs_;
  IdxVector secondaryEqualDofs_;
  idx_t pbcRank_;
};
