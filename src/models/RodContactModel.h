/**
 * @file RodContactModel.h
 * @author Til Gärtner
 * @brief Contact model for rod-to-rod interactions
 *
 * This model implements contact mechanics between rod elements using penalty
 * methods. It supports both segment-to-segment (STS) and node-to-segment (NTS)
 * contact formulations with configurable penalty parameters and contact filtering.
 */

#pragma once

#include <jem/base/Object.h>

#include "misc/Line3D.h"
#include "models/LatticeModel.h"
#include "models/SpecialCosseratRodModel.h"
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

//-----------------------------------------------------------------------
//   class RodContactModel
//-----------------------------------------------------------------------

/// @class RodContactModel
/// @brief Model for rod-to-rod contact interactions using penalty methods
///
/// The RodContactModel implements contact mechanics between rod elements using
/// penalty-based formulations. It supports both segment-to-segment and node-to-segment
/// contact types with automatic contact detection, blacklist filtering, and
/// configurable penalty parameters.
///
/// Features:
/// - Segment-to-segment (STS) and node-to-segment (NTS) contact formulations
/// - Automatic contact pair detection and filtering
/// - Blacklist system to exclude initial contacts
/// - Configurable penalty parameters for different contact types
/// - Rod radius specification for contact detection
/// - Verbose output options for debugging
class RodContactModel : public Model
{

 public:
  JEM_DECLARE_CLASS(RodContactModel, Model);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;        ///< Model type name
  static const char *PENALTY_PROP;     ///< General penalty property
  static const char *PENALTY_STS_PROP; ///< Segment-to-segment penalty property
  static const char *PENALTY_NTS_PROP; ///< Node-to-segment penalty property
  static const char *RADIUS_PROP;      ///< Rod radius property
  static const char *VERBOSE_PROP;     ///< Verbose output property
  /// @}

  /// @brief Constructor
  /// @param[in] name  Model name
  /// @param[out] conf  Actually used configuration properties (output)
  /// @param[in] props  User-specified model properties
  /// @param[in] globdat  Global data container
  explicit RodContactModel(const String &name,
                           const Properties &conf,
                           const Properties &props,
                           const Properties &globdat);

  /// @brief Handle model actions
  /// @param[in] action  Action name
  /// @param[in] params  Action parameters
  /// @param[in] globdat  Global data container
  /// @return true if action was handled
  virtual bool takeAction(const String &action,
                          const Properties &params,
                          const Properties &globdat) override;

  /// @brief Create new RodContactModel instance
  /// @param[in] name  Model name
  /// @param[out] conf  Actually used configuration properties (output)
  /// @param[in] props  User-specified model properties
  /// @param[in] globdat  Global data container
  /// @return New model instance
  static Ref<Model> makeNew(const String &name,
                            const Properties &conf,
                            const Properties &props,
                            const Properties &globdat);

  /// @brief Declare model type to factory
  static void declare();

 protected:
  /// @brief Find pairs of contacts
  /// @param[out] elementsA  Element indices of one side of the contact
  /// @param[out] elementsB  Element indices of the other side
  /// @param[in] disp  Displacement vector of all elements
  virtual void findContacts_(IdxVector &elementsA,
                             IdxVector &elementsB,
                             const Vector &disp) const;

  /// @brief Compute the bounding box of every rod element in the current configuration
  /// @details Each box encloses the displaced element nodes and is widened by
  /// the contact radius. Computed once per contact search, so that the
  /// element-level search of every rod pair only compares boxes.
  /// @param[out] elemBoxMin  Lower box corners (globalRank x element count), column = element index
  /// @param[out] elemBoxMax  Upper box corners (globalRank x element count), column = element index
  /// @param[in] disp  Displacement vector of all elements
  void getElementBoxes_(Matrix &elemBoxMin,
                        Matrix &elemBoxMax,
                        const Vector &disp) const;

  /// @brief Find beams of possible contact pairs
  /// @param[out] beamsA  Indices of the beams in contact
  /// @param[out] beamsB  Indices of corresponding beams
  /// @param[in] disp  Displacement vector of all elements
  virtual void findPossibleBeams_(IdxVector &beamsA,
                                  IdxVector &beamsB,
                                  const Vector &disp) const;

  /// @brief Get the contacting point between two beams in possible contact
  /// @param[out] elemsA  Element index of one side of the contact
  /// @param[out] elemsB  Corresponding element index on the other side
  /// @param[in] beamA  Possible beam on side A
  /// @param[in] beamB  Possible beam on side B
  /// @param[in] elemBoxMin  Lower element box corners (see getElementBoxes_)
  /// @param[in] elemBoxMax  Upper element box corners (see getElementBoxes_)
  virtual void findPossibleElements_(IdxVector &elemsA,
                                     IdxVector &elemsB,
                                     const idx_t &beamA,
                                     const idx_t &beamB,
                                     const Matrix &elemBoxMin,
                                     const Matrix &elemBoxMax) const;

