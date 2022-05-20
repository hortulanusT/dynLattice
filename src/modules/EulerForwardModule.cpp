
#include "modules/EulerForwardModule.h"


//=======================================================================
//   class EulerForwardModule
//=======================================================================

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------


EulerForwardModule::EulerForwardModule 

  ( const String&      name ) :

  Super ( name )

{
  dtime_  =  1.0;
}


EulerForwardModule::~EulerForwardModule ()
{}


//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------


Module::Status EulerForwardModule::init

  ( const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  using jem::util::connect;

  Properties    myConf  = conf .makeProps ( myName_ );
  Properties    myProps = props.findProps ( myName_ );
  Properties    params;
  Ref<AbstractMatrix> inertia;

  // returning an address of an object pointing to the Model

  model_ = Model::get ( globdat, getContext() );

  // returning an address of an object pointing to class DofSpace

  dofs_  = DofSpace::get ( globdat, getContext() );
 
  // Invalidate the state of this module when the DofSpace changes.

  connect ( dofs_->newSizeEvent,  this, &Self::invalidate_ );
  connect ( dofs_->newOrderEvent, this, &Self::invalidate_ );
  valid_ = false;

  // Initialize solver

  jive::solver::declareSolvers();

  Ref<Constraints> cons = Constraints::find ( dofs_, globdat );

  params.set ( ActionParams::CONSTRAINTS, cons );

  model_->takeAction ( Actions::NEW_MATRIX2, params, globdat );

  params.get ( inertia, ActionParams::MATRIX2 );

  Properties sparams = newSolverParams ( globdat, inertia, nullptr, dofs_ );

  model_->takeAction ( Actions::GET_SOLVER_PARAMS, sparams, globdat );

  solver_ = newSolver ( "solver", myConf, myProps, sparams, globdat );

  // Initialize the global simulation time and the time step number.
  Globdat::initTime( globdat );
  Globdat::initStep( globdat );

  return OK;
}


//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------


Module::Status EulerForwardModule::run 

  ( const Properties& globdat )

{
  const idx_t dofCount = dofs_->dofCount();

  using jive::model::STATE;

  Properties    params;

  if ( model_ == NIL )
    return DONE;

  // update time in models (for boundary conditions)

  Globdat::advanceTime( dtime_, globdat );
  Globdat::advanceStep( globdat );
  model_->takeAction ( Actions::ADVANCE, params, globdat );

  // update mass matrix if necessary 
  // solver_->getMatrix()->printTo( jive::util::Printer::get() );
  // jive::util::Printer::flush();

  // TEST_CONTEXT( solver_->getMatrix()->getType()->getName() )
  if ( ! valid_ ) 
    restart_ ( globdat );

  // solver_->getMatrix()->printTo( jive::util::Printer::get() );
  // jive::util::Printer::flush();

  // initialize some variables

  Ref<Constraints>  cons;

  Vector        fint     ( dofCount );
  Vector        fext     ( dofCount );
  Vector        fres     ( dofCount );
  Vector        u        ( dofCount );
  Vector        du       ( dofCount );
  Vector        v        ( dofCount );
  Vector        a        ( dofCount );

  Vector        u_1, v_1, a_1;

  // Get the internal and external force vectors.
  model_->takeAction ( Actions::GET_CONSTRAINTS, params, globdat );

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
  StateVector::get    ( v_1, STATE[1], dofs_, globdat );
  StateVector::get    ( a_1, STATE[2], dofs_, globdat );

  a = 0.0;
  v = 0.0;
  du = 0.0;
  u = 0.0;
  // Compute new displacement values 
  // jive::Matrix F ( dofs_->typeCount(), dofCount_/dofs_->typeCount() );
  // vec2mat( F.transpose(), fres );
  // TEST_CONTEXT(F)

  solver_->solve ( a, fres );

  v = v_1 + a * dtime_;
  du = v; // HACK to proper SO(3) conversion
  u = u_1 + du * dtime_; 

  // jive::Matrix A ( dofs_->typeCount(), dofCount_/dofs_->typeCount() );
  // vec2mat( A.transpose(), a );
  // TEST_CONTEXT(A)
  // jive::Matrix V ( dofs_->typeCount(), dofCount_/dofs_->typeCount() );
  // vec2mat( V.transpose(), v );
  // TEST_CONTEXT(V)
  // jive::Matrix U ( dofs_->typeCount(), dofCount_/dofs_->typeCount() );
  // vec2mat( U.transpose(), u );
  // TEST_CONTEXT(U)

  // Store                
  StateVector::updateOld( dofs_, globdat );
  a_1 = a;
  v_1 = v;
  u_1 = u;

  // commit everything
  Globdat::commitStep( globdat );
  Globdat::commitTime( globdat );
  model_->takeAction ( Actions::COMMIT, params, globdat ); 

  return OK;
}


//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------


void EulerForwardModule::shutdown ( const Properties& globdat )
{
  model_ = NIL;
}


//-----------------------------------------------------------------------
//   configure
//-----------------------------------------------------------------------


void EulerForwardModule::configure

  ( const Properties&  props,
    const Properties&  globdat )

{
  Properties  myProps = props.findProps ( myName_ );

  myProps.find ( dtime_, jive::implict::PropNames::DELTA_TIME, 1.0e-20, 1.0e20 );
}


//-----------------------------------------------------------------------
//   getConfig
//-----------------------------------------------------------------------


void EulerForwardModule::getConfig

  ( const Properties&  conf,
    const Properties&  globdat ) const

{
  Properties  myConf = conf.makeProps ( myName_ );

  myConf.set ( jive::implict::PropNames::DELTA_TIME, dtime_ );
}


//-----------------------------------------------------------------------
//   restart_
//-----------------------------------------------------------------------


void EulerForwardModule::restart_ ( const Properties& globdat )
{
  Properties    params;

  model_->takeAction ( Actions::UPD_MATRIX2, params, globdat );
}


//-----------------------------------------------------------------------
//   invalidate_
//-----------------------------------------------------------------------


void EulerForwardModule::invalidate_ ()
{
  valid_ = false;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module>  EulerForwardModule::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  return newInstance<Self> ( name );
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void EulerForwardModule::declare ()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare ( "EulerForward", &EulerForwardModule::makeNew );
}

