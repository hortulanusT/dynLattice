/**
 * @file PBCGroupOutputModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Wrapper Class for some default PBC Outputs
 * @version 0.1
 * @date 2021-12-06
 * 
 * @copyright Copyright (C) 2021 TU Delft. All rights reserved.
 * 
 */

#include "PBCGroupOutputModule.h"

const char*  PBCGroupOutputModule::TYPE_NAME = "PBCGroupOutput";
const char*  PBCGroupOutputModule::CHILD_NAME = "sampling";

PBCGroupOutputModule::PBCGroupOutputModule
  ( const String&       name) : Super ( name )
{
  child_ = newInstance<SampleModule>( myName_ + "." + CHILD_NAME );
}

Module::Status PBCGroupOutputModule::init
  ( const Properties&   conf,
    const Properties&   props,
    const Properties&   globdat )
{
  StringVector  groups (6);
  for (idx_t i = 0; i < 6; i++)
    groups[i] = PBCGroupInputModule::EDGES[i];

  // fill the properties with the default node and element Groups
  Properties myProps = props.getProps ( myName_ );
  myProps.set ( "nodeGroups", groups );
  myProps.set ( "elemGroups", "all" );

  JEM_PRECHECK2( Super::init( conf, props, globdat ) == Status::OK, "Error setting up the GroupOutputModule!" );

  // configure the output module
  // LATER get multiple Children for different kinds of outputs
  Properties childProps = props.makeProps (  myName_ + "." + CHILD_NAME );
  bool append = false;
  if (!childProps.find( append, PropNames::APPEND ) || !append)
    childProps.set( PropNames::HEADER, getHeader_() );
  childProps.set( PropNames::DATA_SETS, getDataSets_() );
  childProps.set( PropNames::SEPARATOR,  "," );
  
  child_->configure( props, globdat );
  child_->getConfig( conf, globdat );
  JEM_PRECHECK2( child_->init( conf, props, globdat) == Status::OK, "Error setting up the SampleModule!" );

  return Status::OK;
}

Module::Status PBCGroupOutputModule::run
  (const Properties&    globdat)
{
  Super::run(globdat);
  child_->run(globdat);

  return Status::OK;
}

void PBCGroupOutputModule::shutdown
  (const Properties&    globdat)
{
  Super::shutdown(globdat);
  child_->shutdown(globdat);
}

String PBCGroupOutputModule::getHeader_ () const
{
  const idx_t dim = elemDofs_.size();

  // step  
  String head = "step,";

  // displacement gradient
  for (idx_t i = 1; i <= dim; i++)
    for (idx_t j = 1; j <= dim; j++)
      head = head + String::format( "H%d%d,", i, j );  

  // 1st PK Tensor
  for (idx_t i = 1; i <= dim; i++)
    for (idx_t j = 1; j <= dim; j++)      
      head = head + String::format( "P%d%d,", i, j );  

  return head[SliceTo(head.size()-1)];
}

StringVector PBCGroupOutputModule::getDataSets_ () const
{  
  const idx_t dim = elemDofs_.size();

  ArrayBuffer<String> dataSets;

  // step
  dataSets.pushBack( "i" );

  // displacement gradient
  for (idx_t i = 0; i < dim; i++)
    for (idx_t j = 0; j < dim; j++)
    {
      dataSets.pushBack( String::format( "(%s.disp.%s - %s.disp.%s) / all.extent.%s"
        , PBCGroupInputModule::EDGES[1 + 2*j]
        , nodeDofNames_[i]
        , PBCGroupInputModule::EDGES[0 + 2*j]
        , nodeDofNames_[i]
        , elemDofNames_[j] ) );
    }  

  // Prepare areas
  StringVector areas (dim);
  if (dim==3)
  {
    areas[0] = String::format("( all.extent.%s * all.extent.%s )", elemDofNames_[1], elemDofNames_[2] );
    areas[1] = String::format("( all.extent.%s * all.extent.%s )", elemDofNames_[0], elemDofNames_[2] );
    areas[2] = String::format("( all.extent.%s * all.extent.%s )", elemDofNames_[1], elemDofNames_[0] );
  }
  else if (dim==2)
  {
    areas[0] = "all.extent." + elemDofNames_[1];
    areas[1] = "all.extent." + elemDofNames_[0];
  }
  else
    throw jem::Exception(JEM_FUNC, String::format("unkown dimension number %d", dim));
  
  // 1st PK Tensor
  for (idx_t i = 0; i < dim; i++)
    for (idx_t j = 0; j < dim; j++)
        dataSets.pushBack( String::format( "%s.resp.%s / %s"
          , PBCGroupInputModule::EDGES[1 + 2*j]
          , nodeDofNames_[i] 
          , areas[j] ) );  

  return dataSets.toArray();
}

Ref<Module> PBCGroupOutputModule::makeNew
  ( const String&       name,
    const Properties&   conf,
    const Properties&   props,
    const Properties&   globdat )
{
  return newInstance<PBCGroupOutputModule> ( name );
}
  
void PBCGroupOutputModule::declare ()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare ( TYPE_NAME, &makeNew);
}