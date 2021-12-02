/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *  
 * This class implements a joint for several rods
 * (should be used with the specialCosseratRodModel)
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
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Constraints.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/fem/NodeSet.h>

#include "testing.h"
#include "helpers.h"

using jem::util::Properties;

using namespace jive;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::Constraints;
using jive::algebra::MatrixBuilder;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::fem::NodeSet;

using namespace jive_helpers;

class rodJointModel : public Model
{
 public:
  static const char*      TYPE_NAME;

  explicit                rodJointModel

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
  /**
   * @brief assemble the stiffness matrix for rotational springs
   * @param[out] mbld tanget stiffness matrix (via MatrixBuilder object)
   * @param[out] fint internal force Vector
   * @param[in]  disp current values for the DOFs 
   */
  void                    assembleRot_

  ( MatrixBuilder&        mbld,
    const Vector&         fint,
    const Vector&         disp );

  /**
   * @brief assemble the stiffness matrix for translational springs
   * @param[out] mbld tanget stiffness matrix (via MatrixBuilder object)
   * @param[out] fint internal force Vector
   * @param[in]  disp current values for the DOFs 
   */
  void                    assembleTrans_

  ( MatrixBuilder&        mbld,
    const Vector&         fint,
    const Vector&         disp );


  void                    init_ ();

 private:
  Assignable<ElementGroup>egroup_;
  Assignable<NodeSet>     nodes_;
  Assignable<ElementSet>  elems_;
  Ref<DofSpace>           dofs_;
  Ref<Constraints>        constraints_;

  StringVector            lockDofs_;

  StringVector            rotDofs_;
  IdxVector               iRotDofs_;
  double                  rot_stiff_;
  
  StringVector            transDofs_;
  IdxVector               iTransDofs_;
  double                  trans_stiff_;
};