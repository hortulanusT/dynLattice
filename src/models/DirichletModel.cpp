/*
 * 
 *  Copyright (C) 2010 TU Delft. All rights reserved.
 *  
 *  This class implements a model for dirichlet boundary conditions.
 *
 *  Author:  F.P. van der Meer, F.P.vanderMeer@tudelft.nl
 *  Date:    May 2010
 *
 */

#include <jem/base/System.h>
#include <jem/base/Float.h>
#include <jem/numeric/utilities.h>
#include <jem/io/PrintWriter.h>
#include <jive/fem/NodeGroup.h>
#include <jive/model/Actions.h>
#include <jive/model/ModelFactory.h>
#include <jive/util/XDofSpace.h>

#include "DirichletModel.h"
#include "utils/SolverNames.h"
#include "utils/testing.h"

using jem::io::endl;

using jive::fem::NodeGroup;
using jive::model::Actions;
using jive::model::ActionParams;
using jive::util::XDofSpace;


//=======================================================================
//   class DirichletModel
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------


const char*  DirichletModel::TYPE_NAME       = "Dirichlet";

const char*  DirichletModel::DISP_INCR_PROP  = "dispIncr";
const char*  DirichletModel::DISP_RATE_PROP  = "dispRate";
const char*  DirichletModel::INIT_DISP_PROP  = "initDisp";
const char*  DirichletModel::MAX_DISP_PROP   = "maxDisp";
const char*  DirichletModel::NODES_PROP      = "nodeGroups";
const char*  DirichletModel::DOF_PROP        = "dofs";
const char*  DirichletModel::FACTORS_PROP    = "factors";
const char*  DirichletModel::LOADED_PROP     = "loaded";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------


DirichletModel::DirichletModel

  ( const String&      name,
    const Ref<Model>&  child ) :

    Super  ( name  )

{
  dispScale0_  = 0.;
  dispIncr0_   = 0.0;
  initDisp_    = 0.0;
  maxDispVal_  = Float::MAX_VALUE;
  method_      = INCREMENT;
  varName_     = myName_;
  
  idx_t  i     = myName_.find('.');
  while (i>0)
  {
    varName_   = varName_[SliceFrom(i + 1)];
    i          = varName_.find('.');
  }
  varName_     = "var." + varName_ + ".factor";
}


DirichletModel::~DirichletModel ()
{}


//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------


bool DirichletModel::takeAction

  ( const String&      action,
    const Properties&  params,
    const Properties&  globdat )

{
  // initialization

  if ( action == Actions::INIT )
  {
    init_ ( globdat );

    return true;
  }

  // apply displacement increment

  if ( action == Actions::GET_CONSTRAINTS )
  {    
    if (method_ == LOADSCALE) 
      params.get( dispScale_, ActionParams::SCALE_FACTOR );
    
    applyConstraints_ ( params, globdat );

    return true;
  }

  if ( method_ != LOADSCALE)
  {
    // check state

    if ( action == SolverNames::CHECK_COMMIT )
    {
      checkCommit_ ( params, globdat );

      return true;
    }

    // proceed to next time step

    if ( action == Actions::COMMIT )
    {
      commit_ ( params, globdat );

      return true;
    }

    // advance to next time step

    if ( action == Actions::ADVANCE )
    {
      globdat.set ( "var.accepted", true );

      advance_ ( globdat );

      return true;
    }

    // adapt step size

    else if ( action == SolverNames::SET_STEP_SIZE )
    {
      setDT_ ( params );
    }
  }

  return false;
}

//-----------------------------------------------------------------------
//   configure
//-----------------------------------------------------------------------


void DirichletModel::configure

  ( const Properties&  props,
    const Properties&  globdat )

{
  Properties  myProps = props.findProps ( myName_ );

  double maxD = Float::MAX_VALUE;

  if ( myProps.find ( dispRate_, DISP_RATE_PROP ) )
  {
    myProps.find ( initDisp_,  INIT_DISP_PROP );

    method_     = RATE;
    dispScale0_ = dispScale_ = 0.;
  }
  else if ( myProps.find( dispIncr0_, DISP_INCR_PROP ) )
  {
    myProps.find ( initDisp_,  INIT_DISP_PROP );

    method_     = INCREMENT;
    dispIncr_   = dispIncr0_;
    dispScale0_ = dispScale_ = initDisp_ - dispIncr0_;  // cancel first increment
  }
  else
  {
    method_     = LOADSCALE;
  }

  myProps.find ( maxDispVal_,  MAX_DISP_PROP, 0.0, maxD );

  myProps.get( nodeGroups_, NODES_PROP );
  ngroups_ = nodeGroups_.size ( );

  myProps.get( dofTypes_, DOF_PROP );

  if ( dofTypes_.size() != ngroups_ )
  {
    throw IllegalInputException ( JEM_FUNC,
          "dofTypes must have the same length as nodeGroups" );
  }

  if ( myProps.find ( factors_, FACTORS_PROP ) )
  { 
    if ( factors_.size() != ngroups_ )
    {
      throw IllegalInputException ( JEM_FUNC,
            "factors must have the same length as nodeGroups" );
    }
  }
  else
  {
    idx_t loaded;

    factors_.resize ( ngroups_ );

    factors_ = 0.;

    if ( myProps.find( loaded, LOADED_PROP, -1, ngroups_-1 ) )
    {
      factors_[loaded] = 1.;
    }
  }
}


