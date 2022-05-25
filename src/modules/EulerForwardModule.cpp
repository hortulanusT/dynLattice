
#include "modules/EulerForwardModule.h"
#include "utils/testing.h"

//=======================================================================
//   class EulerForwardModule
//=======================================================================

const char* EulerForwardModule::TYPE_NAME = "EulerForward";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

EulerForwardModule::EulerForwardModule 

  ( const String&      name ) :

  Super ( name )

{
  dtime_  = 1.0;
  valid_  = false;
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
  using jive::implict::PropNames;

  Properties            myConf  = conf .makeProps ( myName_ );
  Properties            myProps = props.findProps ( myName_ );
  Properties            params;
  Properties            sparams;
  Ref<AbstractMatrix>   inertia;
  Ref<DiagMatrixObject> diagInertia;

  // returning an address of an object pointing to the Model

  model_ = Model::get ( globdat, getContext() );

  // returning an address of an object pointing to class DofSpace

  dofs_  = DofSpace::get ( globdat, getContext() );
 
  // Invalidate the state of this module when the DofSpace changes.

  connect ( dofs_->newSizeEvent,  this, &Self::invalidate_ );
  connect ( dofs_->newOrderEvent, this, &Self::invalidate_ );

  // Initialize update condition

  if ( myProps.contains( PropNames::UPDATE_COND) )
    FuncUtils::configCond( updCond_, PropNames::UPDATE_COND, myProps, globdat );
  else
    updCond_ = FuncUtils::newCond( false );
  FuncUtils::getConfig( myConf, updCond_, PropNames::UPDATE_COND );

  // Initialize solver
  
  jive::solver::declareSolvers();

  Ref<Constraints> cons = Constraints::find ( dofs_, globdat );
  params.set ( ActionParams::CONSTRAINTS, cons );
  model_->takeAction ( Actions::NEW_MATRIX2, params, globdat );
  params.get ( inertia, ActionParams::MATRIX2 );
  params.clear();

  diagInertia = jem::dynamicCast<DiagMatrixObject>( inertia );

  if (diagInertia)
    mode_ = LUMPED;
  else
    mode_ = CONSISTENT;

  // prepare solver for modes
  if (mode_ == LUMPED)
  {
    massInv_.resize( dofs_->dofCount() );
    Globdat::storeFor( "LumpedMass", diagInertia, this, globdat );
  
    myConf.set( "mode", "lumped" );
  }
  
  if (mode_ == CONSISTENT)
  {
    sparams = newSolverParams ( globdat, inertia, nullptr, dofs_ );
    model_->takeAction ( Actions::GET_SOLVER_PARAMS, sparams, globdat );
    solver_ = newSolver ( "solver", myConf, myProps, sparams, globdat );
    solver_->configure( myProps );
    solver_->getConfig( myConf );

    myConf.set( "mode", "consistent" );
  }

  valid_ = false;

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
  using jive::model::STATE;
  const idx_t dofCount = dofs_->dofCount();

  Properties            params;
  Ref<Constraints>      cons = Constraints::find ( dofs_, globdat );
  Vector      fint     ( dofCount );
  Vector      fext     ( dofCount );
  Vector      fres     ( dofCount );
  Vector      u_new    ( dofCount );
  Vector      du       ( dofCount );
  Vector      v_new    ( dofCount );
  Vector      a_new    ( dofCount );
  Vector      u_old, v_old, a_old;

  // skip if no model exists
  if ( model_ == NIL )
    return DONE;

  // update mass matrix if necessary 
  if ( ! valid_ )
    restart_ ( globdat );

  // Get the state vectors from the last time step
  StateVector::get  ( u_old, STATE[0], dofs_, globdat );
  StateVector::get  ( v_old, STATE[1], dofs_, globdat );
  StateVector::get  ( a_old, STATE[2], dofs_, globdat );

  // Get the internal and external force vectors last time step
  fint = 0.0;
  fext = 0.0;

  params.set ( ActionParams::INT_VECTOR, fint );
  params.set ( ActionParams::EXT_VECTOR, fext );

  model_->takeAction ( Actions::GET_INT_VECTOR, params, globdat );
  model_->takeAction ( "GET_GYRO_VECTOR", params, globdat );
  model_->takeAction ( Actions::GET_EXT_VECTOR, params, globdat );

  fres = fext - fint;
  params.clear();

  // update time in models and boundary conditions
  params.set ( ActionParams::CONSTRAINTS, cons );
  Globdat::advanceTime( dtime_, globdat );
  Globdat::advanceStep( globdat );
  model_->takeAction ( Actions::ADVANCE, params, globdat );
  model_->takeAction ( Actions::GET_CONSTRAINTS, params, globdat );

  // Compute new displacement values 
  if (mode_ == CONSISTENT)
    solver_->solve ( a_new, fres );

  if (mode_ == LUMPED)
  {
    a_new = massInv_ * fres;
    params.get( cons, ActionParams::CONSTRAINTS );
    jive::util::setSlaveDofs( a_new, *cons );
  }

  params.clear();

  v_new = v_old + a_new * dtime_;
  du = v_new; // HACK to proper SO(3) conversion
  u_new = u_old + du * dtime_; 

  // commit everything
  Globdat::commitStep( globdat );
  Globdat::commitTime( globdat );  
  StateVector::updateOld( dofs_, globdat );

  model_->takeAction ( Actions::COMMIT, params, globdat );

  StateVector::store( a_new, STATE[2], dofs_, globdat );
  StateVector::store( v_new, STATE[1], dofs_, globdat );
  StateVector::store( u_new, STATE[0], dofs_, globdat );

  // check if the mass matrix is still valid
  if ( FuncUtils::evalCond( *updCond_, globdat ) ) invalidate_();

  return OK;
}


//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------


void EulerForwardModule::shutdown ( const Properties& globdat )
{
  model_  = nullptr;
  solver_ = nullptr;
  dofs_   = nullptr;
  updCond_= nullptr;
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
  Properties              params;
  model_->takeAction ( Actions::UPD_MATRIX2, params, globdat );

  jem::System::info( myName_ ) << " ...Updating mass information for explicit solver\n";

  if ( mode_ == LUMPED )
  {
    Ref<DiagMatrixObject>   inertia;
    Globdat::findFor( inertia, "LumpedMass", this, globdat );

    massInv_ = inertia->getValues();    
    if ( jem::testany( massInv_ <= jem::Limits<double>::TINY_VALUE ) ) 
      throw jem::ArithmeticException("Zero (or negative) masses cannot be inversed!");
    massInv_ = 1 / massInv_;
  }

  valid_ = true;
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

  ModuleFactory::declare ( TYPE_NAME, &EulerForwardModule::makeNew );
}

