/**
 * @file HingeModel.h
 * @author Til Gärtner
 * @brief Hinge model for rigid joints between Cosserat rod elements
 */

#pragma once

#include <jem/base/Class.h>
#include <jem/util/ArrayBuffer.h>
#include <jem/util/Properties.h>
#include <jem/util/StringUtils.h>

#include <jive/Array.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/fem/NodeSet.h>
#include <jive/fem/XElementSet.h>
#include <jive/fem/XNodeSet.h>
#include <jive/implict/Names.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/StateVector.h>
#include <jive/solver/Solver.h>
#include <jive/util/Assignable.h>
#include <jive/util/Constraints.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>

#include "models/specialCosseratRodModel.h"
#include "utils/helpers.h"
#include "utils/testing.h"

using jem::newInstance;
using jem::Ref;
using jem::String;
using jem::numeric::Function;
using jem::util::Properties;

using jive::BoolMatrix;
using jive::StringVector;
using jive::algebra::MatrixBuilder;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::fem::NodeSet;
using jive::fem::XElementSet;
using jive::fem::XNodeSet;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::DofSpace;
using jive::util::FuncUtils;

typedef jem::util::ArrayBuffer<idx_t> IdxBuffer;

/// @brief Model for plastic hinges connecting multiple Cosserat rod elements
/// @details Implements plastic hinge behavior for geometrically nonlinear rod elements.
/// Should be used with specialCosseratRodModel to create joints between rods with
/// yield condition evaluation and plastic constraint enforcement.
class hingeModel : public Model
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;  ///< Model type name
  static const char *YIELD_PROP; ///< Yield condition property
  /// @}

  JEM_DECLARE_CLASS(hingeModel, Model);

  /// @brief Constructor with configuration and properties
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit hingeModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Handle model actions (INIT, GET_MATRIX, GET_INT_VECTOR, etc.)
  /// @param action Action name to execute
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat);

  /// @brief Factory method for creating new hingeModel instances
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return Reference to new hingeModel instance
  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Register hingeModel type with ModelFactory
  static void declare();

private:
  /// @brief Create hinge elements for specified element group
  /// @param elementName Name of the element group
  /// @param globdat Global database
  /// @return Created element group
  ElementGroup createHinges_(const String &elementName, const Properties &globdat);

  /// @brief Store forces relevant for existing hinges
  /// @param fint Global internal force vector
  void updForces_(const Vector &fint);

  /// @brief Evaluate plastic development in the hinges
  /// @return true if no new plasticity was discovered, false if plastic movement is detected
  bool evalPlastic_(const Vector &disp);

  /// @brief Initialize model state and DOF mappings
  /// @param globdat Global data container
  void init_(const Properties &globdat);

  /// @brief Get constraint information
  /// @brief Get constraint information
  void getCons_();

private:
  /// @name System components
  /// @{
  Assignable<ElementGroup> egroup_; ///< Hinge element group
  Assignable<NodeSet> nodes_;       ///< Node set
  Assignable<ElementSet> elems_;    ///< Element set
  Ref<DofSpace> dofs_;              ///< Degree of freedom space
  Ref<Constraints> cons_;           ///< Constraint manager
  /// @}

  /// @name DOF and element management
  /// @{
  IdxVector jtypes_;    ///< Joint type indices
  StringVector jnames_; ///< Joint type names
  String elName_;       ///< Element group name
  /// @}

  /// @name Iteration control
  /// @{
  double prec_;   ///< Convergence precision
  idx_t maxIter_; ///< Maximum iterations
  idx_t iiter_;   ///< Current iteration counter
  /// @}

  /// @name Material and yield behavior
  /// @{
  Ref<Material> material_;  ///< Material reference
  Ref<Function> yieldCond_; ///< Yield condition function
  Vector ell_;              ///< Element lengths
  Matrix intForces_;        ///< Current internal forces
  Matrix intForcesOld_;     ///< Previous internal forces
  Matrix plasticDisp_;      ///< Plastic displacements
  /// @}
};
