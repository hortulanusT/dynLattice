/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *  
 * This class implements a way to make a certain of of
 * Node-groups equal
 * 
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: July 21
 *
 */
#pragma once

#include <jem/util/Properties.h>

#include <jive/Array.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/Actions.h>
#include <jive/model/StateVector.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Assignable.h>
#include <jive/fem/NodeSet.h>
#include <jive/fem/NodeGroup.h>

#include "utils/testing.h"
#include "utils/helpers.h"

using jem::String;
using jem::Ref;
using jem::newInstance;
using jem::System;
using jem::util::Properties;

using jive::StringVector;
using jive::model::Model;
using jive::model::Actions;
using jive::model::ActionParams;
using jive::model::StateVector;
using jive::util::DofSpace;
using jive::util::Assignable;
using jive::fem::NodeSet;
using jive::fem::NodeGroup;

using namespace jive_helpers;

class FollowerLoadModel : public Model
{
 public:
  static const char*      TYPE_NAME;
  static const char*      NODES_PROP;
  static const char*      START_PROP;
  static const char*      DOF_PROP;
  static const char*      ROT_DOF_PROP;

  explicit                FollowerLoadModel

    ( const String&         name,
      const Properties&     conf,
      const Properties&     props,
      const Properties&     globdat );

  virtual bool            takeAction

    ( const String&         action,
      const Properties&     params,
      const Properties&     globdat );

  static Ref<Model>       makeNew

    ( const String&      name,
      const Properties&  conf,
      const Properties&  props,
      const Properties&  globdat );

  static void             declare ();

 private:
  void                    get_ext_vec_   
  ( const Vector&     fext,
    const double      scale,
    const Properties& globdat ) const;

  void                    advance_rots_
  ( const Vector&     d ) const;

  Assignable<NodeSet>     nodes_;
  String                  nameGroup_;  
  Assignable<NodeGroup>   group_;
  Ref<DofSpace>           dofs_;
  StringVector            force_dofs_;
  IdxVector               idofs_f_;
  StringVector            rot_dofs_;
  IdxVector               idofs_r_;
  Vector                  org_dir_;
  Cubix                   rot_Mats_;
};