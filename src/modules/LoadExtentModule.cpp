#include "LoadExtentModule.h"

const char*  LoadExtentModule::TYPE_NAME = "LoadExtent";

LoadExtentModule::LoadExtentModule
  ( const String&       name) : Module ( name )
{
  // per default extract composite values for all elements
  elemGroups_.resize( 1 );
  elemGroups_[0] = "all";
}

Module::Status LoadExtentModule::init
  ( const Properties&   conf,
    const Properties&   props,
    const Properties&   globdat )
{
  // retrieve and create subpropertysets
  Properties myProps = props.getProps( myName_ );
  Properties myConf  = conf.makeProps( myName_ );

  // get the element groups
  myProps.find( elemGroups_, "elementGroups" );
  myConf .set ( "elementGroups", elemGroups_ );

  // get the node groups
  myProps.find( nodeGroups_, "nodeGroups" );
  myConf .set ( "nodeGroups", nodeGroups_ );

  // get the dofs for the element group
  myProps.find( elemDofNames_, "dimensions" );
  myConf .set ( "dimensions", elemDofNames_ );

  // get the dofs for the node groups
  myProps.find( nodeDofNames_, "dofs" );
  myConf .set ( "dofs", nodeDofNames_ );

  // translate the DOF Names to IDs
  elemDofs_.resize( elemDofNames_.size() );
  nodeDofs_.resize( nodeDofNames_.size() );
  Ref<DofSpace> dofs = DofSpace::get( globdat, getContext() );
  
  for (idx_t idof = 0; idof < elemDofNames_.size(); idof++)
    elemDofs_[idof] = dofs->getTypeIndex( elemDofNames_[idof] ); 
  for (idx_t idof = 0; idof < nodeDofNames_.size(); idof++)
    nodeDofs_[idof] = dofs->getTypeIndex( nodeDofNames_[idof] );

  return elemDofs_.size()+nodeDofs_.size()>0 ? run( globdat ) : Status::DONE;
}
  
Module::Status LoadExtentModule::run
  ( const Properties&   globdat )
{    
  Properties myVars   = Globdat::getVariables( globdat );
  Properties currentVars, loadVars, respVars, dispVars, extentVars;
  NodeSet    nodes    = NodeSet::get( globdat, getContext() );
  ElementSet elems    = ElementSet::get ( globdat, getContext() );

  IdxVector  nodeIndices;
  Ref<DofSpace> dofs  = DofSpace::get( globdat, getContext() );
  IdxVector  dofIndices;

  Vector     allLoad  ( dofs->dofCount() );
  Vector     allResp  ( dofs->dofCount() );
  Vector     allDisp  ( dofs->dofCount() );
  Matrix     coords   ( nodes.size(), nodes.rank() );
  StateVector::get    ( allDisp, dofs, globdat );
  Ref<Model> model    = Model::get( globdat, getContext() );
  Properties params   ( "actionParams" );
  allLoad             = 0.;
  params.set          ( ActionParams::EXT_VECTOR, allLoad );
  model->takeAction   ( Actions::GET_EXT_VECTOR, params, globdat );
  params.erase        ( ActionParams::EXT_VECTOR );

  allResp             = 0.;
  params.set          ( ActionParams::INT_VECTOR, allResp );
  model->takeAction   ( Actions::GET_INT_VECTOR, params, globdat );
  params.erase        ( ActionParams::INT_VECTOR );

  Vector     load, disp, resp;

  // iterate through the node groups
  for (idx_t iNodeGroup = 0; iNodeGroup < nodeGroups_.size(); iNodeGroup++)
  {
    // get the nodes associated with this node group
    currentVars         = myVars.makeProps( nodeGroups_[iNodeGroup] );
    loadVars            = currentVars.makeProps( "load" );
    respVars            = currentVars.makeProps( "resp" );
    dispVars            = currentVars.makeProps( "disp" );
    NodeGroup nodeGroup = NodeGroup::get ( nodeGroups_[iNodeGroup], nodes, globdat, getContext() );
    nodeIndices.resize  ( nodeGroup.size() );
    dofIndices.resize   ( nodeGroup.size() );
    load.resize         ( nodeGroup.size() );
    resp.resize         ( nodeGroup.size() );
    disp.resize         ( nodeGroup.size() );
    nodeIndices         = nodeGroup.getIndices();

    // iterate through all the dofs to report
    for (idx_t iDof = 0; iDof < nodeDofs_.size(); iDof++)
    {
      // get the indices of the dofs at those nodes
      dofs->getDofIndices (dofIndices, nodeIndices, nodeDofs_[iDof] );

      // get the displacements and loads for those dofs
      disp = allDisp[dofIndices];
      load = allLoad[dofIndices];
      resp = allResp[dofIndices];

      // report it back (sum for load/resp and avg for disp)
      loadVars.set( nodeDofNames_[iDof], sum( load ));
      respVars.set( nodeDofNames_[iDof], sum( resp ));
      dispVars.set( nodeDofNames_[iDof], sum( disp ) / disp.size() );
    }
  }

  // iterate through the element groups
  for (idx_t iElemGroup = 0; iElemGroup < elemGroups_.size(); iElemGroup++)
  {
    // get the nodes associated with this element group
    currentVars         = myVars.makeProps( elemGroups_[iElemGroup] );
    extentVars          = currentVars.makeProps( "extent" );
    ElementGroup elemGroup = ElementGroup::get ( elemGroups_[iElemGroup], elems, globdat, getContext() );
    nodeIndices.resize  ( elemGroup.getNodeIndices().size() );
    dofIndices.resize   ( nodeIndices.size() );
    load.resize         ( nodeIndices.size() );
    disp.resize         ( nodeIndices.size() );
    nodeIndices         = elemGroup.getNodeIndices();

    nodes.getCoords     ( coords.transpose() );

    // iterate through the dimensions
    for (idx_t iDof = 0; iDof < elemDofs_.size(); iDof++)
    {
      // get the indices of the dofs at those nodes
      dofs->getDofIndices (dofIndices, nodeIndices, elemDofs_[iDof] );

      // get the displacements and loads for those dofs
      disp = allDisp[dofIndices];
      load = allLoad[dofIndices];

      // report back the extent
      extentVars.set( elemDofNames_[iDof], max( coords[elemDofs_[iDof]] ) - min( coords[elemDofs_[iDof]] )); // LATER update with values from last timestep maybe?

      // LATER implement stress/strain measures      
    }
    // TEST_CONTEXT(extentVars)
  }

  // TEST_CONTEXT( myVars )
  return Status::OK;
}

Ref<Module> LoadExtentModule::makeNew
  ( const String&       name,
    const Properties&   conf,
    const Properties&   props,
    const Properties&   globdat )
{
  return newInstance<LoadExtentModule> ( name );
}
  
void LoadExtentModule::declare ()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare ( TYPE_NAME, &makeNew);
}