/**
 * @file ImpactModel.h  
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Model that updates accelleration based on response forces and impactor weights
 * @version 0.1
 * @date 2022-06-28
 * 
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 * 
 */
#pragma once

#include <jem/base/System.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/model/Model.h>
#include <jive/model/Actions.h>
#include <jive/model/ModelFactory.h>
#include <jive/fem/NodeSet.h>
#include <jive/fem/NodeGroup.h>
#include <jive/util/Globdat.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Constraints.h>

using jem::idx_t;
using jem::newInstance;
using jive::Ref;
using jive::String;
using jive::Vector;
using jive::IdxVector;
using jive::StringVector;
using jive::Properties;
using jive::model::Model;
using jive::model::Actions;
using jive::fem::NodeSet;
using jive::fem::NodeGroup;
using jive::util::Globdat;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::Constraints;

class ImpactModel : public Model
{
 public:
  JEM_DECLARE_CLASS     ( ImpactModel, Model );

  static const char*    TYPE_NAME;
  static const char*    NODES_PROP;
  static const char*    DOF_PROP;
  static const char*    WEIGHTS_PROP;

  explicit              ImpactModel

    ( const String&           name,
      const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  virtual bool          takeAction

    ( const String&           action,
      const Properties&       params,
      const Properties&       globdat )  override;

  static Ref<Model>         makeNew

    ( const String&           name,
      const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  static void               declare         ();

 private:

  Ref<DofSpace>             dofs_;
  Ref<Constraints>          cons_;
  Assignable<NodeSet>       nodes_;
  
  StringVector              nodeGroups_;
  StringVector              dofNames_;
  Vector                    weights_;
};