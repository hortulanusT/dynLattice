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
#include <jive/implict/SolverInfo.h>
#include <jive/implict/SolverModule.h>
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
using jive::implict::SolverInfo;
using jive::implict::SolverModule;
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

class ExplicitModule : public SolverModule
{
public:
  enum MassMode
  {
    LUMPED,
    CONSISTENT
  };

  JEM_DECLARE_CLASS(ExplicitModule, SolverModule);

  static const char *TYPE_NAME;
  static const char *STEP_COUNT;
  static const char *SO3_DOFS;
  static const char *REPORT_ENERGY;

  virtual Status init

    (const Properties& conf,
     const Properties& props,
     const Properties& globdat) override;

  virtual void shutdown

      (const Properties &globdat);

  virtual void configure

    (const Properties& props, const Properties& globdat) override;

  virtual void getConfig

    (const Properties& props, const Properties& globdat) const override;

  virtual void advance

    (const Properties& globdat) override;

  virtual void solve

    (const Properties& info, const Properties& globdat) = 0;

  virtual void cancel

    (const Properties& globdat) override;

  /// @brief comupte the next step size
  /// @return whether this step can be accepted
  virtual bool commit

    (const Properties& globdat) override;

  virtual void setPrecision

    (double eps) override;

  virtual double getPrecision() const override;

  // static Ref<Module> makeNew

  //     (const String &name, const Properties &conf,
  //      const Properties &props, const Properties &globdat);

  // static void declare();

protected:
  explicit ExplicitModule(const String& name = "");

  virtual ~ExplicitModule();

  void updMass(const Properties& globdat);

  void invalidate();

  void store_energy(const Properties& globdat);

  /// @brief Adams Bashforth 2 step update
  inline void ABupdate(const Vector& delta_y,
                       const Vector& f_cur,
                       const Vector& f_old) const;
  /// @brief Adams Bashforth 1 step update (Euler Explicit)
  inline void ABupdate(const Vector& delta_y, const Vector& f_cur) const;

  /// @brief update of the displacement vectors optionally taking SO(3)
  /// into account
  void updateVec(const Vector& y_new,
                 const Vector& y_old,
                 const Vector& delta_y,
                 const bool rot = false);

  /// @brief get the accelration (and return the resulting force Vector)
  void getAcce(const Vector& a,
               const Ref<Constraints>& cons,
               const Vector& fres,
               const Properties& globdat);

  /// @brief get the forces
  /// @return resulting forces = external - internal
  Vector getForce(const Vector& fint,
                  const Vector& fext,
                  const Properties& globdat);

  inline double getQuality(const Vector& y_pre, const Vector& y_cor);

protected:
  bool valid_;
  bool report_energy_;

  double dtime_;
  double prec_;
  double minDtime_;
  double maxDtime_;
  double saftey_;
  double incrFact_;
  double decrFact_;

  MassMode mode_;
  idx_t order_;

  Ref<Function> updCond_;
  Vector massInv_;
  IdxVector SO3_dofs_;
  IdxMatrix rdofs_;

  Ref<Model> model_;
  Ref<DofSpace> dofs_;
  Ref<Constraints> cons_;
  Ref<Solver> solver_;
};

inline void
ExplicitModule::ABupdate(const Vector& delta_y,
                         const Vector& f_cur,
                         const Vector& f_old) const
{
  delta_y = dtime_ / 2. * (3. * f_cur - 1. * f_old);
}

inline void
ExplicitModule::ABupdate(const Vector& delta_y, const Vector& f_cur) const
{
  delta_y = dtime_ * f_cur;
}

inline double
ExplicitModule::getQuality(const Vector& y_pre, const Vector& y_cor)
{
  return sqrt(norm2(Vector(y_pre - y_cor)) / (double)y_pre.size());
}