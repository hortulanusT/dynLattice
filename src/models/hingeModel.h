/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *
 * This class implements a joint for several rods
 * (should be used with the specialCosseratRodModel)
 *
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: July 21
 *
 */

#pragma once

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
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/StateVector.h>
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

using namespace jive_helpers;

class hingeModel : public Model
{
public:
  static const char *TYPE_NAME;
  static const char *YIELD_PROP;

  explicit hingeModel

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  virtual bool takeAction

      (const String &action,
       const Properties &params,
       const Properties &globdat);

  static Ref<Model> makeNew

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  static void declare();

private:
  /**
   * @brief creates the elements for the hinges
   *
   * @param elementName name of the Element group
   * @param globdat global database
   *
   * @return created element Group
   */
  ElementGroup createHinges_(const String &elementName, const Properties &globdat);

  /**
   * @brief stores the forces relevant for the existing hinges
   *
   * @param fint global internal force vector
   */
  void updForces_(const Vector &fint);

  /**
   * @brief evaluate the plastic development in the beams
   *
   * @return true if not new plasticity was discovered
   * @return false if some plastic movement is discovered
   */
  bool evalPlastic_(const Vector &disp);

  void init_(const Properties &globdat);

  void getCons_();

private:
  Assignable<ElementGroup> egroup_;
  Assignable<NodeSet> nodes_;
  Assignable<ElementSet> elems_;

  Ref<DofSpace> dofs_;
  Ref<Constraints> cons_;
  IdxVector jtypes_;
  StringVector jnames_;

  Ref<Material> material_;
  Ref<Function> yieldCond_;
  Vector ell_;
  Matrix intForces_;
  Matrix intForcesOld_;
  Matrix plasticDisp_;
};
