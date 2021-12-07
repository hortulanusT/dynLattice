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
const char*      periodicBCModel::ROT_NAMES_PROP  = "rotDofs";

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
  
  // get the dof names
  myProps.get ( dofNames_, DOF_NAMES_PROP );
  myConf .set ( DOF_NAMES_PROP, dofNames_ );

  pbcRank_ = dofNames_.size();

  // get the rot names
  myProps.get ( rotNames_, ROT_NAMES_PROP );
  myConf .set ( ROT_NAMES_PROP, rotNames_ );

  // get the displacement Gradient  
  dispGrad_.resize(pbcRank_, pbcRank_);
  dispGrad_ = NAN;
  for (idx_t iDisp = 0; iDisp < pbcRank_; iDisp++)
  {
    for (idx_t iDir = 0; iDir < pbcRank_; iDir++)
    { 
      myProps.find( dispGrad_(iDisp, iDir), DISP_GRAD_PROP + String(iDisp+1) + String(iDir+1) );
      myConf .set ( DISP_GRAD_PROP + String(iDisp+1) + String(iDir+1), dispGrad_(iDisp, iDir) );
    }    
  }
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
  IdxVector rdofs ( rotNames_.size() );  
  for (idx_t iDof = 0; iDof < rdofs.size(); iDof++)
    rdofs[iDof] = dofs_->getTypeIndex( rotNames_[iDof] ); 

  jdofs_.resize( dofNames_.size() );
  for (idx_t iDof = 0; iDof < jdofs_.size(); iDof++)
    jdofs_[iDof] = dofs_->getTypeIndex( dofNames_[iDof] ); 

  masterDofs_.resize( pbcRank_, pbcRank_ );
  slaveDofs_.resize( pbcRank_, pbcRank_ );

  IdxVector             masterRots;
  IdxVector             slaveRots;

  Assignable<NodeGroup> masters;
  Assignable<NodeGroup> slaves;

  for (idx_t iEdge = 0; iEdge < pbcRank_; iEdge++)
  {
    masters   = NodeGroup::get( PBCGroupInputModule::EDGES[2*iEdge  ], nodes_, globdat, getContext() );
    slaves    = NodeGroup::get( PBCGroupInputModule::EDGES[2*iEdge+1], nodes_, globdat, getContext() );

    // save the translational DOFs for the 
    for (idx_t iDof = 0; iDof < pbcRank_; iDof++ )
    {
      masterDofs_(iDof, iEdge).resize( masters.size() );
      slaveDofs_(iDof, iEdge).resize( slaves.size() );
      dofs_->getDofIndices( masterDofs_(iDof, iEdge), masters.getIndices(), jdofs_[iDof] );
      dofs_->getDofIndices( slaveDofs_(iDof, iEdge), slaves.getIndices(), jdofs_[iDof] );
      
      // if the dispGrad isnt configured set some ground rules
      if ( std::isnan(dispGrad_(iDof, iEdge)) )
      {
        if ( iDof == iEdge ) 
          // if the dispGrad for this is not configured and its a normal term, just fix the master Nodes to the ground
          for (idx_t iNode = 0; iNode < masterDofs_(iDof, iEdge).size(); iNode++)
            cons_->addConstraint(masterDofs_(iDof, iEdge)[iNode]);
        else
          // if the dispGrad for this is not configured and its a shear term, just fix the slave Nodes to the master nodes
          for (idx_t iNode = 0; iNode < masterDofs_(iDof, iEdge).size(); iNode++)
            cons_->addConstraint(slaveDofs_(iDof, iEdge)[iNode], masterDofs_(iDof, iEdge)[iNode], 1.0);
      }
    } 

    // lock the rotational DOFs for the edges
    for (idx_t iDof = 0; iDof < pbcRank_; iDof++ )
    {
      masterRots.resize( masters.size() );
      slaveRots.resize( slaves.size() );

      dofs_->getDofIndices( masterRots, masters.getIndices(), rdofs[iDof] );
      dofs_->getDofIndices( slaveRots, slaves.getIndices(), rdofs[iDof] );

      for (idx_t iN = 0; iN < masterRots.size(); iN++)
      {
        cons_->addConstraint( slaveRots[iN], masterRots[iN], 1.0 );
      }      
    } 
  }  
}

void      periodicBCModel::setConstraints_   
  ( const Properties&     globdat,
    const double          scale )
{  
  // cons_->printTo(jive::util::Printer::get());
  // jive::util::Printer::flush();
  // REPORT( scale )
  // TEST_CONTEXT(masterDofs_)
  // TEST_CONTEXT(slaveDofs_)
  // TEST_CONTEXT(cons_->getSlaveDofs())
  double extent = 1.;

  for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
    for (idx_t iEdge = 0; iEdge < pbcRank_; iEdge++ )
    {
      if ( std::isnan(dispGrad_(iDof, iEdge)) )
        continue;// if the dispGrad for this is not configured, skip it
        
      Globdat::getVariables( "all.extent", globdat ).get( extent, dofNames_[iEdge]);
      
      // TEST_CONTEXT(iDof)
      // TEST_CONTEXT(iEdge)

      for (idx_t iNode = 0; iNode < masterDofs_(iDof, iEdge).size(); iNode++)
      {
        // set the master DOF to zero and the slave DOF to the prescribed strain
        cons_->addConstraint(masterDofs_(iDof, iEdge)[iNode]);
        cons_->addConstraint(slaveDofs_(iDof, iEdge)[iNode],  scale*dispGrad_(iDof, iEdge)*extent);
      }
    }

  // cons_->printTo(jive::util::Printer::get());
  // jive::util::Printer::flush();
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