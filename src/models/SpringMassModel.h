/**
 * @file SpringMassModel.h
 * @author Til Gärtner
 * @brief Spring-mass system model for boundary conditions
 *
 * This model emulates singular beams at boundaries to create spring-mass
 * systems. It provides a way to model boundary conditions through equivalent
 * spring-mass representations with configurable spring elements.
 */

#pragma once

#include <jem/base/Object.h>

#include <jem/base/Array.h>
#include <jem/base/System.h>
#include <jem/util/ArrayBuffer.h>
#include <jem/util/Properties.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/NodeGroup.h>
#include <jive/fem/XElementSet.h>
#include <jive/fem/XNodeSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/util/Assignable.h>
#include <jive/util/Constraints.h>
#include <jive/util/DofSpace.h>

using jem::ALL;
using jem::Array;
using jem::Ref;
using jem::util::ArrayBuffer;
using jive::idx_t;
using jive::IdxMatrix;
using jive::IdxVector;
using jive::Matrix;
using jive::Properties;
using jive::String;
using jive::StringVector;
using jive::Vector;
using jive::fem::ElementGroup;
using jive::fem::NodeGroup;
using jive::fem::XElementSet;
using jive::fem::XNodeSet;
using jive::model::Actions;
using jive::model::Model;
using jive::model::ModelFactory;
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::DofSpace;

//-----------------------------------------------------------------------
//   class SpringMassModel
//-----------------------------------------------------------------------

/**
 * @class SpringMassModel
 * @brief Model for spring-mass systems at domain boundaries
 *
 * The SpringMassModel emulates singular beams at boundaries to create
 * spring-mass systems for boundary condition modeling. It automatically
 * generates spring elements along specified boundaries and manages
 * child model instances for the spring elements.
 *
 * Features:
 * - Automatic spring element generation at boundaries
 * - Configurable spring properties and element counts
 * - Edge node constraint management
 * - Child model integration for spring behavior
 * - Extent vector specification for spring placement
 */
class SpringMassModel : public Model
{

public:
  JEM_DECLARE_CLASS(SpringMassModel, Model);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;          ///< Model type name
  static const char *BOUNDARY_PROP;      ///< Boundary specification property
  static const char *SPRING_NAMES;       ///< Spring names property
  static const char *EXTENT_VECTOR_PROP; ///< Extent vector property
  static const char *N_ELEM_PROP;        ///< Number of elements property
  static const char *P_ELEM_PROP;        ///< Element parameter property
  /// @}

  /// @brief Constructor
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit SpringMassModel(const String &name,
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

  /// @brief Create new SpringMassModel instance
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

protected:
  /// @brief Protected destructor
  virtual ~SpringMassModel();

  /// @brief Apply constraints to the spring-mass system
  /// @param globdat Global data container
  /// @return true if constraints were applied successfully
  virtual bool applyConstraints_(const Properties &globdat) const;

private:
  Array<Ref<Model>> children_;     ///< Child model instances
  Assignable<NodeGroup> boundary_; ///< Boundary node group
  IdxMatrix edgeNodes_;            ///< Edge node connectivity
};
