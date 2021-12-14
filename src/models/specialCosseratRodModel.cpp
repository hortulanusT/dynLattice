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

#include "specialCosseratRodModel.h"

//=======================================================================
//    class specialCosseratRodModel -- implementation
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char*   specialCosseratRodModel::TYPE_NAME          = "specialCosseratRod";
const char*   specialCosseratRodModel::SHAPE_IDENTIFIER   = "shape";
const char*   specialCosseratRodModel::TRANS_DOF_DEFAULT  = "trans_";
const char*   specialCosseratRodModel::ROT_DOF_DEFAULT    = "rot_";
const char*   specialCosseratRodModel::YOUNGS_MODULUS     = "young";
const char*   specialCosseratRodModel::SHEAR_MODULUS      = "shear_modulus";
const char*   specialCosseratRodModel::POISSION_RATIO     = "poission_ratio";
const char*   specialCosseratRodModel::AREA               = "area";
const char*   specialCosseratRodModel::AREA_MOMENT        = "area_moment";
const char*   specialCosseratRodModel::POLAR_MOMENT       = "polar_moment";
const char*   specialCosseratRodModel::SHEAR_FACTOR       = "shear_correction";
const char*   specialCosseratRodModel::TRANS_DOF_NAMES    = "dofNamesTrans";
const char*   specialCosseratRodModel::ROT_DOF_NAMES      = "dofNamesRot";
const char*   specialCosseratRodModel::INCREMENTAL        = "incremental";
const char*   specialCosseratRodModel::MATERIAL_Y_DIR     = "material_ey";
const idx_t   specialCosseratRodModel::TRANS_DOF_COUNT    = 3;
const idx_t   specialCosseratRodModel::ROT_DOF_COUNT      = 3;
const Slice   specialCosseratRodModel::TRANS_PART         = jem::SliceFromTo ( 0, TRANS_DOF_COUNT );
const Slice   specialCosseratRodModel::ROT_PART           = jem::SliceFromTo ( TRANS_DOF_COUNT, TRANS_DOF_COUNT+ROT_DOF_COUNT );

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------

specialCosseratRodModel::specialCosseratRodModel

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat ) :

    Model ( name )

