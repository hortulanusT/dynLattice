/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *  
 * This class implements a periodic BC Model
 * 
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: September 21
 *
 */
#include "PeriodicBCModel.h"

const char*      periodicBCModel::TYPE_NAME       = "PeriodicBC";
const char*      periodicBCModel::SETS_PROP       = "nodeSets";
const char*      periodicBCModel::DIR_PROP        = "periodDir";
const char*      periodicBCModel::PERIOD_PROP     = "periodicity";
const char*      periodicBCModel::LOCK_DOFS_PROP  = "periodDofs";

periodicBCModel::periodicBCModel

  ( const String&         name,
    const Properties&     conf,
    const Properties&     props,
    const Properties&     globdat ) : Model ( name )
{
  // Get the Properties associated with this model
  Properties  myProps = props.findProps ( myName_ );
  Properties  myConf  = conf .makeProps ( myName_ );
  
  // get the global nodeSet, dofSpace and Constraints
  nodes_  = NodeSet::get      ( globdat, getContext() );
  dofs_   = DofSpace::get     ( nodes_.getData(), globdat, getContext() );
  cons_   = Constraints::get  ( dofs_, globdat );

  // get the name of the groups
  myProps.get ( nodeSets_, SETS_PROP );
  JEM_ASSERT2 ( nodeSets_.size() == 2, "Only 2 Nodesets at one time are supported");
  myConf .set ( SETS_PROP, nodeSets_ );

  // periodicity direction
  idx_t iDir = -1;
  if (myProps.find ( iDir, DIR_PROP ) and iDir >= 0 and iDir <= nodes_.rank())
  {
    Matrix coords ( nodes_.rank(), nodes_.size() );
    nodes_.getCoords ( coords );
    periodVector_.resize ( nodes_.rank() );

    for (idx_t i = 0; i < nodes_.rank(); i++)
    {
      if (iDir == i) periodVector_[i] = max(coords(i, ALL)) - min(coords(i, ALL));
      else periodVector_[i] = 0.;
    }    

    System::info( getContext() ) << "Peridocity direction " << iDir << " resulted in a periocity vector of " << periodVector_ << " \n";
  }
  else myProps.get ( periodVector_, PERIOD_PROP );

  myConf .set ( PERIOD_PROP, periodVector_ );

  // periodicity dofs
  myProps.get ( periodDofs_, LOCK_DOFS_PROP );
  myConf .set ( LOCK_DOFS_PROP, periodDofs_ );
}

bool      periodicBCModel::takeAction

  ( const String&         action,
    const Properties&     params,
    const Properties&     globdat )
{
  using jive::model::Actions;

  if (action == Actions::GET_CONSTRAINTS)
  {
    setConstraints_ ( globdat );    
    return true;
  }

  return false;
}

void      periodicBCModel::setConstraints_ 
  
  ( const Properties&     globdat )
{
  Vector    master_coords ( nodes_.rank() );
  Vector    slave_coords  ( nodes_.rank() );
  IdxVector jtypes        ( periodDofs_.size() );
  idx_t     slaveDof      ( periodDofs_.size() );
  idx_t     masterDof     ( periodDofs_.size() );

  NodeGroup masterGroup   = NodeGroup::get( nodeSets_[0], nodes_, globdat, getContext() );
  NodeGroup slaveGroup    = NodeGroup::get( nodeSets_[1], nodes_, globdat, getContext() );
  
  IdxVector masterNodes   = masterGroup.getIndices();
  IdxVector slaveNodes    = slaveGroup.getIndices();

  for (idx_t i = 0; i<periodDofs_.size(); i++) jtypes[i] = dofs_->getTypeIndex ( periodDofs_[i] );

  // iterate through the master nodes
  for (idx_t inode = 0; inode < masterNodes.size(); inode++)
  {
    nodes_.getNodeCoords( master_coords, masterNodes[inode] );
    nodes_.getNodeCoords ( slave_coords, slaveNodes[inode] );
    // find the slave node corresponding to the master node
    JEM_ASSERT2( jem::isTiny(norm2(master_coords+periodVector_-slave_coords)) 
      || jem::isTiny(norm2(master_coords-periodVector_-slave_coords)), 
      "master and slave nodes not ordered in the same way");

    // apply all the single periodic dofs
    for (idx_t i = 0; i < periodDofs_.size(); i++)
    {
      masterDof = dofs_->getDofIndex( masterNodes[inode], jtypes[i]);
      slaveDof  = dofs_->getDofIndex( slaveNodes[inode], jtypes[i]);

      cons_->addConstraint ( slaveDof, masterDof, 1.0 );
    }        
  }  
}

Ref<Model>periodicBCModel::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )
{
  return newInstance<periodicBCModel> ( name, conf, props, globdat );
}

void      periodicBCModel::declare ()
{
  using jive::model::ModelFactory;

  ModelFactory::declare ( TYPE_NAME, & makeNew );
}