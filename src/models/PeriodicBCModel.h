/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *
 * This class implements a periodic BC Model
 *
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: September 21
 *
 */
#pragma once

#include <cmath>

#include <jem/util/Properties.h>
#include <jem/util/PropertyException.h>

#include <jive/Array.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/Actions.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Constraints.h>
#include <jive/util/Assignable.h>
#include <jive/util/Globdat.h>
#include <jive/fem/NodeSet.h>
#include <jive/fem/NodeGroup.h>
#include <jive/implict/ArclenActions.h>

#include "utils/testing.h"
#include "utils/helpers.h"
#include "modules/PBCGroupInputModule.h"

using jem::util::Properties;
using jive::BoolMatrix;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::implict::ArclenActions;
using jive::implict::ArclenParams;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::DofSpace;
using jive::util::Globdat;

using namespace jive_helpers;

class periodicBCModel : public Model
{
public:
  static const char *TYPE_NAME;
  static const char *MODE_PROP;
  static const char *GRAD_PROP;
  static const char *DOF_NAMES_PROP;
  static const char *ROT_NAMES_PROP;

  enum Mode
  {
    LOAD,
    DISP
  };

  explicit periodicBCModel

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

  void setConstraints_

      (const Properties &globdat,
       const double scale);

  void getExtVec_

      (const Vector &f,
       const Properties &globdat,
       const double scale = 1.);

private:
  Assignable<NodeSet> nodes_;
  Ref<DofSpace> dofs_;
  Ref<Constraints> cons_;
  String gradName_;
  Matrix grad_;
  StringVector dofNames_;
  StringVector rotNames_;
  IdxVector jdofs_;
  IdxVectorMatrix masterDofs_;
  IdxVectorMatrix slaveDofs_;
  idx_t pbcRank_;
  Mode mode_;
};