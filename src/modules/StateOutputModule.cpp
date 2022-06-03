#include <filesystem>
#include <jem/base/ClassTemplate.h>
#include "modules/StateOutputModule.h"

JEM_DEFINE_CLASS( StateOutputModule );

//=======================================================================
//   class StateOutputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------


const char*  StateOutputModule::TYPE_NAME = "StateOutput";
const char*  StateOutputModule::STATE_PROP[3] =
              { "writeState0", "writeState1", "writeState2" };
const char*  StateOutputModule::NODE_PROP = "nodeSet";


//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------


StateOutputModule::StateOutputModule ( const String& name ) :

  Super ( name )

{
  dofsOut_.resize( 0 );
  stateOut_.resize( 3 );
  stateOut_ = { true, false, false };  
  
  started_    = false;
  sampleCond_ = FuncUtils::newCond ();
  output_     = nullptr;
}


StateOutputModule::~StateOutputModule ()
{}


//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------


Module::Status StateOutputModule::init

  ( const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )
{
  Properties myProps  = props.findProps( myName_ );
  Properties myConf   = conf .makeProps( myName_ );

  // StateVector Outputs
  for (idx_t i = 0; i < 3; i++)
  {
    myProps.find( stateOut_[i], STATE_PROP[i] );
    myConf .set ( STATE_PROP[i], stateOut_[i] );
  }

  // DOF Outputs
  dofs_ = DofSpace::get( globdat, getContext() );

  StringVector  dofNames (dofs_->typeCount());
  IdxVector     dofTypes (dofs_->typeCount());

  dofNames = dofs_->getTypeNames();
  myProps.find( dofNames, jive::femodel::PropNames::DOFS );

  dofTypes.resize( dofNames.size() );
  for (idx_t i = 0; i < dofTypes.size(); i++)
    dofTypes[i] = dofs_->getTypeIndex( dofNames[i] );
  myConf .set( jive::femodel::PropNames::DOFS, dofNames );

  // Node Outputs
  Assignable<NodeSet> nodes;
  String              nodesName = "all";

  myProps.find( nodesName, NODE_PROP );
  nodes = NodeSet::get( globdat, getContext() );
  dofsOut_.resize( nodes.size()*dofTypes.size() );
  for (idx_t i = 0; i < nodes.size(); i++)
    dofs_->getDofIndices( 
      dofsOut_[jem::SliceFromTo(i*dofTypes.size(), (i+1)*dofTypes.size())]
      , nodes.getNodeID(i), dofTypes);
  myConf. set( NODE_PROP, nodesName );

  // Output Condition
  FuncUtils::configCond ( sampleCond_,
                          jive::app::PropNames::SAMPLE_COND,
                          myProps, globdat );   
  FuncUtils::getConfig  ( myConf, sampleCond_,
                          jive::app::PropNames::SAMPLE_COND );

  // Output File
  String    outFile;
  props.get( outFile, CASE_NAME );
  outFile = outFile + "/stateVectors.csv";

  myProps.find( outFile, jive::app::PropNames::FILE );
  // construct the file folder
  std::filesystem::path folder = makeCString(outFile).addr();
  folder.remove_filename();
  std::filesystem::create_directories( folder );
  // construct the PrintWriter
  Ref<Writer> fileWriter;
  fileWriter = newInstance<FileWriter> ( outFile, jem::io::FileFlags::WRITE );
  // set the writer opject
  output_   = newInstance<PrintWriter> ( fileWriter );

  return OK;
}


//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------


Module::Status StateOutputModule::run ( const Properties& globdat )
{
  if (FuncUtils::evalCond ( *sampleCond_, globdat ) )
  {
    Vector  state ( dofsOut_.size() );
    idx_t   step;
    double  time = -1;

    globdat.get ( step, jive::util::Globdat::TIME_STEP );
    globdat.find( time, jive::util::Globdat::TIME );

    if (!started_) writeHeader_( time >= 0 );

    for (idx_t i = 0; i < 3; i++)
      if ( stateOut_[i] )
      {
        jive::model::StateVector::get ( state, jive::model::STATE[i], dofs_, globdat );
        writeLine_( (Vector)state[dofsOut_], step, jive::model::STATE[i], time );
      }   
  }

  return OK;
}


//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------


void StateOutputModule::shutdown ( const Properties& globdat )
{
  output_->close();
  output_ = nullptr;
}
//-----------------------------------------------------------------------
//   writeHeader_
//-----------------------------------------------------------------------

void StateOutputModule::writeHeader_ 

  ( const bool          time )
{
  print( *output_, "step", ',' );
  if (time) print( *output_, "time", ',' );
  print( *output_, "state" );
  
  for (idx_t idof : dofsOut_) 
    print( *output_, ',', dofs_->getDofName(idof) );

  print( *output_, jem::io::endl ); 

  started_ = true;
}


//-----------------------------------------------------------------------
//   writeLine_
//-----------------------------------------------------------------------

void StateOutputModule::writeLine_

  ( const Vector&         data,
    const idx_t           step,
    const StateTag        state,
    const double          time ) const
{
  print( *output_, step, ',' );
  if (time >= 0) print( *output_, time, ',' );
  print( *output_, state );
  
  for (double datum : data) print( *output_, ',', datum );

  print( *output_, jem::io::endl );  
}


//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------


Ref<Module> StateOutputModule::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  return jem::newInstance<Self> ( name );
}


//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------


void StateOutputModule::declare ()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare ( TYPE_NAME,  & makeNew );
  ModuleFactory::declare ( CLASS_NAME, & makeNew );
}