{
  // Get the Properties associated with this model
  Properties  myProps = props.findProps ( myName_ );
  Properties  myConf  = conf .makeProps ( myName_ );

  // Get the elements and inodes from the global database.
  egroup_ = ElementGroup::get ( myConf, myProps, globdat, getContext() ); // only the desired group
  elems_  = egroup_.getElements();  // all the elements
  nodes_  = elems_.getNodes (); //all the inodes

  // Initialize the internal shape.
  shape_ = newInstance<Line3D> ( SHAPE_IDENTIFIER, myConf, myProps );

  // Check whether the mesh is valid.
  egroup_.checkElements ( getContext(), shape_->nodeCount() );

  // Define the DOFs.
  Ref<XDofSpace>  dofs = XDofSpace::get ( nodes_.getData(), globdat ); 
  trans_types_.resize ( TRANS_DOF_COUNT );
  rot_types_.resize   ( ROT_DOF_COUNT );
  jtypes_.resize      ( TRANS_DOF_COUNT + ROT_DOF_COUNT );

  // set the default
  Array<String> trans_dofs ( TRANS_DOF_COUNT );
  Array<String> rot_dofs   ( ROT_DOF_COUNT );

  for (idx_t i = 0; i < TRANS_DOF_COUNT; i++) trans_dofs[i] = TRANS_DOF_DEFAULT + String(i);
  for (idx_t i = 0; i < ROT_DOF_COUNT; i++)   rot_dofs[i]   = ROT_DOF_DEFAULT + String(i);

  // get the names
  myProps.find ( trans_dofs, TRANS_DOF_NAMES );
  myProps.find ( rot_dofs, ROT_DOF_NAMES );

  myConf .set  ( TRANS_DOF_NAMES, trans_dofs );
  myConf .set  ( ROT_DOF_NAMES, rot_dofs );  

  // create the DOFs
  for (idx_t i = 0; i < TRANS_DOF_COUNT; i++) trans_types_[i] = dofs->addType( trans_dofs[i] );
  for (idx_t i = 0; i < ROT_DOF_COUNT; i++)   rot_types_[i]   = dofs->addType( rot_dofs[i] );

  jtypes_[ TRANS_PART]= trans_types_;
  jtypes_[ ROT_PART ] = rot_types_;
   
  // Assign the DOFs to the inodes
  for ( idx_t inode = 0; inode < nodes_.size(); inode++ )
  {
    for ( idx_t idof = 0; idof < trans_types_.size(); idof++ ) dofs->addDof ( inode, trans_types_[idof] );
    for ( idx_t idof = 0; idof < rot_types_.size(); idof++ )   dofs->addDof ( inode, rot_types_[idof] );
  }

  // get the nonmutable DOF-Space into the class member
  dofs_ = dofs;

  // get the incremental property
  incremental_ = false;
  myProps.find( incremental_, INCREMENTAL );
  if (incremental_) jem::System::warn() << "incremental update procedure not yet implemented! Setting will be ignored.";
  incremental_ = false;
  myConf .set ( INCREMENTAL, incremental_ );

  // Get the material parameters. //LATER non-isotropic features
  if (myProps.find ( material_ey_, MATERIAL_Y_DIR ))
  {
    JEM_ASSERT( material_ey_.size() == nodes_.rank() );
    JEM_ASSERT( norm2(material_ey_) == 1. );
    myConf.set( MATERIAL_Y_DIR, material_ey_ );
  }

  myProps.get ( young_, YOUNGS_MODULUS );  
  if (!myProps.find ( shearMod_, SHEAR_MODULUS))
  {
    double nu;
    myProps.get ( nu, POISSION_RATIO );
    shearMod_ = young_ / 2. / ( nu + 1.);
  }
  myProps.get ( area_,  AREA );
  myProps.get ( areaMoment_, AREA_MOMENT);
  polarMoment_= 2. * areaMoment_;
  myProps.find( polarMoment_, POLAR_MOMENT);
  shearParam_ = 5./6.;
  myProps.find( shearParam_, SHEAR_FACTOR);

  // Report the used material parameters.
  myConf.set ( YOUNGS_MODULUS, young_ );
  myConf.set ( AREA,  area_ );
  myConf.set ( AREA_MOMENT, areaMoment_ );
  myConf.set ( SHEAR_MODULUS, shearMod_ );
  myConf.set ( POLAR_MOMENT, polarMoment_ );
  myConf.set ( SHEAR_FACTOR, shearParam_ );

  // Prepare linear stiffness matrix //LATER nonlinear material?
  C_material_.resize( 6, 6 );
  C_material_ = 0.0;
  C_material_ ( 0, 0 ) = shearMod_ * shearParam_ * area_;
  C_material_ ( 1, 1 ) = shearMod_ * shearParam_ * area_;
  C_material_ ( 2, 2 ) = young_ * area_;
  C_material_ ( 3, 3 ) = young_ * areaMoment_;
  C_material_ ( 4, 4 ) = young_ * areaMoment_;
  C_material_ ( 5, 5 ) = shearMod_ * polarMoment_;

  // TEST_CONTEXT ( C_material_ )
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------

// Performs an action requested by a module or a parent model.

bool specialCosseratRodModel::takeAction

  ( const String&      action,
    const Properties&  params,
    const Properties&  globdat )

{
  using jive::model::Actions;
  using jive::model::ActionParams;
  using jive::model::StateVector;

  if ( action == Actions::INIT )
  {
    REPORT ( Actions::INIT )
    init_rot_       ();
    TEST_CONTEXT ( LambdaN_ )
    init_strain_    ();
    // TEST_CONTEXT ( mat_strain0_ )
    return true;
  }

  if ( action == Actions::GET_TABLE )
  {       
    Ref<XTable> table;
    Vector      weights;
    String      name;
    // Get the action-specific parameters.
    params.get  ( table,    ActionParams::TABLE ); 
    params.get  ( weights,  ActionParams::TABLE_WEIGHTS);                                                                                                                                                                                                 
    params.get  ( name,     ActionParams::TABLE_NAME);

    // Check whether the requested table is supported by
    // this model.
    if ( table->getRowItems() == elems_.getData() )
    {                                                                              
      Vector              disp;    
      StateVector::get  ( disp, dofs_, globdat );

      if (name=="strain") get_strain_table_ ( *table, weights, disp );
      else if (name=="stress") get_stress_table_ ( *table, weights, disp );
      else if (name=="mat_strain") get_strain_table_ ( *table, weights, disp, true );
      else if (name=="mat_stress") get_stress_table_ ( *table, weights, disp, true );
      else return false;

      return true;
    }
  }

  if ( action == Actions::GET_MATRIX0 )
  {
    Ref<MatrixBuilder>  mbld;
    Vector              fint;
    Vector              disp;
    Vector              dispOld;

    // Get the action-specific parameters.
    params.get ( mbld, ActionParams::MATRIX0 );
    params.get ( fint, ActionParams::INT_VECTOR );
    // TEST_CONTEXT ( fint )

    // Get the current displacements.
    StateVector::get    ( disp, dofs_, globdat );
    StateVector::getOld ( dispOld, dofs_, globdat );
    // TEST_CONTEXT( disp )

    // Assemble the global stiffness matrix together with
    // the internal vector.
    assemble_ ( *mbld, fint, disp, dispOld );

    // //DEBUGGING
    // IdxVector   dofList ( fint.size() );
    // Matrix      K ( fint.size(), fint.size() );
    // Matrix      D ( dofs_->typeCount(), nodes_.size() );
    // Matrix      F ( dofs_->typeCount(), nodes_.size() );
    // for (idx_t i = 0; i<dofList.size(); i++) dofList[i] = i;
    // mbld->getBlock( K, dofList, dofList );
    // vec2mat( D.transpose(), disp );
    // vec2mat( F.transpose(), fint );
    // REPORT( action )
    // TEST_CONTEXT ( D )
    // TEST_CONTEXT ( K )
    // TEST_CONTEXT ( F )

    return true;
  } 
  
  if ( action == Actions::GET_INT_VECTOR )
  {
    Vector              fint;
    Vector              disp;
    Vector              dispOld;

    // Get the action-specific parameters.
    params.get ( fint, ActionParams::INT_VECTOR );

    // Get the current displacements.
    StateVector::get    ( disp, dofs_, globdat );
    StateVector::getOld ( dispOld, dofs_, globdat );

    // Assemble the global stiffness matrix together with
    // the internal vector.
    assembleFint_ ( fint, disp, dispOld );
    
    // //DEBUGGING
    // Matrix      F ( dofs_->typeCount(), nodes_.size() );
    // vec2mat( F.transpose(), fint );
    // TEST_CONTEXT ( F )

    return true;
  }
  
  if ( incremental_ && action == Actions::COMMIT )
  {
    Vector            disp;
    StateVector::get  ( disp, dofs_, globdat );

    update_rot_ ( disp );

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------
//   get_strain_table_
//-----------------------------------------------------------------------
void   specialCosseratRodModel::get_strain_table_

( XTable&               strain_table,
  const Vector&         weights,
  const Vector&         disp,
  const bool            mat_vals  )
{
  const idx_t  elemCount      = egroup_.size();
  const idx_t  nodeCount      = shape_->nodeCount();
  const idx_t  ipCount        = shape_->ipointCount();
  const idx_t  rank           = shape_->globalRank();
  String       dofName        = ""; 

  IdxVector    icols          ( dofs_->typeCount() );
  IdxVector    inodes         ( nodeCount );  

  Matrix       u      ( rank, nodeCount );
  Matrix       theta      ( rank, nodeCount );
  
  Vector       ipWeights      ( ipCount );
  Matrix       ip_phi         ( TRANS_DOF_COUNT, ipCount );
  Cubix        ip_Lambda      ( TRANS_DOF_COUNT, TRANS_DOF_COUNT, ipCount );
  Matrix       ip_phiP        ( TRANS_DOF_COUNT, ipCount );
  Cubix        ip_LambdaP       ( TRANS_DOF_COUNT, TRANS_DOF_COUNT, ipCount );
  Cubix        grads          ( rank, nodeCount, ipCount );
  Matrix       shapes         ( nodeCount, ipCount);

  Matrix       spat_strains   ( TRANS_DOF_COUNT + ROT_DOF_COUNT, ipCount );
  Matrix       mat_strains    ( TRANS_DOF_COUNT + ROT_DOF_COUNT, ipCount );

  // add all the dofs to the Table
  for (idx_t idof = 0; idof < dofs_->typeCount(); idof++)
  {
    dofName = dofs_->getTypeName ( idof );
    if (idof < TRANS_DOF_COUNT) icols[idof] = strain_table.addColumn( "gamma_" + dofName[SliceFrom(dofName.size() - 1)] );
    else                        icols[idof] = strain_table.addColumn( "omega_" + dofName[SliceFrom(dofName.size() - 1)] );
  }  

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    idx_t ielem = egroup_.getIndices()[ie];
    elems_.getElemNodes( inodes, ielem );    
    get_disps_( u, theta, inodes, disp );

    get_strains_( spat_strains, mat_strains, weights, ie, u, theta );

    for (idx_t ip = 0; ip < ipCount; ip++)
    {
      if (!mat_vals) strain_table.addRowValues ( ie, icols, spat_strains ( ALL, ip ) );
      else strain_table.addRowValues ( ie, icols, mat_strains ( ALL, ip ) );
      weights[ie]              += ipWeights[ip];
    }    
  }
}

//-----------------------------------------------------------------------
//   get_stress_table_
//-----------------------------------------------------------------------
void    specialCosseratRodModel::get_stress_table_

( XTable&               stress_table,
  const Vector&         weights,
  const Vector&         disp,
  const bool            mat_vals )
{
  const idx_t  elemCount      = egroup_.size();
  const idx_t  nodeCount      = shape_->nodeCount();
  const idx_t  ipCount        = shape_->ipointCount();
  const idx_t  rank           = shape_->globalRank();
  String       dofName        = ""; 

  IdxVector    icols          ( dofs_->typeCount() );
  IdxVector    inodes         ( nodeCount );

  Matrix       u      ( rank, nodeCount );
  Matrix       theta      ( rank, nodeCount );
  
  Vector       ipWeights      ( ipCount );
  Matrix       Lambda_r       ( TRANS_DOF_COUNT, TRANS_DOF_COUNT );
  Matrix       ip_phi         ( TRANS_DOF_COUNT, ipCount );
  Cubix        ip_Lambda      ( TRANS_DOF_COUNT, TRANS_DOF_COUNT, ipCount );
  Matrix       ip_phiP        ( TRANS_DOF_COUNT, ipCount );
  Cubix        ip_LambdaP     ( TRANS_DOF_COUNT, TRANS_DOF_COUNT, ipCount );
  Cubix        grads          ( rank, nodeCount, ipCount );
  Matrix       shapes         ( nodeCount, ipCount);

  Matrix       spat_strains   ( TRANS_DOF_COUNT + ROT_DOF_COUNT, ipCount );
  Matrix       mat_strains    ( TRANS_DOF_COUNT + ROT_DOF_COUNT, ipCount );
  Matrix       spat_stresses  ( TRANS_DOF_COUNT + ROT_DOF_COUNT, ipCount );
  Matrix       mat_stresses   ( TRANS_DOF_COUNT + ROT_DOF_COUNT, ipCount );

  Vector       el_stresses    ( TRANS_DOF_COUNT + ROT_DOF_COUNT );

  // add all the dofs to the Table
  for (idx_t idof = 0; idof < dofs_->typeCount(); idof++)
  {
    dofName = dofs_->getTypeName ( idof );
    if (idof < TRANS_DOF_COUNT) icols[idof] = stress_table.addColumn( "n_" + dofName[SliceFrom(dofName.size() - 1)] );
    else                        icols[idof] = stress_table.addColumn( "m_" + dofName[SliceFrom(dofName.size() - 1)] );
  }  

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    idx_t ielem = egroup_.getIndices()[ie];
    elems_.getElemNodes( inodes, ielem );    
    get_disps_( u, theta, inodes, disp );

    get_stresses_( spat_stresses, mat_stresses, weights, ie, u, theta );

    for (idx_t ip = 0; ip < ipCount; ip++)
    {
      if (!mat_vals) stress_table.addRowValues ( ie, icols, spat_stresses ( ALL, ip ) );
      else stress_table.addRowValues ( ie, icols, mat_stresses ( ALL, ip ) );
      weights[ie]              += ipWeights[ip];
    }
  }
}

//-----------------------------------------------------------------------
//  init_strain_
//-----------------------------------------------------------------------
void     specialCosseratRodModel::init_strain_ ()
{
  const idx_t   rank          = shape_->globalRank();
  const idx_t   dofCount      = dofs_->typeCount    ();
  const idx_t   ipCount       = shape_->ipointCount ();
  const idx_t   elemCount     = egroup_.size();
  const idx_t   nodeCount     = shape_->nodeCount   ();  
  
// PER ELEMENT VALUES
  Vector        weights       ( ipCount );
// STRAINS
  Matrix        spat_strains  ( dofCount, ipCount );
  Matrix        mat_strains   ( dofCount, ipCount );
  Matrix        null_mat      ( rank, nodeCount );
  null_mat      = 0.;

  mat_strain0_.resize ( dofCount, ipCount, elemCount );
  mat_strain0_ = 0.;

  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    get_strains_( spat_strains, mat_strains, weights, ie, null_mat, null_mat );
    mat_strain0_[ie] = mat_strains;
  }
}

