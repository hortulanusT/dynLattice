/**
 * @file RodContactModel.h
 * @author Til Gärtner
 * @brief contact model for rods
 *
 *
 */

#pragma once

#include "misc/Line3D.h"
#include "models/LatticeModel.h"
#include "models/specialCosseratRodModel.h"
#include <jem/base/Array.h>
#include <jem/base/Error.h>
#include <jem/base/System.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/algebra/NullMatrixBuilder.h>
#include <jive/app/Names.h>
#include <jive/fem/ElementSet.h>
#include <jive/implict/Names.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/XTable.h>

using jem::ALL;
using jem::newInstance;
using jem::Ref;
using jive::idx_t;
using jive::IdxMatrix;
using jive::IdxVector;
using jive::Matrix;
using jive::Properties;
using jive::String;
using jive::Vector;
using jive::algebra::MatrixBuilder;
using jive::algebra::NullMatrixBuilder;
using jive::fem::ElementSet;
using jive::fem::NodeSet;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::FuncUtils;
using jive::util::XTable;

class RodContactModel : public Model
{
public:
  JEM_DECLARE_CLASS(RodContactModel, Model);

  static const char *TYPE_NAME;
  static const char *PENALTY_PROP;
  static const char *PENALTY_STS_PROP;
  static const char *PENALTY_NTS_PROP;
  static const char *RADIUS_PROP;
  static const char *VERBOSE_PROP;

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
       const Vector &disp);

  /**
   * @brief build the list of nodes initially in contact
   *
   * @param elementsA
   * @param elementsB
   * @param disp
   */
  virtual void computeBlacklist_

      (
          const IdxVector &elementsA,
          const IdxVector &elementsB,
          const Vector &disp);

  /**
   * @brief check weather a contact is on the blacklist
   *
   * @param elementA element ID
   * @param elementB element ID
   * @returns true if the contact is on the blacklist
   */
  virtual bool filterBlacklist_

      (const idx_t elementsA,
       const idx_t elementsB) const;

  /**
   * @brief find the local coordinates of the closest points on two beams
   *
   * @param uA local coordinate Beam A
   * @param uB local coordinate Beam B
   * @param possA positions Beam A
   * @param possB positions Beam B
   */
  virtual void findClosestPoints_

      (double &uA,
       double &uB,
       const Matrix &possA,
       const Matrix &possB) const;

  /**
   * @brief Get the closest coordinate on the main element to the secondary point
   *
   * @param posS secondary point
   * @param possM main element positions
   * @returns double coordinate on the main element
   */
  virtual double getClosestPoint_

      (const Vector &posS,
       const Matrix &possM) const;

  /**
   * @brief compute the force and stiffness contributions of a segment-to-segment contact
   *
   * @param f_contrib force contribution
   * @param k_contrib stiffness contribution
   * @param possA positions of Beam A
   * @param possB positions of Beam B
   * @param uA local coordinate Beam A
   * @param uB local coordinate Beam B
   */
  virtual bool computeSTS_

      (Vector &f_contrib,
       Matrix &k_contrib,
       const Matrix &possA,
       const Matrix &possB,
       const double uA,
       const double uB) const;

  /**
   * @brief compute the force and stiffness contiributions of a node-to-segment contact
   *
   * @param f_contrib force contribution
   * @param k_contrib stiffness contribution
   * @param possS position of the secondary node
   * @param possM positions of the main beam
   * @param uM local coordinate of the main beam
   */
  virtual bool computeNTS_

      (Vector &f_contrib,
       Matrix &k_contrib,
       const Vector &possS,
       const Matrix &possM,
       const double uM) const;

private:
  Assignable<NodeSet> allNodes_;
  Assignable<ElementSet> allElems_;
  Array<Assignable<ElementGroup>> rodList_;
  Ref<DofSpace> dofs_;
  Ref<Line3D> shape_;

  IdxVector blacklistA_;
  IdxVector blacklistB_;

  ArrayBuffer<idx_t> contactsA_;
  ArrayBuffer<idx_t> contactsB_;

  Ref<Function> updCond_;

  double penaltySTS_;
  double penaltyNTS_;
  double radius_;
  bool verbose_;
};
