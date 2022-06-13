#include <filesystem>
#include <jem/base/ClassTemplate.h>
#include "modules/ForceOutputModule.h"

JEM_DEFINE_CLASS( ForceOutputModule );

//=======================================================================
//   class ForceOutputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------


const char*  ForceOutputModule::TYPE_NAME = "ForceOutput";
const char*  ForceOutputModule::INT_PROP  = "writeIntForce";
const char*  ForceOutputModule::EXT_PROP  = "writeExtForce";
const char*  ForceOutputModule::GYRO_PROP = "writeGyroForce";
const char*  ForceOutputModule::NODE_PROP = "nodeSet";


//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------


ForceOutputModule::ForceOutputModule ( const String& name ) :

  Super ( name )

{
  dofsOut_.resize( 0 );
  intOut_ = true;
  extOut_ = true;  
  gyroOut_ = false;  
  
  started_    = false;
  sampleCond_ = FuncUtils::newCond ();
  output_     = nullptr;
}


ForceOutputModule::~ForceOutputModule ()
{}


//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------


Module::Status ForceOutputModule::init

  ( const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )
{
  Properties myProps  = props.findProps( myName_ );
  Properties myConf   = conf .makeProps( myName_ );

  // ForceVector Outputs
  myProps.find( intOut_, INT_PROP );
  myConf .set ( INT_PROP, intOut_ );
  myProps.find( extOut_, EXT_PROP );
  myConf .set ( EXT_PROP, extOut_ );
  myProps.find( gyroOut_, GYRO_PROP );
  myConf .set ( GYRO_PROP, gyroOut_ );

  // get the model
  model_ = Model::get( globdat, getContext() );

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
  outFile = outFile + "/forceVectors.csv";

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
  output_->nformat.setFractionDigits(8);
  output_->nformat.setShowSign();
  output_->nformat.setScientific();

  return OK;
}


//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------


Module::Status ForceOutputModule::run ( const Properties& globdat )
{
  if (FuncUtils::evalCond ( *sampleCond_, globdat ) )
  {
    Vector  force ( dofsOut_.size() );
    idx_t   step;
    double  time = -1;

    globdat.get ( step, jive::util::Globdat::TIME_STEP );
    globdat.find( time, jive::util::Globdat::TIME );

    if (!started_) writeHeader_( time >= 0 );

    Properties params;
    
    if (intOut_)
    {
      force = 0.;
      params.set( ActionParams::INT_VECTOR, force );
      model_->takeAction( Actions::GET_INT_VECTOR, params, globdat );
      writeLine_( force, step, ActionParams::INT_VECTOR, time );
      params.clear();
    }

    if (extOut_)
    {
      force = 0.;
      params.set( ActionParams::EXT_VECTOR, force );
      model_->takeAction( Actions::GET_EXT_VECTOR, params, globdat );
      writeLine_( force, step, ActionParams::EXT_VECTOR, time );
      params.clear();
    }

    if (gyroOut_)
    {
      force = 0.;
      params.set( ActionParams::INT_VECTOR, force );
      model_->takeAction( "GET_GYRO_VECTOR", params, globdat );
      writeLine_( force, step, "gyroVector", time );
      params.clear();
    }
  }

  return OK;
}


//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------


void ForceOutputModule::shutdown ( const Properties& globdat )
{
  output_->close();
  output_ = nullptr;
}
//-----------------------------------------------------------------------
//   writeHeader_
//-----------------------------------------------------------------------

void ForceOutputModule::writeHeader_ 

  ( const bool          time )
{
  if (time) print( *output_, "time", ',' );
  else print( *output_, "step", ',' );
  print( *output_, "force" );
  
  for (idx_t idof : dofsOut_) 
    print( *output_, ',', dofs_->getDofName(idof) );

  print( *output_, jem::io::endl ); 

  started_ = true;
}


//-----------------------------------------------------------------------
//   writeLine_
//-----------------------------------------------------------------------

void ForceOutputModule::writeLine_

  ( const Vector&         data,
    const idx_t           step,
    const String          type,
    const double          time ) const
{
  if (time >= 0) print( *output_, time, ',' );
  else print( *output_, step, ',' );
  print( *output_, type );
  
  for (double datum : data) print( *output_, ',', datum );

  print( *output_, jem::io::endl );  
}


//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------


Ref<Module> ForceOutputModule::makeNew

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


void ForceOutputModule::declare ()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare ( TYPE_NAME,  & makeNew );
  ModuleFactory::declare ( CLASS_NAME, & makeNew );
}