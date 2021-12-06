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

  JEM_ASSERT2( Super::init( conf, props, globdat ) == Status::OK, "Error setting up the GroupOutputModule!" );

  // configure the output module
  Properties childProps = props.makeProps (  myName_ + "." + CHILD_NAME );
  // FIXME make with options & string constants
  childProps.set( "header", "Hyy\tnu_xy" );
  childProps.set( "seperator", "\t" );
  childProps.set( "dataSets", StringVector( { "ymax.disp.dy/all.extent.dy", "-1 * (ymax.diff_disp.dy/all.extent.dy) / (xmax.diff_disp.dx/all.extent.dx)" } ) );
  
  child_->configure( props, globdat );
  child_->getConfig( conf, globdat );
  JEM_ASSERT2( child_->init( conf, props, globdat) == Status::OK, "Error setting up the SampleModule!" );

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