
#include "modules/ExplicitModule.h"

//=======================================================================
//   class ExplicitModule
//=======================================================================

const char* ExplicitModule::TYPE_NAME   = "Explicit";
const char* ExplicitModule::STEP_COUNT  = "stepCount";
const char* ExplicitModule::SO3_DOFS    = "dofs_SO3";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

ExplicitModule::ExplicitModule 

  ( const String&      name ) :

  Super ( name )

{
  dtime_  = 1.0;
  valid_  = false;
  stepCount_  = 1;
  SO3_dofs_.resize( 0 );
  rdofs_.resize( 0, 0 );
}


ExplicitModule::~ExplicitModule ()
{}


//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------


Module::Status ExplicitModule::init

  ( const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  using jem::util::connect;
  using jive::implict::PropNames;

  Properties            myConf  = conf .makeProps ( myName_ );
  Properties            myProps = props.findProps ( myName_ );
  StringVector          SO3_dof_names;
  Properties            params;
  Properties            sparams;
  Ref<AbstractMatrix>   inertia;
  Ref<DiagMatrixObject> diagInertia;

  // get the step count

  myProps.find( stepCount_, STEP_COUNT, 1, 2 );
  myConf .set ( STEP_COUNT, stepCount_ );

  // returning an address of an object pointing to the Model

  model_ = Model::get ( globdat, getContext() );

  // returning an address of an object pointing to class DofSpace

  dofs_  = DofSpace::get ( globdat, getContext() );
 
  // Invalidate the state of this module when the DofSpace changes.

  connect ( dofs_->newSizeEvent,  this, &Self::invalidate_ );
  connect ( dofs_->newOrderEvent, this, &Self::invalidate_ );

  // get the SO3 dof names

  if ( myProps.find( SO3_dof_names, SO3_DOFS ) )
  {
    JEM_PRECHECK( SO3_dof_names.size() == 3 );
    SO3_dofs_.resize( SO3_dof_names.size() );
    for (idx_t i = 0; i < SO3_dof_names.size(); i++)
      SO3_dofs_[i] = dofs_->getTypeIndex( SO3_dof_names[i] );
    myConf .set( SO3_DOFS, SO3_dof_names );
  }

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

  // Initialize the global simulation time and the time step number as well as the constraints
  Globdat::initTime( globdat );
  Globdat::initStep( globdat );
  model_->takeAction ( Actions::GET_CONSTRAINTS, params, globdat );

  return OK;
}


//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------

Module::Status ExplicitModule::run 

  ( const Properties& globdat )

{
  using jive::model::STATE;
  const idx_t dofCount = dofs_->dofCount();
  const idx_t rotCount = SO3_dofs_.size();

  idx_t                 step;
  Properties            params;
  Ref<Constraints>      cons = Constraints::find ( dofs_, globdat );
  Vector      fint     ( dofCount );
  Vector      fext     ( dofCount );
  Vector      fres     ( dofCount );
  Vector      u_old, v_old, a_old;
  Vector      dv       ( dofCount );
  Vector      du       ( dofCount );
  Vector      u_new    ( dofCount );
  Vector      v_new    ( dofCount );
  Vector      a_new    ( dofCount );

  Vector      r_node   ( rotCount );
  Vector      d_r      ( rotCount );
  Matrix      R_old    ( rotCount, rotCount );
  Matrix      R_new    ( rotCount, rotCount );
  Matrix      V_upd    ( rotCount, rotCount );

  // skip if no model exists
  if ( !(model_) )
    return DONE;

  // update mass matrix if necessary 
  if ( ! valid_ )
    restart_ ( globdat );

  // Get the state vectors from the last time step
  StateVector::get  ( u_old, STATE[0], dofs_, globdat );
  StateVector::get  ( v_old, STATE[1], dofs_, globdat );
  StateVector::get  ( a_old, STATE[2], dofs_, globdat );

  // Get the internal and external force vectors last time step
  fext = 0.0;
  fint = 0.0;

  params.set ( ActionParams::EXT_VECTOR, fext );
  params.set ( ActionParams::INT_VECTOR, fint );

  model_->takeAction ( Actions::GET_EXT_VECTOR, params, globdat );
  model_->takeAction ( Actions::GET_INT_VECTOR, params, globdat );
  model_->takeAction ( "GET_GYRO_VECTOR", params, globdat );

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
  globdat.get( step, Globdat::TIME_STEP );
  
  // update velocity
  if (stepCount_ >= 2 && step >= 2)
    dv = dtime_/2 * (3*a_new - 1*a_old);
  else
    dv = dtime_ * a_new;

  v_new = v_old + dv;

  // update position
  if (stepCount_ >= 2 && step >= 2)
    du = dtime_/2 * (3*v_new - 1*v_old);
  else
    du = dtime_ * v_new;

  u_new = u_old + du;

  // do different update for rotational dofs
  for ( idx_t inode = 0; inode < rdofs_.size(1); inode++ )
  {
    r_node  = u_old[rdofs_[inode]];
    d_r     = du[rdofs_[inode]];
    
    expVec  ( R_old, r_node );
    expVec  ( V_upd, d_r );
    matmul  ( R_new, V_upd, R_old );

    logMat  ( r_node, R_new );

    u_new[rdofs_[inode]]  = r_node;
  }
  
  // commit everything
  Globdat::commitStep( globdat );
  Globdat::commitTime( globdat );  
  model_->takeAction ( Actions::COMMIT, params, globdat );

  StateVector::updateOld( dofs_, globdat );
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


void ExplicitModule::shutdown ( const Properties& globdat )
{
  model_  = nullptr;
  solver_ = nullptr;
  dofs_   = nullptr;
  updCond_= nullptr;
}


//-----------------------------------------------------------------------
//   configure
//-----------------------------------------------------------------------


void ExplicitModule::configure

  ( const Properties&  props,
    const Properties&  globdat )

{
  Properties  myProps = props.findProps ( myName_ );

  myProps.find ( dtime_, jive::implict::PropNames::DELTA_TIME, 1.0e-20, 1.0e20 );
}


//-----------------------------------------------------------------------
//   getConfig
//-----------------------------------------------------------------------


void ExplicitModule::getConfig

  ( const Properties&  conf,
    const Properties&  globdat ) const

{
  Properties  myConf = conf.makeProps ( myName_ );

  myConf.set ( jive::implict::PropNames::DELTA_TIME, dtime_ );
}


//-----------------------------------------------------------------------
//   restart_
//-----------------------------------------------------------------------


void ExplicitModule::restart_ ( const Properties& globdat )
{
  Properties    params;
  IdxVector     iitems   ( dofs_->getItems()->size() );


  jem::System::info( myName_ ) << " ...Updating mass information for explicit solver\n";
  model_->takeAction ( Actions::UPD_MATRIX2, params, globdat );

  if (SO3_dofs_.size())
  {
    jem::System::info( myName_ ) << " ...Updating SO(3) dof inormation for explicit solver\n";
    rdofs_.resize ( SO3_dofs_.size(), dofs_->getItems()->size() );
    for ( idx_t idof = 0; idof < SO3_dofs_.size(); idof++)
      dofs_->getDofsForType( rdofs_( idof, ALL ), iitems, SO3_dofs_[idof] );
  }

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


void ExplicitModule::invalidate_ ()
{
  valid_ = false;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module>  ExplicitModule::makeNew

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

void ExplicitModule::declare ()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare ( TYPE_NAME, &ExplicitModule::makeNew );
}