  /// @brief Compute the effects of the contact
  /// @param[in,out] mbld  Stiffness matrix builder, updated with contact contributions
  /// @param[in,out] fint  Internal force vector, updated with contact contributions
  /// @param[in] elementsA  Element indices of one side of the contact
  /// @param[in] elementsB  Element indices of the other side
  /// @param[in] disp  Displacement vector of all elements
  virtual void computeContacts_(MatrixBuilder &mbld,
                                const Vector &fint,
                                const IdxVector &elementsA,
                                const IdxVector &elementsB,
                                const Vector &disp);

  /// @brief Build the list of nodes initially in contact
  /// @param[in] elementsA  Element indices A
  /// @param[in] elementsB  Element indices B
  /// @param[in] disp  Displacement vector
  virtual void computeBlacklist_(const IdxVector &elementsA,
                                 const IdxVector &elementsB,
                                 const Vector &disp);

  /// @brief Check whether a contact is on the blacklist
  /// @param[in] elementsA  Element index A
  /// @param[in] elementsB  Element index B
  /// @return true if the contact is on the blacklist
  virtual bool filterBlacklist_(const idx_t elementsA,
                                const idx_t elementsB) const;

  /// @brief Find the local coordinates of the closest points on two beams
  /// @param[out] uA  Local coordinate Beam A
  /// @param[out] uB  Local coordinate Beam B
  /// @param[in] possA  Positions Beam A
  /// @param[in] possB  Positions Beam B
  virtual void findClosestPoints_(double &uA,
                                  double &uB,
                                  const Matrix &possA,
                                  const Matrix &possB) const;

  /// @brief Get the closest coordinate on the main element to the secondary point
  /// @param[in] posS  Secondary point
  /// @param[in] possM  Main element positions
  /// @return Coordinate on the main element
  virtual double getClosestPoint_(const Vector &posS,
                                  const Matrix &possM) const;

  /// @brief Compute the force and stiffness contributions of a segment-to-segment contact
  /// @param[out] f_contrib  Force contribution
  /// @param[out] k_contrib  Stiffness contribution
  /// @param[in] possA  Positions of Beam A
  /// @param[in] possB  Positions of Beam B
  /// @param[in] uA  Local coordinate Beam A
  /// @param[in] uB  Local coordinate Beam B
  /// @return true if contact is active
  /// @see [Wriggers, Zavarise (1999)](https://doi.org/10.1002/(SICI)1099-0887(199706)13:6%3C429::AID-CNM70%3E3.0.CO;2-X)
  virtual bool computeSTS_(Vector &f_contrib,
                           Matrix &k_contrib,
                           const Matrix &possA,
                           const Matrix &possB,
                           const double uA,
                           const double uB) const;

  /// @brief Compute the force and stiffness contributions of a node-to-segment contact
  /// @param[out] f_contrib  Force contribution
  /// @param[out] k_contrib  Stiffness contribution
  /// @param[in] possS  Position of the secondary node
  /// @param[in] possM  Positions of the main beam
  /// @param[in] uM  Local coordinate of the main beam
  /// @return true if contact is active
  /// @see [Wriggers, Simo (1985)](https://doi.org/10.1002/cnm.1630010503)
  virtual bool computeNTS_(Vector &f_contrib,
                           Matrix &k_contrib,
                           const Vector &possS,
                           const Matrix &possM,
                           const double uM) const;

 private:
  Assignable<NodeSet> allNodes_;            ///< All nodes in the model
  Assignable<ElementSet> allElems_;         ///< All elements in the model
  Array<Assignable<ElementGroup>> rodList_; ///< List of rod element groups
  Array<IdxVector> rodNodes_;               ///< Cached unique node indices of each rod
  Ref<DofSpace> dofs_;                      ///< DOF space
  Ref<Line3D> shape_;                       ///< Line shape functions

  IdxVector blacklistA_; ///< Blacklisted elements A
  IdxVector blacklistB_; ///< Blacklisted elements B

  ArrayBuffer<idx_t> contactsA_; ///< Contact elements A
  ArrayBuffer<idx_t> contactsB_; ///< Contact elements B

  Ref<Function> updCond_; ///< Update condition function

  double penaltySTS_; ///< Segment-to-segment penalty parameter
  double penaltyNTS_; ///< Node-to-segment penalty parameter
  double radius_;     ///< Rod radius
  bool verbose_;      ///< Verbose output flag
};