//-----------------------------------------------------------------------
//   update_rot_
//-----------------------------------------------------------------------
void     specialCosseratRodModel::update_rot_ 
  ( const Vector& disp ) // LATER non-straight rods?
{
  const idx_t   elemCount     = egroup_.size();
  const idx_t   rank          = shape_->globalRank();

  IdxVector     iDofs         ( rank );
  Vector        theta         ( rank );
  Matrix        Theta         ( rank, rank );

  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    idx_t     ielem     = egroup_.getIndices()[ie];  
    idx_t     nodeCount = elems_.getElemNodeCount(ie);

    IdxVector iNodes    ( nodeCount );
    Vector    w         ( nodeCount );
    Matrix    coords    ( rank, nodeCount );
    Cubix     Li        ( rank, rank, shape_->ipointCount() );

    elems_.getElemNodes ( iNodes, ielem );
    nodes_.getSomeCoords( coords, iNodes );

    for (idx_t inode = 0; inode < nodeCount; inode++)
    {
      dofs_->getDofIndices( iDofs, iNodes[inode], rot_types_ );
      theta = disp[iDofs];
      expVec( Theta, theta );

      dofs_->getDofIndices( iDofs, iNodes[inode], trans_types_ );
      coords[inode] += disp[iDofs];

      LambdaN_(ALL, ALL, ielem, inode) = matmul ( Theta, LambdaN_(ALL, ALL, ielem, inode));
    }    
  }  
}

