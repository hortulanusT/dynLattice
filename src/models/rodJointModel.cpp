/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *  
 * This class implements a special Cosserat Rod model
 * (also called Simo Reissner Rod or nonlinear
 * Timoshenko Rod)
 *
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: July 21
 *
 */

#include "rodJointModel.h"
//=======================================================================
//    class specialCosseratRodModel -- implementation
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char*   rodJointModel::TYPE_NAME          = "rodJoint";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------

rodJointModel::rodJointModel

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat ) :

    Model ( name )
    
{
  // Get the Properties associated with this model
  Properties  myProps = props.findProps ( myName_ );
  Properties  myConf  = conf .makeProps ( myName_ );

  // Get the elements and nodes from the global database.
  egroup_       = ElementGroup::get ( myConf, myProps, globdat, getContext() ); // only the desired group
  elems_        = egroup_.getElements();  // all the elements
  nodes_        = elems_.getNodes (); //all the nodes  
  dofs_         = DofSpace::get ( globdat, getContext() ); //all the dofs 
  constraints_  = Constraints::get ( dofs_, globdat ); //all the constraints

  // get the dofs to be locked
  if ( myProps.find ( lockDofs_, "lockDofs" ) )
       myConf .set  ( "lockDofs", lockDofs_ );
  
  // get the dofs to be connected by a spring (rotational)
  if (myProps.find ( rotDofs_, "rotDofs" ))
  {
    JEM_PRECHECK ( rotDofs_.size() == 3 );
    iRotDofs_.resize ( rotDofs_.size() );

    myConf .set ( "rotDofs", rotDofs_ );  
    myProps.get ( rot_stiff_, "rotStiffness" );
    myConf .set ( "rotStiffness", rot_stiff_ );
  }

  // get the dofs to be connected by a spring (translational)
  if (myProps.find ( transDofs_, "transDofs" ))
  {
    JEM_PRECHECK ( transDofs_.size() == 3 );
    iTransDofs_.resize ( transDofs_.size() );

    myConf .set ( "transDofs", transDofs_ );  
    myProps.get ( trans_stiff_, "transStiffness" );
    myConf .set ( "transStiffness", trans_stiff_ );
  }
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------

// Performs an action requested by a module or a parent model.

bool rodJointModel::takeAction

  ( const String&      action,
    const Properties&  params,
    const Properties&  globdat )

{
  using jive::model::Actions;
  using jive::model::ActionParams;
  using jive::model::StateVector;

  if ( action == Actions::INIT )
  {
    init_ ();
    return true;
  }

  if ( action == Actions::GET_CONSTRAINTS )
  {
    getCons_ ();

    TEST_CONTEXT(constraints_->toMatrix())

    return true;
  }

  if ( action == Actions::GET_MATRIX0 )
  {
    Ref<MatrixBuilder>  mbld;
    Vector              fint;
    Vector              disp;

    // Get the action-specific parameters.
    params.get ( mbld, ActionParams::MATRIX0 );
    params.get ( fint, ActionParams::INT_VECTOR );

    // Get the current displacements.
    StateVector::get    ( disp, dofs_, globdat );

    // Assemble the global stiffness matrix together with
    // the internal vector.
    if (rotDofs_.size() > 0) assembleRot_ ( *mbld, fint, disp );
    if (transDofs_.size() > 0) assembleTrans_ ( *mbld, fint, disp );

    // //DEBUGGING
    // IdxVector   dofList ( fint.size() );
    // Matrix      K ( fint.size(), fint.size() );
    // for (idx_t i = 0; i<dofList.size(); i++) dofList[i] = i;
    // mbld->getBlock( K, dofList, dofList );
    // TEST ( disp )
    // TEST ( K )
    // TEST ( fint )

    return true;
  }

  return false;
}


//-----------------------------------------------------------------------
//   assembleRot_
//-----------------------------------------------------------------------
void  rodJointModel::assembleRot_

  ( MatrixBuilder&        mbld,
    const Vector&         fint,
    const Vector&         disp )
{
  idx_t     elemCount   = egroup_.size();

  IdxVector inodes;
  IdxVector idofs       ( 3 );
  Vector    rot_i       ( 3 );
  Matrix    Lambda_i    ( 3, 3 );
  IdxVector jdofs       ( 3 );
  Vector    rot_j       ( 3 );
  Matrix    Lambda_j    ( 3, 3 );

  Matrix    deltaLambda ( 3, 3 );
  Vector    delta_r     ( 3 );
  Vector    torque      ( 3 );

  Matrix    stiff_mat   ( 3, 3 );

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    idx_t ielem         = egroup_.getIndices()[ie];
    idx_t nodeCount     = elems_.getElemNodeCount( ielem );
    inodes.resize       ( nodeCount );
    elems_.getElemNodes ( inodes, ielem );

    stiff_mat           = (Matrix) (rot_stiff_ / binom(nodeCount, 2) * eye()); // LATER get something more meaningfull???

    // iterate through the nodes
    for (idx_t inode = 0; inode < nodeCount; inode++)
    {
      dofs_->getDofIndices ( idofs, inodes[inode], iRotDofs_ );

      rot_i          = disp [ idofs ];
      expVec         ( Lambda_i, rot_i );

      // iterate through the other nodes
      for (idx_t jnode = inode+1; jnode < nodeCount; jnode++)
      {
        dofs_->getDofIndices ( jdofs, inodes[jnode], iRotDofs_ );
        
        rot_j          = disp [ jdofs ];
        expVec         ( Lambda_j, rot_j );

        deltaLambda    = matmul ( Lambda_i.transpose(), Lambda_j );
        logMat  ( delta_r, deltaLambda );

        torque         = matmul ( stiff_mat,  delta_r );

        // add torque
        fint [ idofs ] -= torque;
        fint [ jdofs ] += torque;

        // fill stiffness matrix  
        mbld.addBlock  ( idofs, idofs, (Matrix)(+1. * stiff_mat) );
        mbld.addBlock  ( idofs, jdofs, (Matrix)(-1. * stiff_mat) );   
        mbld.addBlock  ( jdofs, idofs, (Matrix)(-1. * stiff_mat) );   
        mbld.addBlock  ( jdofs, jdofs, (Matrix)(+1. * stiff_mat) );

        // REPORT2 ( inodes[inode], inodes[jnode] )
        // TEST2 ( torque, stiff_mat )
      }     
    }    
  }
}

