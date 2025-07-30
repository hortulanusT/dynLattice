/**
 * @file JointContactModel.h
 * @author Til Gärtner
 * @brief Contact model for spherical joint interactions
 */

#pragma once

#include <jem/base/Class.h>

#include "models/SpecialCosseratRodModel.h"
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

/// @brief Contact model for spherical joint interactions
/// @details Implements contact mechanics between joints modeled as spheres using
/// penalty methods. Detects contact pairs and computes contact forces and stiffness
/// contributions for joint-to-joint interactions.
class JointContactModel : public Model
{
public:
  JEM_DECLARE_CLASS(JointContactModel, Model);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;    ///< Model type name
  static const char *PENALTY_PROP; ///< Penalty parameter
  static const char *RADIUS_PROP;  ///< Joint radius
  static const char *VERBOSE_PROP; ///< Verbose output flag
  /// @}

  /// @}

  /// @brief Constructor with configuration and properties
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit JointContactModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Handle model actions for contact mechanics
  /// @param action Action name to execute
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat) override;

  /// @brief Factory method for creating new JointContactModel instances
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return Reference to new JointContactModel instance
  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Register JointContactModel type with ModelFactory
  static void declare();

protected:
  /// @brief Find pairs of joints in contact
  /// @param disp Displacement vector of all nodes
  virtual void findContacts_

      (const Vector &disp);

  /// @brief Compute contact effects for detected contact pairs
  /// @param mbld Stiffness matrix builder
  /// @param fint Internal force vector
  /// @param nodesA Node IDs of first contact side
  /// @param nodesB Node IDs of second contact side
  /// @param disp Displacement vector of all elements
  virtual void computeContacts_

      (MatrixBuilder &mbld,
       const Vector &fint,
       const IdxVector &nodesA,
       const IdxVector &nodesB,
       const Vector &disp);

  /// @brief Compute single contact interaction
  /// @param contactStiffness Contact stiffness matrix (output)
  /// @param contactForce Contact force vector (output)
  /// @param posA Position of first joint
  /// @param posB Position of second joint
  virtual void computeContact_

      (Matrix contactStiffness,
       Vector contactForce,
       Vector posA,
       Vector posB);

private:
  /// @name System components
  /// @{
  Assignable<NodeSet> allNodes_;    ///< All nodes in the system
  Assignable<ElementSet> allElems_; ///< All elements in the system
  IdxVector jointList_;             ///< List of joint node indices
  Ref<DofSpace> dofs_;              ///< Degree of freedom space
  /// @}

  /// @name Contact detection
  /// @{
  ArrayBuffer<idx_t> contactsA_; ///< First nodes in contact pairs
  ArrayBuffer<idx_t> contactsB_; ///< Second nodes in contact pairs
  Ref<Function> updCond_;        ///< Update condition function
  /// @}

  /// @name Contact parameters
  /// @{
  double penalty_; ///< Penalty parameter for contact forces
  double radius_;  ///< Joint radius for contact detection
  bool verbose_;   ///< Enable verbose output
  /// @}
};