//-----------------------------------------------------------------------
//   init_rot_
//-----------------------------------------------------------------------
void     specialCosseratRodModel::init_rot_ () // LATER non-straight rods?
{  
  const idx_t   nodeCount     = nodes_.size();
  const idx_t   elemCount     = egroup_.size();
  const idx_t   elemNodes     = shape_->nodeCount();
  IdxVector     inodes        ( elemNodes );
  IdxVector     allnodes      = egroup_.getNodeIndices();
  Matrix        node_dirs     ( TRANS_DOF_COUNT, elemNodes );
  Matrix        coords        ( TRANS_DOF_COUNT, nodeCount );
  Matrix        rotMat        ( TRANS_DOF_COUNT, TRANS_DOF_COUNT );
  Matrix        oldMat        ( TRANS_DOF_COUNT, TRANS_DOF_COUNT );
  Matrix        newMat        ( TRANS_DOF_COUNT, TRANS_DOF_COUNT );
  Vector        turnRot       ( ROT_DOF_COUNT );
  Vector        delta_phi     ( TRANS_DOF_COUNT );
  Vector        v             ( TRANS_DOF_COUNT );
  Vector        e_x           ( TRANS_DOF_COUNT );
  Vector        e_y           ( TRANS_DOF_COUNT );
  Vector        e_z           ( TRANS_DOF_COUNT );
  double        c;


  LambdaN_.resize  ( TRANS_DOF_COUNT, TRANS_DOF_COUNT, elemCount, elemNodes );
  LambdaN_         = NAN;  
  node_dirs        = 0.;

  nodes_.getCoords( coords );  
  
  // get the direction for each element //ONLY APPROXIMATION FOR NON-STRAIGHT ELEMENTS
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    idx_t ielem = egroup_.getIndices()[ie];
    elems_.getElemNodes ( inodes, ielem );

    node_dirs ( ALL, 0 ) = coords ( ALL, inodes[1] ) - coords ( ALL, inodes[0] );
    for (idx_t in = 1; in < elemNodes-1; in++)
      node_dirs ( ALL, in ) = coords ( ALL, inodes[in+1] ) - coords ( ALL, inodes[in-1] );
    node_dirs ( ALL, elemNodes-1 ) = coords ( ALL, inodes[elemNodes-1] ) - coords ( ALL, inodes[elemNodes-2] );

    for ( idx_t inode = 0; inode < elemNodes; inode++)
    {
      delta_phi = node_dirs ( ALL, inode ) / norm2( node_dirs ( ALL, inode ) );

      if ( material_ey_.size() ) // if the y-direction is given, construct the z direction and then the x-direction
      {
        e_y = material_ey_;
        e_z = delta_phi;
        e_x = matmul( skew(e_y), e_z );

        rotMat[0] = e_x;
        rotMat[1] = e_y;
        rotMat[2] = e_z;
      }
      else // no y-direction given
      {
        v = matmul ( e3, skew ( delta_phi ) );
        c = dotProduct ( delta_phi, e3 );

        rotMat = eye();
        if ( c != -1.) // 180 deg turn == point mirroring
          rotMat += skew ( v ) + 1/( 1 + c ) * matmul ( skew(v), skew(v) ); 
        else
          rotMat *= -1.;
      }

      LambdaN_( ALL, ALL, ie, inode ) = rotMat;    
    }    
  }
}

