/**
 * @file LatticeModel.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief LatticeModel for collection of rods
 * @version 0.1
 * @date 2022-04-26
 *
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 *
 */

#pragma once

#include <jem/base/CString.h>
#include <jem/base/IllegalInputException.h>
#include <jem/base/System.h>
#include <jem/base/array/Array.h>
#include <jem/numeric/algebra/utilities.h>
#include <jem/util/ArrayBuffer.h>
#include <jem/util/Properties.h>
#include <jem/base/Array.h>
#include <jive/algebra/AbstractMatrix.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/util/XTable.h>
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

class LatticeModel : public Model
{
public:
  static const char *TYPE_NAME;
  static const char *CHILD_PROPS;
  static const char *ROD_CONTACT_PROP;
  static const char *JOINT_CONTACT_PROP;
  static const char *ROD_LIST_PROP;
  static const char *NAME_PREFIX;

  explicit LatticeModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat);

  /**
   * Calculates the kinetic energy of the special Cosserat rod model.
   *
   * @param globdat The global data.
   * @return The kinetic energy.
   */
  double calc_kin_Energy_(const Properties &globdat) const;
  void calc_kin_Energy_(XTable &energy_table, const Vector &table_weights, const Properties &globdat) const;

  /**
   * Calculates the mass of the special cosserat rod model
   *
   * @param globdat the global data
   * @return the total mass
   */
  double calc_mass_(const Properties &globdat) const;
  void calc_mass_(XTable &mass_table, const Vector &table_weights, const Properties &globdat) const;

  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  static void declare();

private:
  Array<Ref<Model>> children_;
  Ref<Model> contact_;
  Ref<Model> jointContact_;

  Ref<AbstractMatrix> M_;
};
