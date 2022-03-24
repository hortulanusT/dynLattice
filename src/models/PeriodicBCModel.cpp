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
const char*      periodicBCModel::MODE_PROP       = "mode";
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
  myProps.find( rotNames_, ROT_NAMES_PROP );
  myConf .set ( ROT_NAMES_PROP, rotNames_ );

  // get the mode name
  String mode = "DISP";
  myProps.find( mode, MODE_PROP );
  myConf .set ( MODE_PROP, mode.toLower() );
  if (mode.toUpper() == "DISP")
  {  
    gradName_ = "H";
    mode_ = DISP;
  }
  else if (mode.toUpper() == "LOAD")
  {
    gradName_ = "P";
    mode_ = LOAD;
  }
  else
    throw jem::IllegalInputException("Unknown mode");

  // get the Gradient  
  // H_ij = du_i/dX_j
  grad_.resize(pbcRank_, pbcRank_);
  grad_ = NAN;
  for (idx_t iDisp = 0; iDisp < pbcRank_; iDisp++)
  {
    for (idx_t iDir = 0; iDir < pbcRank_; iDir++)
    { 
      myProps.find( grad_(iDisp, iDir), gradName_ + String(iDisp+1) + String(iDir+1) );
      myConf .set ( gradName_ + String(iDisp+1) + String(iDir+1), grad_(iDisp, iDir) );
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

  if (action == Actions::GET_CONSTRAINTS && mode_ == DISP)
  {    
    double scale;

    // get the scale factor
    params.get ( scale, ActionParams::SCALE_FACTOR );

    setConstraints_ ( globdat, scale );    
    return true;
  }

  if (action == Actions::GET_EXT_VECTOR && mode_ == LOAD)
  {
    double scale;
    Vector f;

    // get the scale factor & external force vector
    params.get ( f, ActionParams::EXT_VECTOR );

    if (params.find( scale, ActionParams::SCALE_FACTOR ))
      getExtVec_( f, globdat, scale );
    else 
      getExtVec_( f, globdat );

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
  BoolMatrix  not_given ( grad_.shape() );
  for (idx_t iDir = 0; iDir < pbcRank_; iDir++)
    for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
      not_given(iDof, iDir) = std::isnan(grad_(iDof, iDir));

  for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
  {
    if (testall(not_given(iDof, ALL)) || ( !not_given(iDof, iDof) && sum(!not_given(iDof, ALL)) == 1 ) ) 
    {
      // if no dispGrad for this DOF is not configured fix normal direction and apply PBC to the shear directions
      // do the same if only the normal DOF for a direction is configured
      System::info( myName_ ) << " ...Locking for no or normal grad w.r.t " << dofNames_[iDof] << "\n";
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
      System::info( myName_ ) << " ...Locking for shear grad w.r.t " << dofNames_[iDof] << "\n";
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
  // cons_->printTo(jive::util::Printer::get());
  // jive::util::Printer::flush();
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
      if ( std::isnan(grad_(iDof, iDir)) )
        continue;// if the dispGrad for this is not configured, skip it
        
      Globdat::getVariables( "all.extent", globdat ).get( extent, dofNames_[iDir]);

      System::info( myName_ ) << " ...Applying strain in direction of " << dofNames_[iDof] << "\n";
      System::info( myName_ ) << "      of magnitude " << scale*grad_(iDof, iDir) << "\n";
      System::info( myName_ ) << "      between " << PBCGroupInputModule::EDGES[2*iDir] << " and " << PBCGroupInputModule::EDGES[2*iDir+1] << " \n";
      
      for (idx_t iNode = 0; iNode < masterDofs_(iDof, iDir).size(); iNode++)
      {
        // set the slave DOFs to the prescribed strain
        cons_->addConstraint(slaveDofs_(iDof, iDir)[iNode],  scale*grad_(iDof, iDir)*extent);
      }
    }

  // cons_->printTo(jive::util::Printer::get());
  // jive::util::Printer::flush();
}

void      periodicBCModel::getExtVec_
  ( const Vector&         f,
    const Properties&     globdat,
    const double          scale )
{
  double extent = 1.;
  double area   = 1.;
  idx_t  nNodes = 0.;

  for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
    for (idx_t iDir = 0; iDir < pbcRank_; iDir++ )
    {
      // TEST_CONTEXT(f[slaveDofs_(iDof, iDir)])
      if ( std::isnan(grad_(iDof, iDir)) )
        continue;// if the dispGrad for this is not configured, skip it

      area = 1.;
      for (idx_t iDim = 0; iDim < pbcRank_; iDim++)
        if (iDim != iDir)
        {
          try
          {
            Globdat::getVariables( "all.extent", globdat ).get( extent, dofNames_[iDim]);  
          } 
          catch(const jem::util::PropertyException& e)
          {
            Globdat::getVariables ( "SIZE", globdat ).get( extent, jem::String(dofNames_[iDim].back()).toUpper() );
          }
          
          area *= extent;
        }
      nNodes = masterDofs_(iDof, iDir).size();
      // TEST_CONTEXT(area)
      
      System::info( myName_ ) << " ...Applying stress in direction of " << dofNames_[iDof] << "\n";
      System::info( myName_ ) << "      of magnitude " << scale*grad_(iDof, iDir) << "\n";
      System::info( myName_ ) << "      at " << PBCGroupInputModule::EDGES[2*iDir+1] << " \n";
      
      for (idx_t iNode = 0; iNode < masterDofs_(iDof, iDir).size(); iNode++)
      {
        // set the unit load at the slave nodes
        f[slaveDofs_(iDof, iDir)[iNode]] += scale*grad_(iDof, iDir)*area / nNodes;
      }

      // TEST_CONTEXT(f[slaveDofs_(iDof, iDir)])
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