void specialCosseratRodModel::get_spatialC_
  ( const Cubix&        c,
    const Vector&       w,
    const idx_t&        ie,
    const Matrix&       theta ) const
{
  const idx_t dofCount  = dofs_->typeCount  ();
  const idx_t globRank  = shape_->globalRank();
  const idx_t nodeCount = theta.size(1);
  const idx_t ipCount   = shape_->ipointCount();
  MatmulChain<double, 3> mc3;

  Matrix      Pi_mat      ( dofCount, dofCount );
  IdxVector   elNodes     ( nodeCount );
  Cubix       nodeRots    ( globRank, globRank, nodeCount );
  IdxVector   rotDofs     ( globRank );
  Cubix       ipRots      ( globRank, globRank, ipCount );

  for (idx_t iNode = 0; iNode < nodeCount; iNode++)
  {
    expVec( nodeRots[iNode], theta[iNode] );
    // include initial rotations
    nodeRots[iNode] = matmul(nodeRots[iNode], LambdaN_(ALL, ALL, ie, iNode));
  }
  
  shape_->getRotations( ipRots, nodeRots );

  for (idx_t ip = 0; ip < ipCount; ip++)
  {
    Pi_mat = 0.;
    Pi_mat( TRANS_PART, TRANS_PART ) = ipRots[ip];
    Pi_mat( ROT_PART, ROT_PART ) = ipRots[ip];

    c[ip] = mc3.matmul( Pi_mat, C_material_, Pi_mat.transpose() );
  }  
}

