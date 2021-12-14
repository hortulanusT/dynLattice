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
  // H_ij = du_i/dX_j
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

  for (idx_t iDir = 0; iDir < pbcRank_; iDir++)
  {
    masters   = NodeGroup::get( PBCGroupInputModule::EDGES[2*iDir  ], nodes_, globdat, getContext() );
    slaves    = NodeGroup::get( PBCGroupInputModule::EDGES[2*iDir+1], nodes_, globdat, getContext() );

    // save the translational DOFs for the 
    for (idx_t iDof = 0; iDof < pbcRank_; iDof++ )
    {
      masterDofs_(iDof, iDir).resize( masters.size() );
      slaveDofs_(iDof, iDir).resize( slaves.size() );
      dofs_->getDofIndices( masterDofs_(iDof, iDir), masters.getIndices(), jdofs_[iDof] );
      dofs_->getDofIndices( slaveDofs_(iDof, iDir), slaves.getIndices(), jdofs_[iDof] );
    } 

    // lock the rotational DOFs for the edges
    for (idx_t iDof = 0; iDof < rotNames_.size(); iDof++ )
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
    
  
  // set some ground rules depending on where the dispGrad is configured
  BoolMatrix  not_given ( dispGrad_.shape() );
  for (idx_t iDir = 0; iDir < pbcRank_; iDir++)
    for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
      not_given(iDof, iDir) = std::isnan(dispGrad_(iDof, iDir));

  for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
  {
    if (testall(not_given(iDof, ALL)) || ( !not_given(iDof, iDof) && sum(!not_given(iDof, ALL)) == 1 ) ) 
    {
      // if no dispGrad for this DOF is not configured fix normal direction and apply PBC to the shear directions
      // do the same if only the normal DOF for a direction is configured
      System::info( myName_ ) << " ...Locking for no or normal dispGrad w.r.t " << dofNames_[iDof] << "\n";
      for (idx_t iDir = 0; iDir < pbcRank_; iDir++)     
      {
        if (iDof == iDir)
        {
          System::info( myName_ ) << "       at " << PBCGroupInputModule::EDGES[2*iDir] << " to 0\n";
          for (idx_t iNode = 0; iNode < masterDofs_(iDof, iDir).size(); iNode++)
            cons_->addConstraint(masterDofs_(iDof, iDir)[iNode]);
          System::info( myName_ ) << "       at " << PBCGroupInputModule::EDGES[2*iDir+1] << " to each other\n";
          for (idx_t iNode = 1; iNode < slaveDofs_(iDof, iDir).size(); iNode++)
            cons_->addConstraint(slaveDofs_(iDof, iDir)[iNode], slaveDofs_(iDof, iDir)[0], 1.0);
        }
        else
        {
          System::info( myName_ ) << "       at " << PBCGroupInputModule::EDGES[2*iDir+1] << " to " << PBCGroupInputModule::EDGES[2*iDir] << "\n";
          for (idx_t iNode = 0; iNode < masterDofs_(iDof, iDir).size(); iNode++)
            cons_->addConstraint(slaveDofs_(iDof, iDir)[iNode], masterDofs_(iDof, iDir)[iNode], 1.0);
        }
      }
    }
    else if (not_given(iDof, iDof) && sum(!not_given(iDof, ALL)) > 0)
    {
      // if a dispGrad for this DOF is set for a shear term, fix the base for this shear term (and set other shear terms to PBC)
      System::info( myName_ ) << " ...Locking for shear dispGrad w.r.t " << dofNames_[iDof] << "\n";
      for (idx_t iDir = 0; iDir < pbcRank_; iDir++)     
      {
        if (!not_given(iDof, iDir))
        {
          System::info( myName_ ) << "       at " << PBCGroupInputModule::EDGES[2*iDir] << " to 0\n";
          for (idx_t iNode = 0; iNode < masterDofs_(iDof, iDir).size(); iNode++)
            cons_->addConstraint(masterDofs_(iDof, iDir)[iNode]);
        }
        else
        {
          System::info( myName_ ) << "       at " << PBCGroupInputModule::EDGES[2*iDir+1] << " to " << PBCGroupInputModule::EDGES[2*iDir] << "\n";
          for (idx_t iNode = 0; iNode < masterDofs_(iDof, iDir).size(); iNode++)
            cons_->addConstraint(slaveDofs_(iDof, iDir)[iNode], masterDofs_(iDof, iDir)[iNode], 1.0);
        }
      }
    }
    else
    {
      throw jem::Exception( getContext(), "not supported comibnation of displacment gradients given!");
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
    for (idx_t iDir = 0; iDir < pbcRank_; iDir++ )
    {
      if ( std::isnan(dispGrad_(iDof, iDir)) )
        continue;// if the dispGrad for this is not configured, skip it
        
      Globdat::getVariables( "all.extent", globdat ).get( extent, dofNames_[iDir]);

      System::info( myName_ ) << " ...Applying strain in direction of " << dofNames_[iDof] << "\n";
      System::info( myName_ ) << "      of magnitude " << scale*dispGrad_(iDof, iDir) << "\n";
      System::info( myName_ ) << "      between " << PBCGroupInputModule::EDGES[2*iDir] << " and " << PBCGroupInputModule::EDGES[2*iDir+1] << " \n";
      
      for (idx_t iNode = 0; iNode < masterDofs_(iDof, iDir).size(); iNode++)
      {
        // set the master DOF to zero and the slave DOF to the prescribed strain
        cons_->addConstraint(masterDofs_(iDof, iDir)[iNode]);
        cons_->addConstraint(slaveDofs_(iDof, iDir)[iNode],  scale*dispGrad_(iDof, iDir)*extent);
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