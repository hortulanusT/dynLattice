/**
 * @file JointContactModel.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief contact model for joints (assumed to be two spheres)
 * @version 0.1
 * @date 2025-01-07
 *
 * @copyright Copyright (C) 2024 TU Delft. All rights reserved.
 *
 */

#pragma once

#include "models/specialCosseratRodModel.h"
#include <jem/base/ClassTemplate.h>
#include <jem/util/ArrayBuffer.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/algebra/NullMatrixBuilder.h>
#include <jive/app/Names.h>
#include <jive/fem/ElementSet.h>
#include <jive/fem/NodeGroup.h>
#include <jive/implict/Names.h>
#include <jive/model/Model.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>

using jem::Array;
using jem::numeric::Function;
using jem::util::ArrayBuffer;
using jive::idx_t;
using jive::IdxVector;
using jive::Matrix;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::Vector;
using jive::algebra::MatrixBuilder;
using jive::algebra::NullMatrixBuilder;
using jive::fem::ElementSet;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::FuncUtils;

class JointContactModel : public Model
{
public:
  JEM_DECLARE_CLASS(JointContactModel, Model);

  static const char *TYPE_NAME;
  static const char *PENALTY_PROP;
  static const char *RADIUS_PROP;
  static const char *VERBOSE_PROP;

  explicit JointContactModel

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
  /**
   * @brief find pairs of contacts
   *
   * @param[out] nodesA node IDs of one side of the contact
   * @param[out] nodesB node IDs of the other side
   * @param[in] disp displacement vector of all nodes
   */
  virtual void findContacts_

      (IdxVector &nodesA,
       IdxVector &nodesB,
       const Vector &disp) const;

  /**
   * @brief compute the effects of the contact
   *
   * @param[out] mbld stiffness matrix builder
   * @param[out] fint internal force vector
   * @param[in] nodesA node IDs of one side of the contact
   * @param[in] nodesB node IDs of the other side
   * @param[in] disp displacement vector of all elements
   */
  virtual void computeContacts_

      (MatrixBuilder &mbld,
       const Vector &fint,
       const IdxVector &nodesA,
       const IdxVector &nodesB,
       const Vector &disp);

  /**
   * @brief commpute a single contact
   *
   * @param contactStiffness stiffness matrix of the contact
   * @param contactForce force vector of the contact
   * @param posA position of the first node
   * @param posB position of the second node
   *
   */
  virtual void computeContact_

      (Matrix contactStiffness,
       Vector contactForce,
       Vector posA,
       Vector posB);

private:
  Assignable<NodeSet> allNodes_;
  Assignable<ElementSet> allElems_;
  IdxVector jointList_;
  Ref<DofSpace> dofs_;

  ArrayBuffer<idx_t> contactsA_;
  ArrayBuffer<idx_t> contactsB_;

  Ref<Function> updCond_;

  double penalty_;
  double radius_;
  bool verbose_;
};
