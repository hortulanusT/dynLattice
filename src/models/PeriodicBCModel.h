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

#include <jem/util/Properties.h>

#include <jive/Array.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/Actions.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Constraints.h>
#include <jive/util/Assignable.h>
#include <jive/fem/NodeSet.h>
#include <jive/fem/NodeGroup.h>

#include "testing.h"
#include "helpers.h"

using jem::util::Properties;

using namespace jive;
using jive::model::Model;
using jive::util::DofSpace;
using jive::util::Constraints;
using jive::util::Assignable;
using jive::fem::NodeSet;
using jive::fem::NodeGroup;

using namespace jive_helpers;

class periodicBCModel : public Model
{
 public:
  static const char*      TYPE_NAME;
  static const char*      SETS_PROP;
  static const char*      DIR_PROP;
  static const char*      PERIOD_PROP;
  static const char*      LOCK_DOFS_PROP;

  explicit                periodicBCModel

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
  void                    init_ 
  
    ( const Properties&     globdat );

  void                    setConstraints_ 
  
    ( const Properties&     globdat );
    
 private:
  Assignable<NodeSet>     nodes_;
  Ref<DofSpace>           dofs_;
  Ref<Constraints>        cons_;
  StringVector            nodeSets_;
  Vector                  periodVector_;
  StringVector            periodDofs_;
};