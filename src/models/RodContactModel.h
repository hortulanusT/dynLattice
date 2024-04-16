/**
 * @file RodContactModel.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief contact model for rods
 * @version 0.1
 * @date 2024-04-16
 *
 * @copyright Copyright (C) 2024 TU Delft. All rights reserved.
 *
 */

#pragma once

#include "models/LatticeModel.h"
#include <jem/base/Array.h>
#include <jem/base/System.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/fem/ElementSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>

using jem::ALL;
using jem::newInstance;
using jem::Ref;
using jive::idx_t;
using jive::IdxVector;
using jive::Matrix;
using jive::Properties;
using jive::String;
using jive::Vector;
using jive::algebra::MatrixBuilder;
using jive::fem::ElementSet;
using jive::fem::NodeSet;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::Assignable;
using jive::util::DofSpace;

class RodContactModel : public Model
{
public:
  JEM_DECLARE_CLASS(RodContactModel, Model);

  static const char *TYPE_NAME;

  explicit RodContactModel

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
   * @param[out] elementsA element IDs of one side of the contact
   * @param[out] elementsB element IDs of the other side
   * @param[in] disp displacement vector of all elements
   */
  virtual void findContacts_

      (IdxVector &elementsA,
       IdxVector &elementsB,
       const Vector &disp) const;

  /**
   * @brief find beams of possible contact pairs
   *
   * @param beamsA indices of the beams in contact
   * @param beamsB indices of corresponding beams
   * @param disp displacement vector of all elements
   */
  virtual void findPossibleBeams_

      (IdxVector &beamsA,
       IdxVector &beamsB,
       const Vector &disp) const;

  /**
   * @brief get the contacting point between to beams in possible contact
   *
   * @param elemsA element ID of one side of the contact
   * @param elemsB corresponding element ID on the other side
   * @param beamA possible beam on side A
   * @param beamB possible beam on side B
   * @param disp displacement vector of all elements
   */

  virtual void findPossibleElements_

      (IdxVector &elemsA,
       IdxVector &elemsB,
       const idx_t &beamA,
       const idx_t &beamB,
       const Vector &disp) const;

  /**
   * @brief compute the effects of the contact
   *
   * @param[out] mbld stiffness matrix builder
   * @param[out] fint internal force vector
   * @param[in] elementsA element IDs of one side of the contact
   * @param[in] elementsB element IDs of the other side
   * @param[in] disp displacement vector of all elements
   */
  virtual void computeContacts_

      (MatrixBuilder &mbld,
       const Vector &fint,
       const IdxVector &elementsA,
       const IdxVector &elementsB,
       const Vector &disp) const;

  /**
   * @brief compute the effects of the contact
   *
   * @param[out] fint internal force vector
   * @param[in] elementsA element IDs of one side of the contact
   * @param[in] elementsB element IDs of the other side
   * @param[in] disp displacement vector of all elements
   */
  virtual void computeContacts_

      (const Vector &fint,
       const IdxVector &elementsA,
       const IdxVector &elementsB,
       const Vector &disp) const;

private:
  Assignable<NodeSet> allNodes_;
  Assignable<ElementSet> allElems_;
  Array<Assignable<ElementGroup>> rodList_;
  Ref<DofSpace> dofs_;
};
