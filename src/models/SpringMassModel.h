/**
 * @file SpringMassModel.h
 * @author Til Gärtner
 * @brief Emulate singular beams at boundaries to model spring-mass systems
 *
 *
 */

#pragma once

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

class SpringMassModel : public Model
{
public:
  JEM_DECLARE_CLASS(SpringMassModel, Model);

  static const char *TYPE_NAME;
  static const char *BOUNDARY_PROP;
  static const char *SPRING_NAMES;
  static const char *EXTENT_VECTOR_PROP;
  static const char *N_ELEM_PROP;
  static const char *P_ELEM_PROP;

  explicit SpringMassModel(
      const String &name,
      const Properties &conf,
      const Properties &props,
      const Properties &globdat);

  virtual bool takeAction(
      const String &action,
      const Properties &params,
      const Properties &globdat);

  static Ref<Model> makeNew(
      const String &name,
      const Properties &conf,
      const Properties &props,
      const Properties &globdat);

  static void declare();

protected:
  virtual ~SpringMassModel();

  virtual bool applyConstraints_(
      const Properties &globdat) const;

private:
  Array<Ref<Model>> children_;
  Assignable<NodeGroup> boundary_;
  IdxMatrix edgeNodes_;
};