void specialCosseratRodModel::get_geomStiff_
  ( const Cubix&        B,
    const Vector&       w,
    const Matrix&       spat_stresses,
    const Matrix&       coords,
    const Matrix&       u ) const
{
  const idx_t dofCount  = dofs_->typeCount    ();
  const idx_t globRank  = shape_->globalRank();
  const idx_t nodeCount = coords.size(1);
  const idx_t ipCount   = shape_->ipointCount();

  Matrix    shapeGrads  ( nodeCount, ipCount );
  Matrix    nodePhi     ( globRank, nodeCount );
  Matrix    phiP        ( globRank, ipCount );

  // get phi_prime
  shape_->getShapeGradients( shapeGrads, w, coords );
  nodePhi = coords + u;  
  phiP = matmul( nodePhi, shapeGrads );

  // for every iPoint assemble the B-Matrix
  for (idx_t ip = 0; ip < ipCount; ip++)
  {  
    B[ip] = 0.;
    B[ip]( SliceFrom( dofCount ), TRANS_PART ) -= skew( spat_stresses(TRANS_PART, ip) );
    B[ip]( TRANS_PART, SliceFrom( dofCount ) ) += skew( spat_stresses(TRANS_PART, ip) );
    B[ip]( ROT_PART, SliceFrom( dofCount ) )   -= skew( spat_stresses(ROT_PART, ip) );
    B[ip]( SliceFrom( dofCount ), SliceFrom( dofCount ) ) += matmul( spat_stresses(TRANS_PART, ip), phiP[ip] );
    B[ip]( SliceFrom( dofCount ), SliceFrom( dofCount ) ) -= dot( spat_stresses(TRANS_PART, ip), phiP[ip] ) * eye();
  }  
}

void specialCosseratRodModel::get_strains_
  ( const Matrix&       spat_strains,
    const Matrix&       mat_strains,
    const Vector&       w,
    const idx_t&        ie,
    const Matrix&       u,
    const Matrix&       theta ) const
{
  const idx_t ielem     = egroup_.getIndices()[ie];
  const idx_t globRank  = shape_->globalRank();
  const idx_t nodeCount = elems_.getElemNodeCount( ielem );
  const idx_t ipCount   = shape_->ipointCount();

  IdxVector elNodes     ( nodeCount );
  Matrix    coords      ( globRank, nodeCount );
  Matrix    shapeVals   ( nodeCount, ipCount );
  Matrix    shapeGrads  ( nodeCount, ipCount );
  Cubix     Lambda      ( globRank, globRank, ipCount );
  Cubix     LambdaP     ( globRank, globRank, ipCount );
  Matrix    curv        ( globRank, ipCount );
  Cubix     nodeRots    ( globRank, globRank, nodeCount );  
  Matrix    nodePhi     ( globRank, nodeCount );
  Matrix    phiP        ( globRank, ipCount );

  // REPORT( ielem )

  elems_.getElemNodes ( elNodes, ielem );
  nodes_.getSomeCoords( coords, elNodes);

  // TEST_CONTEXT( coords )
  // TEST_CONTEXT( u )
  // TEST_CONTEXT( theta )

  // get position derivative
  shapeVals = shape_->getShapeFunctions();
  shape_->getShapeGradients( shapeGrads, w, coords );
  // TEST_CONTEXT( shapeGrads )
  nodePhi = coords + u;
  phiP = matmul( nodePhi, shapeGrads );
  // TEST_CONTEXT( phiP )
  // get curvature
  for (idx_t iNode = 0; iNode < nodeCount; iNode++)
  {
    expVec( nodeRots[iNode], theta[iNode] );
    // TEST_CONTEXT( nodeRots[iNode] )
    // TEST_CONTEXT( LambdaN_.shape() )
    // TEST_CONTEXT( LambdaN_(ALL, ALL, ie, iNode) )
    // include initial rotations
    nodeRots[iNode] = matmul(nodeRots[iNode], LambdaN_(ALL, ALL, ie, iNode));
    // TEST_CONTEXT( nodeRots[iNode] )
  }
  // TEST_CONTEXT(nodeRots)
  // shape_->getRotStrain_global( curv, w, coords, theta );
  shape_->getRotStrain_local( curv, w, coords, nodeRots );
  shape_->getRotations( Lambda, nodeRots );
  // TEST_CONTEXT(Lambda)

  // TEST_CONTEXT(phiP)
  // TEST_CONTEXT(curv)

  // calculate the material strains + spatial strains
  for (idx_t ip = 0; ip < ipCount; ip++)
  { 
    mat_strains( TRANS_PART, ip )   = matmul( Lambda[ip].transpose(), phiP[ip] );
    mat_strains( ROT_PART, ip )     = matmul( Lambda[ip].transpose(), curv[ip] );
    // TEST_CONTEXT(mat_strains[ip])
    mat_strains[ip]                -= mat_strain0_[ie][ip];
    // TEST_CONTEXT(mat_strains[ip])
    
    spat_strains( TRANS_PART, ip )  = matmul( Lambda[ip], mat_strains( TRANS_PART, ip ) );
    spat_strains( ROT_PART, ip )    = matmul( Lambda[ip], mat_strains( ROT_PART, ip ) );
  }
  // TEST_CONTEXT(mat_strains)
  // TEST_CONTEXT(spat_strains)
}

