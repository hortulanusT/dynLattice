#include <jem/base/System.h>
#include <jem/base/limits.h>
#include <jem/base/array/operators.h>
#include <jem/base/ArithmeticException.h>
#include <jem/util/Event.h>
#include <jem/util/Properties.h>
#include <jive/util/Globdat.h>
#include <jive/util/ItemSet.h>
#include <jive/util/DofSpace.h>
#include <jive/fem/ElementSet.h>
#include <jive/util/Constraints.h>
#include <jive/algebra/DiagMatrixObject.h>
#include <jive/algebra/LumpedMatrixBuilder.h>
#include <jive/algebra/SparseMatrixBuilder.h>
#include <jive/app/ModuleFactory.h>
#include <jive/fem/FEMatrixBuilder.h>
#include <jive/implict/utilities.h>
#include <jive/model/Model.h>
#include <jive/model/Actions.h>
#include <jive/model/StateVector.h>
#include <jive/solver/declare.h>
#include <jive/solver/utilities.h>
#include <jive/solver/SolverParams.h>

#include "TimeStepModule.h"
#include "utils/testing.h"
#include "utils/helpers.h"

using namespace jive_helpers;

using jem::newInstance;
using jem::NIL;
using jem::io::endl;
using jem::idx_t;

using jive::util::Constraints;
using jive::util::Globdat;
using jive::model::Actions;
using jive::model::ActionParams;
using jive::model::StateVector;
using jive::implict::newSolverParams;
using jive::solver::SolverParams;
using jive::solver::newSolver;


//=======================================================================
//   class TimeStepModule
//=======================================================================

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------


TimeStepModule::TimeStepModule 

  ( const String&      name ) :

  Super ( name )

{
  dtime_  =  1.0;
  istep_  = -1;
  interval_ = 0;
}


TimeStepModule::~TimeStepModule ()
{}


//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------


Module::Status TimeStepModule::init

  ( const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  using jem::util::connect;

  const String  context = getContext ();

  double        t;
  int           i;

  Properties  myConf  = conf .makeProps ( myName_ );
  Properties  myProps = props.findProps ( myName_ );

  // returning an address of an object pointing to the Model

  model_ = Model::get ( globdat, context );

  // returning an address of an object pointing to class DofSpace

  dofs_  = DofSpace::get ( globdat, context );
 
  istep_ = -1;

  // Invalidate the state of this module when the DofSpace changes.

  connect ( dofs_->newSizeEvent,  this, &Self::invalidate_ );
  connect ( dofs_->newOrderEvent, this, &Self::invalidate_ );

  // Initialize solver

  Properties params;

  jive::solver::declareSolvers();

  Ref<Constraints> cons = Constraints::find ( dofs_, globdat );

  params.set ( ActionParams::CONSTRAINTS, cons );

  model_->takeAction ( Actions::NEW_MATRIX2, params, globdat );

  params.get ( matrix_, ActionParams::MATRIX2 );

  Properties sparams = newSolverParams ( globdat, matrix_, nullptr, dofs_ );

  model_->takeAction ( Actions::GET_SOLVER_PARAMS, sparams, globdat );

  solver_ = newSolver ( "solver", myConf, myProps, sparams, globdat );

  // Initialize the global simulation time and the time step number.

  i = 0;
  t = 0.0;

  if ( ! globdat.find( t, Globdat::TIME ) )
  {
    globdat.set ( Globdat::TIME, t );
  }

  if ( ! globdat.find( i, Globdat::TIME_STEP ) )
  {
    globdat.set ( Globdat::TIME_STEP, i );
  }

  globdat.set ( Globdat::OLD_TIME,      t );
  globdat.set ( Globdat::OLD_TIME_STEP, i );

  return OK;
}


//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------


Module::Status TimeStepModule::run 

  ( const Properties& globdat )

