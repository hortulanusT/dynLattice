/**
 * @file NeumannModel.h
 * @author Frans P. van der Meer
 * @brief Neumann boundary conditions model for applying loads on node groups
 *
 * This model implements Neumann boundary conditions that can apply loads on
 * changing NodeGroups. Unlike PointLoadModel, this model can handle dynamic
 * node group configurations and provides load increment control capabilities.
 */

#pragma once

#include <jem/base/Object.h>
#include <jem/io/Writer.h>
#include <jive/Array.h>
#include <jive/fem/NodeGroup.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/util/Assignable.h>
#include <jive/util/XDofSpace.h>

using jem::Float;
using jem::idx_t;
using jem::IllegalInputException;
using jem::newInstance;
using jem::Ref;
using jem::SliceFrom;
using jem::String;
using jem::System;
using jem::io::Writer;
using jem::util::Properties;
using jive::IntVector;
using jive::StringVector;
using jive::Vector;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::Constraints;
using jive::util::DofSpace;
using jive::util::XDofSpace;

//-----------------------------------------------------------------------
//   class NeumannModel
//-----------------------------------------------------------------------

/**
 * @class NeumannModel
 * @brief Model for applying Neumann boundary conditions on node groups
 *
 * The NeumannModel applies load boundary conditions on specified node groups
 * with configurable load increment control. Unlike point loads, this model
 * can handle dynamic node group configurations and provides sophisticated
 * load scaling and increment management for nonlinear analysis.
 *
 * Features:
 * - Load application on multiple node groups with individual DOF types
 * - Load increment control with adaptive stepping
 * - Initial load specification and load reduction capabilities
 * - External scaling support for displacement control
 * - Maximum load value constraints
 */
class NeumannModel : public Model
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;      ///< Model type name
  static const char *LOAD_INCR_PROP; ///< Load increment property
  static const char *INIT_LOAD_PROP; ///< Initial load property
  static const char *MIN_LOAD_PROP;  ///< Minimum load increment property
  static const char *MAX_LOAD_PROP;  ///< Maximum load value property
  static const char *REDUCTION_PROP; ///< Load reduction factor property
  static const char *NODES_PROP;     ///< Node groups property
  static const char *DOF_PROP;       ///< DOF types property
  static const char *FACTORS_PROP;   ///< Load factors property
  /// @}

  JEM_DECLARE_CLASS(NeumannModel, Model);

  /// @brief Constructor
  /// @param name Model name
  explicit NeumannModel(const String &name);

  /// @brief Configure the model from properties
  /// @param props User-specified model properties
  /// @param globdat Global data container
  virtual void configure(const Properties &props,
                         const Properties &globdat) override;

  /// @brief Get current model configuration
  /// @param conf Actually used configuration properties (output)
  /// @param globdat Global data container
  virtual void getConfig(const Properties &conf,
                         const Properties &globdat) const override;

  /// @brief Handle model actions
  /// @param action Action name
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction(const String &action,
                          const Properties &params,
                          const Properties &globdat) override;

  /// @brief Set load increment
  /// @param incr New load increment value
  void setLoadIncr(double incr);

  /// @brief Get current load increment
  /// @return Current load increment
  inline double getLoadIncr() const;

  /// @brief Create new NeumannModel instance
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
  virtual ~NeumannModel();

private:
  /// @brief Initialize model
  /// @param globdat Global data container
  void init_(const Properties &globdat);

  /// @brief Compute external force vector
  /// @param fext External force vector
  /// @param globdat Global data container
  void getExtVector_(const Vector &fext,
                     const Properties &globdat) const;

  /// @brief Compute external force vector with scaling
  /// @param fext External force vector
  /// @param globdat Global data container
  /// @param scale Scaling factor
  void getExtVector_(const Vector &fext,
                     const Properties &globdat,
                     const double scale) const;

  /// @brief Advance to next time step
  /// @param globdat Global data container
  void advance_(const Properties &globdat);

  /// @brief Commit current step
  /// @param params Action parameters
  /// @param globdat Global data container
  void commit_(const Properties &params,
               const Properties &globdat);

  /// @brief Reduce step size
  /// @param params Action parameters
  /// @param globdat Global data container
  void reduceStep_(const Properties &params,
                   const Properties &globdat);

  /// @brief Increase step size
  /// @param params Action parameters
  /// @param globdat Global data container
  void increaseStep_(const Properties &params,
                     const Properties &globdat);

private:
  Ref<DofSpace> dofs_;        ///< DOF space
  Assignable<NodeSet> nodes_; ///< Node set

  idx_t ngroups_;   ///< Number of node groups
  IntVector idofs_; ///< DOF indices

  double loadScale0_; ///< Previous load scale
  double loadScale_;  ///< Current load scale
  double loadIncr_;   ///< Current load increment

  // Input specification for boundary conditions
  StringVector nodeGroups_; ///< Node group names
  StringVector dofTypes_;   ///< DOF type names
  Vector factors_;          ///< Load factors per group

  // Configuration parameters
  double reduction_;   ///< Load reduction factor
  double loadIncr0_;   ///< Initial load increment
  double minLoadIncr_; ///< Minimum load increment
  double maxLoadVal_;  ///< Maximum load value
  double initLoad_;    ///< Initial load value

  String varName_; ///< Variable name for globdat
  bool extScale_;  ///< External scaling flag
};
