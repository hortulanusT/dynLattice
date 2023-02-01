#pragma once

#include "utils/helpers.h"
#include <jem/base/ArithmeticException.h>
#include <jem/base/Array.h>
#include <jem/base/ClassTemplate.h>
#include <jem/base/System.h>
#include <jem/base/array/operators.h>
#include <jem/util/Event.h>
#include <jem/util/Properties.h>
#include <jem/util/PropertyException.h>
#include <jive/algebra/AbstractMatrix.h>
#include <jive/algebra/DiagMatrixObject.h>
#include <jive/algebra/FlexMatrixBuilder.h>
#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/implict/Names.h>
#include <jive/implict/utilities.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/StateVector.h>
#include <jive/solver/Solver.h>
#include <jive/solver/SolverParams.h>
#include <jive/solver/declare.h>
#include <jive/solver/utilities.h>
#include <jive/util/Constraints.h>
#include <jive/util/DenseTable.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/Globdat.h>
#include <jive/util/ItemSet.h>
#include <jive/util/XTable.h>

using jem::idx_t;
using jem::newInstance;
using jem::numeric::Function;

using jive::IdxMatrix;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::StringVector;
using jive::Vector;
using jive::algebra::AbstractMatrix;
using jive::algebra::DiagMatrixObject;
using jive::algebra::FlexMatrixBuilder;
using jive::app::Module;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::implict::newSolverParams;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::solver::newSolver;
using jive::solver::Solver;
using jive::util::Constraints;
using jive::util::DenseTable;
using jive::util::DofSpace;
using jive::util::FuncUtils;
using jive::util::Globdat;
using jive::util::ItemSet;
using jive::util::XTable;

using jive_helpers::expVec;
using jive_helpers::logMat;

//-----------------------------------------------------------------------
//   class ExplicitModule
//-----------------------------------------------------------------------

class ExplicitModule : public Module
{
public:
  enum MassMode
  {
    LUMPED,
    CONSISTENT
  };

  typedef Module Super;
  typedef ExplicitModule Self;

  static const char *TYPE_NAME;
  static const char *STEP_COUNT;
  static const char *SO3_DOFS;
  static const char *REPORT_ENERGY;

  explicit ExplicitModule

      (const String &name = "Explicit");

  virtual Status init

      (const Properties &conf, const Properties &props,
       const Properties &globdat);

  virtual Status run

      (const Properties &globdat);

  virtual void shutdown

      (const Properties &globdat);

  virtual void configure

      (const Properties &props, const Properties &globdat);

  virtual void getConfig

      (const Properties &props, const Properties &globdat) const;

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

protected:
  virtual ~ExplicitModule();

protected:
  idx_t advance_(const Properties &globdat);

  void restart_(const Properties &globdat);

  void invalidate_();

  void store_energy_(const Properties &globdat);

  /// @brief Adams Bashforth 2 step update
  inline void ABupdate_(const Vector &delta_y, const Vector &f_cur,
                        const Vector &f_old) const;
  /// @brief Adams Bashforth 1 step update (Euler Explicit)
  inline void ABupdate_(const Vector &delta_y, const Vector &f_cur) const;

  /// @brief update of the displacement vectors optionally taking SO(3)
  /// into account
  void updateVec_(const Vector &y_new, const Vector &y_old,
                  const Vector &delta_y, const bool rot = false);
  /// @brief get the accelration (and return the internal force Vector)
  void getAcce_(const Vector &a, const Ref<Constraints> &cons,
                const Vector &fres, const Properties &globdat);

  /// @brief get the forces
  /// @return resulting forces = external - internal
  Vector getForce_(const Vector &fint, const Vector &fext,
                   const Properties &globdat);

protected:
  bool valid_;
  bool report_energy_;
  double dtime_;
  double prec_;
  double minDtime_;
  double maxDtime_;
  idx_t stepCount_;
  MassMode mode_;

  Ref<Function> updCond_;
  IdxVector SO3_dofs_;
  IdxMatrix rdofs_;

  Ref<Model> model_;
  Ref<DofSpace> dofs_;
  Ref<Constraints> cons_;

  Vector massInv_;
  Ref<Solver> solver_;
};

inline void ExplicitModule::ABupdate_(const Vector &delta_y,
                                      const Vector &f_cur,
                                      const Vector &f_old) const
{
  delta_y = dtime_ / 2. * (3. * f_cur - 1. * f_old);
}

inline void ExplicitModule::ABupdate_(const Vector &delta_y,
                                      const Vector &f_cur) const
{
  delta_y = dtime_ * f_cur;
}