void specialCosseratRodModel::get_stresses_
  ( const Matrix&       spat_stresses,
    const Matrix&       mat_stresses,
    const Vector&       w,
    const idx_t&        ie,
    const Matrix&       u,
    const Matrix&       theta ) const
{
  const idx_t ielem     = egroup_.getIndices()[ie];
  const idx_t dofCount  = dofs_->typeCount    ();
  const idx_t nodeCount = elems_.getElemNodeCount( ielem );
  const idx_t ipCount   = shape_->ipointCount();

  Cubix       c_spat    ( dofCount, dofCount, ipCount );
  IdxVector   elNodes   ( nodeCount );
  Matrix      spat_strains( dofCount, ipCount );
  Matrix      mat_strains( dofCount, ipCount );

  // get the strains
  get_strains_( spat_strains, mat_strains, w, ie, u, theta );

  // get the stiffness matrices
  get_spatialC_( c_spat, w, ie, theta );

  // get the stresses (material + spatial );
  for (idx_t ip = 0; ip < ipCount; ip++)
  {
    mat_stresses[ip]  = matmul( C_material_, mat_strains[ip] );
    spat_stresses[ip] = matmul( c_spat[ip], spat_strains[ip] ); 
  }
}

void            specialCosseratRodModel::get_disps_
  ( const Matrix&       u,
    const Matrix&       theta,
    const IdxVector&    inodes,
    const Vector&       disp ) const
{
  IdxVector idofs_trans (TRANS_DOF_COUNT);
  IdxVector idofs_rot   (ROT_DOF_COUNT);

  for (idx_t inode = 0; inode < inodes.size(); inode++)
  {
    dofs_->getDofIndices( idofs_trans, inodes[inode], trans_types_ );
    dofs_->getDofIndices( idofs_rot, inodes[inode], rot_types_ );

    u[inode] = disp[idofs_trans];
    theta[inode] = disp[idofs_rot];
  }  
}