//-----------------------------------------------------------------------
//   assembleTrans_
//-----------------------------------------------------------------------
void  rodJointModel::assembleTrans_

  ( MatrixBuilder&        mbld,
    const Vector&         fint,
    const Vector&         disp )
{
  idx_t     elemCount   = egroup_.size();

  IdxVector inodes;
  IdxVector idofs       ( 3 );
  Vector    disp_i      ( 3 );
  IdxVector jdofs       ( 3 );
  Vector    disp_j      ( 3 );

  Vector    delta       ( 3 );
  Vector    force       ( 3 );

  Matrix    stiff_mat   ( 3, 3 );

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    idx_t ielem         = egroup_.getIndices()[ie];
    idx_t nodeCount     = elems_.getElemNodeCount( ielem );
    inodes.resize       ( nodeCount );
    elems_.getElemNodes ( inodes, ielem );

    stiff_mat           = (Matrix) (trans_stiff_ / binom(nodeCount, 2) * eye());

    // iterate through the nodes
    for (idx_t inode = 0; inode < nodeCount; inode++)
    {
      dofs_->getDofIndices ( idofs, inodes[inode], iTransDofs_ );

      disp_i          = disp [ idofs ];

      // iterate through the other nodes
      for (idx_t jnode = inode+1; jnode < nodeCount; jnode++)
      {
        dofs_->getDofIndices ( jdofs, inodes[jnode], iTransDofs_ );
        
        disp_j         = disp [ jdofs ];
        
        delta          = disp_i - disp_j;

        force          = matmul ( stiff_mat,  delta );

        // add torque
        fint [ idofs ] -= force;
        fint [ jdofs ] += force;

        // fill stiffness matrix  
        mbld.addBlock  ( idofs, idofs, (Matrix)(+1. * stiff_mat) );
        mbld.addBlock  ( idofs, jdofs, (Matrix)(-1. * stiff_mat) );   
        mbld.addBlock  ( jdofs, idofs, (Matrix)(-1. * stiff_mat) );   
        mbld.addBlock  ( jdofs, jdofs, (Matrix)(+1. * stiff_mat) );
      }     
    }    
  }
}

//-----------------------------------------------------------------------
//   init_
//-----------------------------------------------------------------------

void rodJointModel::init_ ()
{
  // get all the spring dofs and store in array
  for (idx_t iDof = 0; iDof < rotDofs_.size(); iDof++)    iRotDofs_[iDof]   = dofs_->getTypeIndex ( rotDofs_[iDof] );    
  for (idx_t iDof = 0; iDof < transDofs_.size(); iDof++)  iTransDofs_[iDof] = dofs_->getTypeIndex ( transDofs_[iDof] );    
}

//-----------------------------------------------------------------------
//   init_
//-----------------------------------------------------------------------

void rodJointModel::getCons_ ()
{
  IdxVector   inodes      ( 0 );
  IdxVector   idofs       ( lockDofs_.size() );

  idx_t       masterNode  = -1;
  IdxVector   masterDofs  ( lockDofs_.size() );
  idx_t       slaveNode   = -1;
  IdxVector   slaveDofs   ( lockDofs_.size() );

  // lock all dofs in the array
  for (idx_t iDof = 0; iDof < lockDofs_.size(); iDof++)
  {
    idofs[iDof] = dofs_->getTypeIndex ( lockDofs_[iDof] ); 
  }  
  // iterate through all the elements
  for (idx_t ie = 0; ie < egroup_.size(); ie++)
  {
    idx_t ielem = egroup_.getIndices()[ie];      
    // get the nodes of the element
    inodes.resize ( elems_.getElemNodeCount ( ielem ) );
    elems_.getElemNodes ( inodes, ielem );
    // define the first node of the element as master node
    masterNode = inodes [0];
    dofs_->getDofIndices ( masterDofs, masterNode, idofs );

    for (idx_t in = 1; in < elems_.getElemNodeCount ( ielem ); in++)
    {
      slaveNode = inodes [in];
      dofs_->getDofIndices ( slaveDofs, slaveNode, idofs );
      
      //iterate through the dofs
      for (idx_t id = 0; id < lockDofs_.size(); id++)
      {
        constraints_->addConstraint ( slaveDofs[id], masterDofs[id], 1.);
      }
    }    
  }   
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Model>      rodJointModel::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  return newInstance<rodJointModel> ( name, conf, props, globdat );
}

//-----------------------------------------------------------------------
//   declare rodJointModel
//-----------------------------------------------------------------------

// Registers the rodJointModel class with the ModelFactory.

void    rodJointModel::declare ()
{
  using jive::model::ModelFactory;

  ModelFactory::declare ( TYPE_NAME, & makeNew );
}