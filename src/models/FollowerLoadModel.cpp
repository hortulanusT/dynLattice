/*
 * 
 *  Copyright (C) 2010 TU Delft. All rights reserved.
 *  
 *  This class implements a model for a follower load
 * 
 *  Author:  T. Gärtner t.gartner@tudelft.nl
 *  Date:    September 2021
 *
 */

#include "FollowerLoadModel.h"

//=======================================================================
//   class FollowerLoadModel
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------


const char*  FollowerLoadModel::TYPE_NAME       = "FollowerLoad";

const char*  FollowerLoadModel::NODES_PROP      = "nodeGroup";
const char*  FollowerLoadModel::START_PROP      = "start_dir";
const char*  FollowerLoadModel::DOF_PROP        = "dofs";
const char*  FollowerLoadModel::ROT_DOF_PROP    = "rot_dofs";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------
FollowerLoadModel::FollowerLoadModel

( const String&         name,
  const Properties&     conf,
  const Properties&     props,
  const Properties&     globdat ) :
        Model ( name )
{
  // Get the Properties associated with this model
  Properties  myProps = props.findProps ( myName_ );
  Properties  myConf  = conf .makeProps ( myName_ );

  // get the global nodes
  nodes_ = NodeSet::find    ( globdat );

  // get all the properties
  myProps.get ( nameGroup_, NODES_PROP );
  myConf .set ( NODES_PROP, nameGroup_ );

  myProps.get ( org_dir_, START_PROP );
  myConf .set ( START_PROP, org_dir_ );

  myProps.get ( force_dofs_, DOF_PROP );
  myConf .set ( DOF_PROP, force_dofs_ );

  myProps.get ( rot_dofs_, ROT_DOF_PROP );
  myConf .set ( ROT_DOF_PROP, rot_dofs_ );
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------
bool FollowerLoadModel::takeAction

( const String&         action,
  const Properties&     params,
  const Properties&     globdat )
{
  if ( action == Actions::INIT )
  {    
    nodes_ = NodeSet::find    ( globdat ); 
    group_ = NodeGroup::get   ( nameGroup_, nodes_, globdat, getContext() );
    dofs_  = DofSpace::get   ( nodes_.getData(), globdat, getContext() );

    idofs_f_.resize ( force_dofs_.size() );
    idofs_r_.resize ( rot_dofs_.size() );
    rot_Mats_.resize ( force_dofs_.size(), force_dofs_.size(), group_.size() );

    for (idx_t i = 0; i < idofs_f_.size(); i++)  idofs_f_[i]  = dofs_->getTypeIndex(force_dofs_[i]);
    for (idx_t i = 0; i < idofs_r_.size(); i++)  idofs_r_[i]  = dofs_->getTypeIndex(rot_dofs_[i]);
    for (idx_t i = 0; i < group_.size(); i++)    rot_Mats_[i] = eye(); 

    return true;
  }

  if ( action == Actions::ADVANCE )
  {
    Vector d;

    // Get the last loadstep displacements
    StateVector::getOld ( d, dofs_, globdat );

    advance_rots_ ( d );

    return true;
  }

  if ( action == Actions::GET_EXT_VECTOR )
  {
    Vector f;
    double scale;

    // get the external vector
    params.get ( f, ActionParams::EXT_VECTOR );
    // get the scale factor
    params.get ( scale, ActionParams::SCALE_FACTOR );

    System::info() << " ...Scale Factor for " << myName_ << ": " << scale << "\n";

    get_ext_vec_ ( f, scale, globdat );

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------
//   get_ext_vec_
//-----------------------------------------------------------------------
void FollowerLoadModel::get_ext_vec_ 
  ( const Vector&     fext,
    const double      scale,
    const Properties& globdat ) const
{
  IdxVector inodes  ( group_.size() );
  IdxVector if_dofs ( idofs_f_.size() );

  Matrix    Lambda  ( idofs_f_.size(), idofs_f_.size() );
  
  // get the node group indices
  inodes = group_.getIndices();

  for (idx_t inode = 0; inode < inodes.size(); inode++)
  {
    dofs_->getDofIndices ( if_dofs, inodes[inode], idofs_f_ );

    fext[if_dofs] += scale * matmul( rot_Mats_[inode], org_dir_ );
  }  
}

//-----------------------------------------------------------------------
//   advance_rots
//-----------------------------------------------------------------------
void FollowerLoadModel::advance_rots_ 
  ( const Vector&     d ) const
{
  IdxVector inodes  ( group_.size() );
  IdxVector ir_dofs ( idofs_r_.size() );

  Matrix    Lambda  ( idofs_f_.size(), idofs_f_.size() );
  
  // get the node group indices
  inodes = group_.getIndices();
  
  // update the rotations
  for (idx_t inode = 0; inode < inodes.size(); inode++)
  {
    dofs_->getDofIndices ( ir_dofs, inodes[inode], idofs_r_ );

    // TEST_CONTEXT(d[ir_dofs])
    expVec ( rot_Mats_[inode], (Vector)d[ir_dofs] );
  }  
  // TEST_CONTEXT(rot_Mats_)
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------
Ref<Model> FollowerLoadModel::makeNew

( const String&      name,
  const Properties&  conf,
  const Properties&  props,
  const Properties&  globdat )
{
  return newInstance<FollowerLoadModel> ( name, conf, props, globdat );
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------
void FollowerLoadModel::declare ()
{
  using jive::model::ModelFactory;

  ModelFactory::declare ( TYPE_NAME, & makeNew );
}