{
  using jive::util::setSlaveDofs;
  using jive::model::STATE;

  Properties    params;

  if ( model_ == NIL )
  {
    return DONE;
  }

  // update time in models (for boundary conditions)

  model_->takeAction ( Actions::ADVANCE, params, globdat );

  // update mass matrix if necessary 

  if ( istep_ < 0 )
  {
    restart_ ( globdat );
  }

  // initialize some variables

  Ref<Constraints>  cons;

  Vector        fint     ( dofCount_ );
  Vector        fext     ( dofCount_ );
  Vector        fres     ( dofCount_ );
  Vector        u        ( dofCount_ );
  Vector        du       ( dofCount_ );
  Vector        v        ( dofCount_ );
  Vector        a        ( dofCount_ );

  Vector        u_1, u_2, v_1, v_2, a_1, a_2;

  double        t;
  int           i;

  globdat.get ( t, Globdat::TIME );
  globdat.get ( i, Globdat::TIME_STEP );

  // Get the internal and external force vectors.

  fint = 0.0;
  fext = 0.0;

  params.set ( ActionParams::INT_VECTOR, fint );
  params.set ( ActionParams::EXT_VECTOR, fext );

  model_->takeAction ( Actions::GET_INT_VECTOR, params, globdat );
  model_->takeAction ( "GET_GYRO_VECTOR", params, globdat );
  model_->takeAction ( Actions::GET_EXT_VECTOR, params, globdat );

  fres = fext - fint;

  // Get the state vectors and compute the state vector at the next
  // time step.

  StateVector::get    ( u_1, STATE[0], dofs_, globdat );
  StateVector::getOld ( u_2,  STATE[0], dofs_, globdat );
  StateVector::get    ( v_1, STATE[1], dofs_, globdat );
  StateVector::getOld ( v_2,  STATE[1], dofs_, globdat );
  StateVector::get    ( a_1, STATE[2], dofs_, globdat );
  StateVector::getOld ( a_2,  STATE[2], dofs_, globdat );

  // update constraints
  
  cons = Constraints::find ( dofs_, globdat );

  if ( cons != NIL )
  {
    params.set ( ActionParams::CONSTRAINTS, cons );

    model_->takeAction ( Actions::GET_CONSTRAINTS, params, globdat );
  }

  a = 0.0;
  v = 0.0;
  du = 0.0;
  u = 0.0;
  // Compute new displacement values 
  jive::Matrix F ( dofs_->typeCount(), dofCount_/dofs_->typeCount() );
  vec2mat( F.transpose(), fres );
  TEST_CONTEXT(F)

  solver_->solve ( a, fres );

  v = v_1 + a * dtime_;
  du = v; // HACK to proper SO(3) conversion
  u = u_1 + du * dtime_;

  jive::Matrix A ( dofs_->typeCount(), dofCount_/dofs_->typeCount() );
  vec2mat( A.transpose(), a );
  TEST_CONTEXT(A)
  jive::Matrix V ( dofs_->typeCount(), dofCount_/dofs_->typeCount() );
  vec2mat( V.transpose(), v );
  TEST_CONTEXT(V)
  jive::Matrix U ( dofs_->typeCount(), dofCount_/dofs_->typeCount() );
  vec2mat( U.transpose(), u );
  TEST_CONTEXT(U)

  // Store
  a_2 = a_1;
  a_1 = a;

  v_2 = v_1;
  v_1 = v;

  u_2 = u_1;
  u_1 = u;
                
  // Update the simulation time and time step.

  globdat.set ( Globdat::OLD_TIME,      t );
  globdat.set ( Globdat::OLD_TIME_STEP, i );
  globdat.set ( Globdat::TIME,          t + dtime_ );
  globdat.set ( Globdat::TIME_STEP,     i + 1 );

  if ( istep_++ > interval_ )
  {
    invalidate_();
  }

  model_->takeAction ( Actions::COMMIT, params, globdat ); 

  return OK;
}


//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------


void TimeStepModule::shutdown ( const Properties& globdat )
{
  model_ = NIL;
}


//-----------------------------------------------------------------------
//   configure
//-----------------------------------------------------------------------


void TimeStepModule::configure

  ( const Properties&  props,
    const Properties&  globdat )

{
  Properties  myProps = props.findProps ( myName_ );

  myProps.find ( interval_, "dtAssembly", 0, 10000 );
  myProps.find ( dtime_, jive::implict::PropNames::DELTA_TIME, 1.0e-20, 1.0e20 );
}


//-----------------------------------------------------------------------
//   getConfig
//-----------------------------------------------------------------------


void TimeStepModule::getConfig

  ( const Properties&  conf,
    const Properties&  globdat ) const

{
  Properties  myConf = conf.makeProps ( myName_ );

  myConf.set ( jive::implict::PropNames::DELTA_TIME, dtime_ );
  myConf.set ( "dtAssembly", interval_ );
}


//-----------------------------------------------------------------------
//   restart_
//-----------------------------------------------------------------------


void TimeStepModule::restart_ ( const Properties& globdat )
{
  using jive::algebra::LumpedMatrixBuilder;
  using jive::algebra::SparseMatrixBuilder;
  using jive::algebra::MatrixBuilder;
  using jive::model::STATE0;
  using jive::model::STATE1;
  using jive::model::STATE2;

  const String  context  = getContext     ();

  dofCount_ = dofs_->dofCount();

  Properties    params;

  Vector        u0, u1, a0, v0;

  StateVector::get    ( u0, STATE0, dofs_, globdat );
  StateVector::getOld ( u1, STATE0, dofs_, globdat );
  StateVector::get    ( v0, STATE1, dofs_, globdat );
  StateVector::get    ( a0, STATE2, dofs_, globdat );

  Vector        fext ( dofCount_ );

  fext = 0.0;

  params.set ( ActionParams::EXT_VECTOR, fext );

  model_->takeAction ( Actions::GET_EXT_VECTOR, params, globdat );

  Ref<SparseMatrixBuilder> mbuilder;
  
  mbuilder = newInstance<SparseMatrixBuilder> ();

  // mbuilder->setOptions ( 0 );
  // mbuilder->setSize    ( dofCount_ );
  // mbuilder->clear      ();

  params.set ( ActionParams::MATRIX2, mbuilder );

  model_->takeAction ( Actions::UPD_MATRIX2, params, globdat );

  matrix_ = mbuilder->getMatrix();
}


//-----------------------------------------------------------------------
//   invalidate_
//-----------------------------------------------------------------------


void TimeStepModule::invalidate_ ()
{
  istep_ = -1;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module>  TimeStepModule::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  return newInstance<Self> ( name );
}

void TimeStepModule::declare ()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare ( "Explicit", &TimeStepModule::makeNew );
}