//-----------------------------------------------------------------------
//   getConfig
//-----------------------------------------------------------------------


void DirichletModel::getConfig

  ( const Properties&  conf,
    const Properties&  globdat ) const

{
  Properties  myConf = conf.makeProps ( myName_ );

  switch (method_)
  {
  case INCREMENT:
    myConf.set ( DISP_INCR_PROP, dispIncr0_  );
    myConf.set ( INIT_DISP_PROP, initDisp_   );
    break;
  
  case RATE:    
    myConf.set ( DISP_RATE_PROP, dispRate_   );
    break;

  default:
    break;
  }

  myConf.set ( MAX_DISP_PROP,  maxDispVal_   );

  myConf.set ( NODES_PROP,     nodeGroups_   );
  myConf.set ( DOF_PROP,       dofTypes_     );
  myConf.set ( FACTORS_PROP,   factors_      );
}



//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------


Ref<Model> DirichletModel::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  return newInstance<Self> ( name );
}

//-----------------------------------------------------------------------
//   init_
//-----------------------------------------------------------------------


void DirichletModel::init_ ( const Properties& globdat )
{
  // Get nodes, then dofs of nodes, and constraints of dofs

  nodes_ = NodeSet::find    ( globdat );
  dofs_  = XDofSpace::get   ( nodes_.getData(), globdat );
  cons_  = Constraints::get ( dofs_, globdat );
}

//-----------------------------------------------------------------------
//   advance_
//-----------------------------------------------------------------------

void DirichletModel::advance_

  ( const Properties&  globdat )

{
  if ( method_ == RATE && jem::numeric::abs(dispIncr_) < Float::EPSILON )
  {
    System::warn() << myName_ << " zero increment in RATE mode."
      << " It seems the time increment has not been set." << endl;
  }

  dispScale_   = dispScale0_ + dispIncr_;

  System::info() << " ...New displacement factor " << dispScale_ << endl;
  globdat.set ( varName_, dispScale_ );
}

//-----------------------------------------------------------------------
//   applyConstraints_
//-----------------------------------------------------------------------

void DirichletModel::applyConstraints_

  ( const Properties&  params,
    const Properties&  globdat )

{
  idx_t                 nn;
  Assignable<NodeGroup> group;
  IdxVector             inodes;
  String                context;

  // loop over node groups

  for ( idx_t ig = 0; ig < ngroups_; ig++ )
  {
    group  = NodeGroup::get ( nodeGroups_[ig], nodes_, globdat, context );

    nn     = group.size();

    inodes . resize ( nn );
    inodes = group.getIndices ();

    idx_t itype  = dofs_->findType ( dofTypes_[ig] );

    double val = dispScale_ * factors_[ig];

    // apply constraint

    for ( idx_t in = 0; in < nn; in++ )
    {
      idx_t idof = dofs_->getDofIndex ( inodes[in], itype );

      cons_->addConstraint ( idof, val );
    }
  }

  // compress for more efficient storage

  cons_->compress();
}

//-----------------------------------------------------------------------
//   checkCommit_
//-----------------------------------------------------------------------

void DirichletModel::checkCommit_

  ( const Properties&  params,
    const Properties&  globdat )

{
  // terminate the computation if displacement exceeds maximum.
  // be careful with this!

  if ( jem::numeric::abs ( dispScale_ ) > maxDispVal_ ) 
  {
    System::out() << myName_ << " says: TERMINATE because "
      << " disp > maxDispVal." << endl;

    params.set ( SolverNames::TERMINATE, "sure" );
  }
}

//-----------------------------------------------------------------------
//   commit_
//-----------------------------------------------------------------------


void DirichletModel::commit_

  ( const Properties&  params,
    const Properties&  globdat )

{
  // store converged boundary quantities

  dispScale0_  = dispScale_;
}

//-----------------------------------------------------------------------
//   setDT_
//-----------------------------------------------------------------------

void DirichletModel::setDT_

  ( const Properties&  params )

{
  double       dt;
  double       dt0;

  params.get ( dt,  SolverNames::STEP_SIZE   );
  params.get ( dt0, SolverNames::STEP_SIZE_0 );

  if ( method_ == RATE )
  {
    dispIncr_ = dispRate_ * dt;
  }
  else
  {
    // rate dependent with an initial increment is also supported
    // the displacement rate is then implicit input as dispIncr0/dt0

    dispIncr_  = dispIncr0_ * dt / dt0;
  }
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------


void DirichletModel::declare ()
{
  using jive::model::ModelFactory;

  ModelFactory::declare ( DirichletModel::TYPE_NAME,
                          & DirichletModel::makeNew );
}
