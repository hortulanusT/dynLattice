/**
 * @file MassModel.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief adding mass to node groups
 * @version 0.1
 * @date 2024-08-29
 *
 * @copyright Copyright (C) 2024 TU Delft. All rights reserved.
 *
 */

#pragma once

#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/fem/NodeGroup.h>
#include <jive/fem/NodeSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>

using jem::Array;
using jive::idx_t;
using jive::IdxVector;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::StringVector;
using jive::Vector;
using jive::algebra::MatrixBuilder;
using jive::fem::DofSpace;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::util::Assignable;

class MassModel : public Model
{
public:
  JEM_DECLARE_CLASS(MassModel, Model);

  static const char *TYPE_NAME;
  static const char *TOTAL_MASS_PROP;
  static const char *NODE_GROUPS_PROP;
  static const char *DOFS_PROP;

  explicit MassModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat) override;

  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  static void declare();

protected:
  virtual ~MassModel();

private:
  Vector totalMass_;
  StringVector nodeGroupNames_;
  StringVector dofNames_;

  Ref<DofSpace> dofs_;

  IdxVector dofTypes_;
  Array<Assignable<NodeGroup>> nodeGroups_;
};
