#pragma once

#include <jem/base/ClassTemplate.h>
#include <jem/base/Array.h>
#include <jem/base/System.h>
#include <jem/base/ArithmeticException.h>
#include <jive/algebra/AbstractMatrix.h>
#include <jive/algebra/DiagMatrixObject.h>
#include <jive/app/Module.h>
#include <jive/solver/Solver.h>
#include <jive/implict/Names.h>
#include <jem/base/array/operators.h>
#include <jem/util/Event.h>
#include <jem/util/Properties.h>
#include <jive/util/Globdat.h>
#include <jive/util/DofSpace.h>
#include <jive/util/Constraints.h>
#include <jive/util/FuncUtils.h>
#include <jive/app/ModuleFactory.h>
#include <jive/implict/utilities.h>
#include <jive/model/Model.h>
#include <jive/model/Actions.h>
#include <jive/model/StateVector.h>
#include <jive/solver/declare.h>
#include <jive/solver/utilities.h>
#include <jive/solver/SolverParams.h>

using jem::newInstance;
using jem::NIL;
using jem::idx_t;
using jem::numeric::Function;

using jive::Vector;
using jive::Ref;
using jive::Properties;
using jive::String;
using jive::algebra::AbstractMatrix;
using jive::algebra::DiagMatrixObject;
using jive::app::Module;
using jive::model::Model;
using jive::util::DofSpace;
using jive::solver::Solver;
using jive::solver::newSolver;
using jive::util::Constraints;
using jive::util::Globdat;
using jive::util::FuncUtils;
using jive::model::Actions;
using jive::model::ActionParams;
using jive::model::StateVector;
using jive::implict::PropNames;
using jive::implict::newSolverParams;

//-----------------------------------------------------------------------
//   class EulerForwardModule
//-----------------------------------------------------------------------


class EulerForwardModule : public Module
{
 public:

  enum MassMode { LUMPED, CONSISTENT };

  typedef Module            Super;
  typedef EulerForwardModule    Self;

  static const char*        TYPE_NAME;

  explicit                  EulerForwardModule

    ( const String&           name = "EulerForward" );

  virtual Status            init

    ( const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  virtual Status            run

    ( const Properties&       globdat );

  virtual void              shutdown

    ( const Properties&       globdat );

  virtual void              configure

    ( const Properties&       props,
      const Properties&       globdat );

  virtual void              getConfig

    ( const Properties&       props,
      const Properties&       globdat )        const;

  static Ref<Module>        makeNew

    ( const String&           name,
      const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  static void               declare ();

 protected:

  virtual                  ~EulerForwardModule  ();


 private:

  void                      restart_

    ( const Properties&       globdat );

  void                      invalidate_     ();


 private:

  bool          valid_;
  double        dtime_;
  MassMode      mode_;
  Ref<Model>    model_;
  Ref<DofSpace> dofs_;

  Vector        massInv_;
  Ref<Solver>   solver_;

  Ref<Function> updCond_;
};
