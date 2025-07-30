/**
 * @file PeriodicBCModel.h
 * @author Til Gärtner
 * @brief Periodic boundary conditions model for rectangular unit cells
 *
 * This model implements periodic boundary conditions that work only for
 * rectangular unit cells. It provides load, displacement, and update modes
 * for applying periodic constraints on lattice structures.
 *
 * Copyright (C) 2021 TU Delft. All rights reserved.
 */

#pragma once

#include <jem/base/Object.h>

#include <cmath>

#include <jem/base/Float.h>
#include <jem/util/Properties.h>
#include <jem/util/PropertyException.h>

#include <jive/Array.h>
#include <jive/fem/NodeGroup.h>
#include <jive/fem/NodeSet.h>
#include <jive/implict/ArclenActions.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/Names.h>
#include <jive/model/StateVector.h>
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
using jive::implict::ArclenActions;
using jive::implict::ArclenParams;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::DofSpace;
using jive::util::Globdat;

using jive_helpers::IdxVectorMatrix;
using jive_helpers::vec2mat;

//-----------------------------------------------------------------------
//   class PeriodicBCModel
//-----------------------------------------------------------------------

/**
 * @class PeriodicBCModel
 * @brief Model for applying periodic boundary conditions on rectangular unit cells
 *
 * The PeriodicBCModel implements periodic boundary conditions specifically
 * designed for rectangular unit cells. It supports multiple operation modes
 * including load control, displacement control, and gradient updates.
 *
 * Features:
 * - Three operation modes: LOAD, UPD (update), and DISP (displacement)
 * - Automatic corner constraint handling
 * - Master-slave edge DOF pairing for periodicity
 * - Gradient-based deformation control
 * - Support for rotational DOFs
 *
 * @note Only works for rectangular unit cells
 */
class PeriodicBCModel : public Model
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;       ///< Model type name
  static const char *MODE_PROP;       ///< Operation mode property
  static const char *GRAD_PROP;       ///< Gradient property
  static const char *DOF_NAMES_PROP;  ///< DOF names property
  static const char *ROT_NAMES_PROP;  ///< Rotation DOF names property
  static const char *FIXEDGRAD_PARAM; ///< Fixed gradient parameter
  /// @}

  JEM_DECLARE_CLASS(PeriodicBCModel, Model);

  /// @brief Operation modes
  enum Mode
  {
    LOAD, ///< Load control mode (NOT IMPLEMENTED)
    UPD,  ///< Update mode
    DISP  ///< Displacement control mode
  };

  /// @brief Constructor
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit PeriodicBCModel(const String &name,
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

  /// @brief Create new PeriodicBCModel instance
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

  /// @brief Fix corner constraints
  /// @param globdat Global data container
  /// @param currentGrad Current gradient matrix
  /// @param scale Scaling factor
  void fixCorners_(const Properties &globdat,
                   const Matrix &currentGrad,
                   const double scale = NAN);

  /// @brief Set periodic constraints
  void setConstraints_();

  /// @brief Get external force vector
  /// @param f External force vector
  /// @param globdat Global data container
  /// @param scale Scaling factor
  void getExtVec_(const Vector &f,
                  const Properties &globdat,
                  const double scale = 1.);

private:
  Assignable<NodeSet> nodes_;      ///< Node set
  Ref<DofSpace> dofs_;             ///< DOF space
  Ref<Constraints> cons_;          ///< Constraints
  String gradName_;                ///< Gradient name
  Matrix grad_;                    ///< Gradient matrix
  StringVector dofNames_;          ///< DOF names
  StringVector rotNames_;          ///< Rotation DOF names
  IdxVector jdofs_;                ///< J DOFs
  IdxVectorMatrix masterEdgeDofs_; ///< Master edge DOFs
  IdxVectorMatrix slaveEdgeDofs_;  ///< Slave edge DOFs
  IdxMatrix cornerDofs_;           ///< Corner DOFs
  IdxVector corner0Dofs_;          ///< Corner 0 DOFs
  idx_t pbcRank_;                  ///< PBC rank
  Mode mode_;                      ///< Operation mode
  bool ghostCorners_;              ///< Ghost corners flag
};
