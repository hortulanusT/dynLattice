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
#include <jive/util/Printer.h>

const char*      periodicBCModel::TYPE_NAME       = "PeriodicBC";
const char*      periodicBCModel::DISP_GRAD_PROP  = "dispGrad";
const char*      periodicBCModel::DOF_NAMES_PROP  = "dofs";

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

  // get the displacement Gradient  
  dispGrad_.resize(nodes_.rank(), nodes_.rank());
  dispGrad_ = NAN;
  for (idx_t iDisp = 0; iDisp < nodes_.rank(); iDisp++)
  {
    for (idx_t iDir = 0; iDir < nodes_.rank(); iDir++)
    { 
      myProps.find( dispGrad_(iDisp, iDir), DISP_GRAD_PROP + String(iDisp+1) + String(iDir+1) );
      myConf .set ( DISP_GRAD_PROP + String(iDisp+1) + String(iDir+1), dispGrad_(iDisp, iDir) );
    }    
  }
  
  // get the dof names
  myProps.get ( dofNames_, DOF_NAMES_PROP );
  myConf .set ( DOF_NAMES_PROP, dofNames_ );
}

bool      periodicBCModel::takeAction

  ( const String&         action,
    const Properties&     params,
    const Properties&     globdat )
{
  using jive::model::Actions;
  using jive::model::ActionParams;

  if (action == Actions::INIT)
  {
    init_ ( globdat );
  }

  if (action == Actions::GET_CONSTRAINTS)
  {    
    double scale;

    // get the scale factor
    params.get ( scale, ActionParams::SCALE_FACTOR );

    setConstraints_ ( globdat, scale );    
    return true;
  }

  return false;
}


void      periodicBCModel::init_  
  ( const Properties&     globdat )
{
  jdofs_.resize( dofNames_.size() );
  for (idx_t iDof = 0; iDof < jdofs_.size(); iDof++)
    jdofs_[iDof] = dofs_->getTypeIndex( dofNames_[iDof] ); 

  masterDofs_.resize( nodes_.rank(), nodes_.rank() );
  slaveDofs_.resize( nodes_.rank(), nodes_.rank() );

  Assignable<NodeGroup> masters;
  Assignable<NodeGroup> slaves;
  bool                  identical;

  for (idx_t iEdge = 0; iEdge < nodes_.rank(); iEdge++)
  {
    masters   = NodeGroup::get( PBCGroupInputModule::EDGES[2*iEdge  ], nodes_, globdat, getContext() );
    slaves    = NodeGroup::get( PBCGroupInputModule::EDGES[2*iEdge+1], nodes_, globdat, getContext() );
    identical = jem::testall(masters.getIDs() == slaves.getIDs());

    if ( !identical ) // check that the investigated body is not flat (otherwise leave the vector empty to be ignored)
      for (idx_t iDof = 0; iDof < nodes_.rank(); iDof++ )
      {
        masterDofs_(iDof, iEdge).resize( masters.size() );
        slaveDofs_(iDof, iEdge).resize( slaves.size() );
        dofs_->getDofIndices( masterDofs_(iDof, iEdge), masters.getIndices(), jdofs_[iDof] );
        dofs_->getDofIndices( slaveDofs_(iDof, iEdge), slaves.getIndices(), jdofs_[iDof] );
      } 
  }  
}

void      periodicBCModel::setConstraints_   
  ( const Properties&     globdat,
    const double          scale )
{
  // TEST_CONTEXT(masterDofs_)
  // TEST_CONTEXT(slaveDofs_)
  // TEST_CONTEXT(cons_->getSlaveDofs())
  double extent = 1.;

  for (idx_t iDof = 0; iDof < nodes_.rank(); iDof++)
    for (idx_t iEdge = 0; iEdge < nodes_.rank(); iEdge++ )
    {
      if ( std::isnan(dispGrad_(iDof, iEdge)) )
      {
        if ( iDof == iEdge ) 
          // if the dispGrad for this is not configured and its a normal term, just fix the master Nodes
          for (idx_t iNode = 0; iNode < masterDofs_(iDof, iEdge).size(); iNode++) cons_->addConstraint(masterDofs_(iDof, iEdge)[iNode]);
        
        // if the dispGrad for this is not configured and its a shear term, skip it
        continue;
      } 
      Globdat::getVariables( "all.extent", globdat ).find( extent, dofNames_[iEdge]); // TODO use this syntax for the LoadExtent Module

      for (idx_t iNode = 0; iNode < masterDofs_(iDof, iEdge).size(); iNode++)
      {
        // set the master DOF to zero and the slave DOF to the prescribed strain
        cons_->addConstraint(masterDofs_(iDof, iEdge)[iNode]);
        cons_->addConstraint(slaveDofs_(iDof, iEdge)[iNode],  scale*dispGrad_(iDof, iEdge)*extent);
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