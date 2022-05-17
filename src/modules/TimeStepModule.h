#pragma once

#include <jive/algebra/AbstractMatrix.h>
#include <jive/app/Module.h>
#include <jive/util/DofSpace.h>
#include <jive/solver/Solver.h>
#include <jive/implict/Names.h>

using jive::Vector;
using jive::Ref;
using jive::Properties;
using jive::String;
using jive::algebra::AbstractMatrix;
using jive::app::Module;
using jive::model::Model;
using jive::util::DofSpace;
using jive::solver::Solver;

//-----------------------------------------------------------------------
//   class TimeStepModule
//-----------------------------------------------------------------------


class TimeStepModule : public Module
{
 public:

  typedef Module            Super;
  typedef TimeStepModule    Self;

  explicit                  TimeStepModule

    ( const String&           name = "step" );

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

  virtual                  ~TimeStepModule  ();


 private:

  void                      restart_

    ( const Properties&       globdat );

  void                      invalidate_     ();


 private:

  bool                      lumped_;
  Ref<Model>                model_;
  Ref<DofSpace>             dofs_;
  Vector                    rmass_;
  double                    dtime_;
  int                       istep_;
  int                       dofCount_;
  int                       interval_;

  Ref<Solver>               solver_;
  Ref<AbstractMatrix>       matrix_;
};
