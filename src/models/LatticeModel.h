/**
 * @file LatticeModel.h
 * @author Til Gärtner
 * @brief Lattice model for managing collections of rod elements
 */

#pragma once

#include <jem/base/Array.h>
#include <jem/base/CString.h>
#include <jem/base/Class.h>
#include <jem/base/IllegalInputException.h>
#include <jem/base/System.h>
#include <jem/base/array/Array.h>
#include <jem/numeric/algebra/utilities.h>
#include <jem/util/ArrayBuffer.h>
#include <jem/util/Properties.h>
#include <jive/algebra/AbstractMatrix.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Globdat.h>
#include <jive/util/ObjectConverter.h>
#include <jive/util/XTable.h>
#include <jive/util/utilities.h>

using jem::Array;
using jem::idx_t;
using jem::newInstance;
using jem::Ref;
using jem::numeric::dotProduct;
using jem::util::ArrayBuffer;
using jive::IdxVector;
using jive::Properties;
using jive::String;
using jive::StringVector;
using jive::Vector;
using jive::algebra::AbstractMatrix;
using jive::algebra::MatrixBuilder;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::ModelFactory;
using jive::model::StateVector;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::Globdat;
using jive::util::XTable;

/// @brief Model for managing lattice structures composed of rod elements
/// @details Coordinates multiple child models representing rod elements and handles
/// contact interactions between rods and joints. Provides utilities for computing
/// global properties like kinetic energy and total mass.
class LatticeModel : public Model
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;          ///< Model type name
  static const char *CHILD_PROPS;        ///< Child model properties
  static const char *ROD_CONTACT_PROP;   ///< Rod contact model property
  static const char *JOINT_CONTACT_PROP; ///< Joint contact model property
  static const char *ROD_LIST_PROP;      ///< Rod list property
  static const char *NAME_PREFIX;        ///< Child name prefix
  /// @}

  JEM_DECLARE_CLASS(LatticeModel, Model);

  /// @brief Constructor with configuration and properties
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit LatticeModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Handle model actions for lattice coordination
  /// @param action Action name to execute
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat);

  /// @brief Calculate total kinetic energy of the lattice
  /// @param globdat Global data container
  /// @return Total kinetic energy
  double getKineticEnergy(const Properties &globdat) const;

  /// @brief Calculate kinetic energy and store in table
  /// @param energy_table Output energy table
  /// @param table_weights Table weights
  /// @param globdat Global data container
  void getKineticEnergy(XTable &energy_table, const Vector &table_weights, const Properties &globdat) const;

  /// @brief Calculate total mass of the lattice
  /// @param globdat Global data container
  /// @return Total mass
  double getMass(const Properties &globdat) const;

  /// @brief Calculate mass and store in table
  /// @param mass_table Output mass table
  /// @param table_weights Table weights
  /// @param globdat Global data container
  void getMass(XTable &mass_table, const Vector &table_weights, const Properties &globdat) const;

  /// @brief Factory method for creating new LatticeModel instances
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return Reference to new LatticeModel instance
  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  /// @brief Register LatticeModel type with ModelFactory
  static void declare();

private:
  /// @name Child models
  /// @{
  Array<Ref<Model>> children_; ///< Array of child rod models
  Ref<Model> contact_;         ///< Rod contact model
  Ref<Model> jointContact_;    ///< Joint contact model
  /// @}

  /// @name System matrices
  /// @{
  Ref<AbstractMatrix> M_; ///< Mass matrix
  /// @}
};