void            specialCosseratRodModel::assemble_
  ( MatrixBuilder&        mbld,
    const Vector&         fint,
    const Vector&         disp,
    const Vector&         dispOld ) const
{
  const idx_t  ipCount        = shape_->ipointCount ();
  const idx_t  nodeCount      = shape_->nodeCount   ();
  const idx_t  elemCount      = egroup_.size        ();
  const idx_t  dofCount       = dofs_->typeCount    ();
  const idx_t  rank           = shape_->globalRank  ();  
  MatmulChain<double, 3>      mc3;

// PER ELEMENT VALUEs
  Matrix       coords         ( rank, nodeCount );
  Matrix       u              ( rank, nodeCount );
  Matrix       theta          ( rank, nodeCount );
  Vector       weights        ( ipCount );
  Quadix       XI             ( dofCount, dofCount, nodeCount, ipCount );
  Quadix       PSI            ( dofCount, dofCount+TRANS_DOF_COUNT, nodeCount, ipCount );
  Cubix        c              ( dofCount, dofCount, ipCount );
  Cubix        B              ( dofCount+TRANS_DOF_COUNT, dofCount+TRANS_DOF_COUNT, ipCount );
  Matrix       spat_stresses  ( dofCount, ipCount );
  Matrix       mat_stresses   ( dofCount, ipCount );

// DOF INDICES
  IdxVector    inodes         ( nodeCount );
  IdxVector    Idofs          ( dofCount );
  IdxVector    Jdofs          ( dofCount );

// HELPERS
  Matrix       addS           ( dofCount, dofCount );
  Matrix       addT           ( dofCount, dofCount );

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    // get the global element Index
    idx_t ielem = egroup_.getIndices()[ie];
    elems_.getElemNodes( inodes, ielem );
    nodes_.getSomeCoords( coords, inodes );
    // REPORT(ielem)

    get_disps_( u, theta, inodes, disp );
    // TEST_CONTEXT( u )
    // TEST_CONTEXT( theta )

    // get the XI and PSI values for this 
    shape_->getXi( XI, weights, coords, u );
    // TEST_CONTEXT(XI)
    shape_->getPsi( PSI, weights, coords );
    // TEST_CONTEXT(PSI)
    get_spatialC_( c, weights, ie, theta ); 
    // TEST_CONTEXT(c)
    get_stresses_( spat_stresses, mat_stresses, weights, ie, u, theta );
    // TEST_CONTEXT(spat_stresses)
    get_geomStiff_( B, weights, spat_stresses, coords, u );
    // TEST_CONTEXT(B)
    // TEST_CONTEXT(weights)

    for (idx_t ip = 0; ip < ipCount; ip++)
    {      
      for (idx_t Inode = 0; Inode < nodeCount; Inode++)
      {
        dofs_->getDofIndices ( Idofs, inodes[Inode], jtypes_ ); 
        // TEST_CONTEXT(Idofs) 

        for (idx_t Jnode = 0; Jnode < nodeCount; Jnode++)
        { 
          dofs_->getDofIndices ( Jdofs, inodes[Jnode], jtypes_ ); 
          // TEST_CONTEXT(Jdofs)

          // Stiffness contribution S ( element stiffness matrix )
          addS = weights[ip] * mc3.matmul ( XI(ALL, ALL, Inode, ip), c[ip], XI(ALL, ALL, Jnode, ip).transpose() );
          // TEST_CONTEXT(addS)
          mbld.addBlock( Idofs, Jdofs, addS );

          // Stiffness contribution T ( element geometric stiffness matrix)
          addT = weights[ip] * mc3.matmul ( PSI(ALL, ALL, Inode, ip), B[ip], PSI(ALL, ALL, Jnode, ip).transpose() );
          // TEST_CONTEXT(addT)
          mbld.addBlock( Idofs, Jdofs, addT );
        }
        // TEST_CONTEXT( matmul ( XI(ALL, ALL, Inode, ip), spat_stresses ( ALL, ip ) ) )
        fint[ Idofs ]   += weights[ip] * matmul ( XI(ALL, ALL, Inode, ip), spat_stresses ( ALL, ip ) );
      }    
    }
  }    
}


void            specialCosseratRodModel::assembleFint_
  ( const Vector&         fint,
    const Vector&         disp,
    const Vector&         dispOld ) const
{
  const idx_t  ipCount        = shape_->ipointCount ();
  const idx_t  nodeCount      = shape_->nodeCount   ();
  const idx_t  elemCount      = egroup_.size        ();
  const idx_t  dofCount       = dofs_->typeCount    ();
  const idx_t  rank           = shape_->globalRank  ();

// PER ELEMENT VALUES
  Matrix       coords         ( rank, nodeCount );
  Matrix       u              ( rank, nodeCount );
  Matrix       theta          ( rank, nodeCount );
  Vector       weights        ( ipCount );
  Quadix       XI             ( dofCount, dofCount, nodeCount, ipCount );
  Matrix       spat_stresses  ( dofCount, ipCount );
  Matrix       mat_stresses   ( dofCount, ipCount );

// DOF INDICES
  IdxVector    inodes          ( nodeCount );
  IdxVector    Idofs          ( dofCount );

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    // get the global element Index
    idx_t ielem = egroup_.getIndices()[ie];
    elems_.getElemNodes( inodes, ielem );
    nodes_.getSomeCoords( coords, inodes );

    get_disps_( u, theta, inodes, disp );
    // REPORT(ielem)
 
    shape_->getXi( XI, weights, coords, u );
    get_stresses_( spat_stresses, mat_stresses, weights, ie, u, theta );
    // TEST_CONTEXT( spat_stresses )

    for (idx_t ip = 0; ip < ipCount; ip++)
    {      
      for (idx_t Inode = 0; Inode < nodeCount; Inode++)
      {
        dofs_->getDofIndices ( Idofs, inodes[Inode], jtypes_ );
        // TEST_CONTEXT( matmul ( XI(ALL, ALL, Inode, ip), spat_stresses ( ALL, ip ) ) )
        fint[ Idofs ]   += weights[ip] * matmul ( XI(ALL, ALL, Inode, ip), spat_stresses ( ALL, ip ) );
      }
    }  
  }

}
//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Model>      specialCosseratRodModel::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  return newInstance<specialCosseratRodModel> ( name, conf, props, globdat );
}

//-----------------------------------------------------------------------
//   declarespecialCosseratRodModel
//-----------------------------------------------------------------------

// Registers the specialCosseratRodModel class with the ModelFactory.

void specialCosseratRodModel::declare ()
{
  using jive::model::ModelFactory;

  ModelFactory::declare ( TYPE_NAME, & makeNew );
}