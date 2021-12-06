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

PBCGroupOutputModule::PBCGroupOutputModule
  ( const String&       name) : Super ( name )
{
}

Module::Status PBCGroupOutputModule::init
  ( const Properties&   conf,
    const Properties&   props,
    const Properties&   globdat )
{
  Ref<DofSpace> dofs = DofSpace::get( globdat, getContext() );
  StringVector  groups (6);
  for (idx_t i = 0; i < 6; i++)
    groups[i] = PBCGroupInputModule::EDGES[i];

  Properties myProps = props.makeProps ( myName_ );
  myProps.set ( "nodeGroups", groups );
  myProps.set ( "elemGroups", "all" );
  myProps.set ( "dofs", dofs->getTypeNames()[jem::SliceTo(3)] );
  myProps.set ( "dimensions", dofs->getTypeNames()[jem::SliceTo(3)] );

  return Super::init( conf, props, globdat );
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