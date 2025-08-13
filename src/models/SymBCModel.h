/**
 * @file SymBCModel.h
 * @author Til Gärtner
 * @brief Symmetry boundary conditions model for plane symmetry constraints
 *
 * This model implements plane symmetry boundary conditions that can enforce
 * both opposing and equal DOF constraints between symmetric surfaces. It
 * supports multiple surface pairs with configurable symmetry types.
 */

#pragma once

#include <jem/base/Object.h>

#include <jem/util/ArrayBuffer.h>
#include <jem/util/Properties.h>

#include <jive/Array.h>
#include <jive/fem/NodeGroup.h>
#include <jive/fem/NodeSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/Names.h>
#include <jive/util/Assignable.h>
#include <jive/util/Constraints.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Globdat.h>

#include "modules/PBCGroupInputModule.h"
#include "utils/helpers.h"
#include "utils/testing.h"

using jem::util::Properties;
using jive::BoolMatrix;
using jive::IdxMatrix;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::DofSpace;
using jive::util::Globdat;

typedef jem::util::ArrayBuffer<idx_t> IdxBuffer;

//-----------------------------------------------------------------------
//   class SymBCModel
//-----------------------------------------------------------------------

/**
 * @class SymBCModel
 * @brief Model for symmetry boundary conditions on plane symmetric domains
 *
 * The SymBCModel implements plane symmetry boundary conditions that enforce
 * constraints between symmetric surfaces. It supports two types of symmetry:
 * opposing DOFs (antisymmetric) and equal DOFs (symmetric). Surface pairs
 * are automatically constrained based on the specified symmetry type.
 *
 * Features:
 * - Opposing DOF constraints for antisymmetric behavior
 * - Equal DOF constraints for symmetric behavior
 * - Multiple surface pair support
 * - Automatic constraint generation between symmetric surfaces
 * - Configurable DOF type specification
 */
class SymBCModel : public Model
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;            ///< Model type name
  static const char *DOF_OPPO_NAMES_PROP;  ///< Opposing DOF names property
  static const char *DOF_EQUAL_NAMES_PROP; ///< Equal DOF names property
  static const char *SURFACES_PROP;        ///< Surface pairs property
  /// @}

  JEM_DECLARE_CLASS(SymBCModel, Model);

  /// @brief Constructor
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit SymBCModel(const String &name,
                      const Properties &conf,
                      const Properties &props,
                      const Properties &globdat);

  /// @brief Handle model actions
  /// @param action Action name
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction(const String &action,
                          const Properties &params,
                          const Properties &globdat) override;

  /// @brief Create new SymBCModel instance
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return New model instance
  static Ref<Model> makeNew(const String &name,
                            const Properties &conf,
                            const Properties &props,
                            const Properties &globdat);

  /// @brief Declare model type to factory
  static void declare();

private:
  /// @brief Initialize model
  /// @param globdat Global data container
  void init_(const Properties &globdat);

  /// @brief Set symmetry constraints
  void setConstraints_();

private:
  Assignable<NodeSet> nodes_;    ///< Node set
  Ref<DofSpace> dofs_;           ///< DOF space
  Ref<Constraints> cons_;        ///< Constraints
  StringVector dofOppoNames_;    ///< Opposing DOF names
  StringVector dofEqualNames_;   ///< Equal DOF names
  StringVector surfaceNames_;    ///< Surface pair names
  IdxVector mainOppoDofs_;       ///< Main opposing DOFs
  IdxVector secondaryOppoDofs_;  ///< Secondary opposing DOFs
  IdxVector mainEqualDofs_;      ///< Main equal DOFs
  IdxVector secondaryEqualDofs_; ///< Secondary equal DOFs
  idx_t pbcRank_;                ///< PBC rank
};
