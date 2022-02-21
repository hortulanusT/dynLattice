/**
 * @file Line3D.cpp
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief shape function for implementing a 1D line function in a 3D space
 * @version 0.1
 * @date 2021-10-28
 * 
 * @copyright Copyright (C) 2021 TU Delft. All rights reserved.
 * 
 */

#include "Line3D.h"
#include <jem/base/ClassTemplate.h>

const String  Line3D::PROP_NODES  = "numPoints";
const String  Line3D::PROP_INT    = "intScheme";
const idx_t   Line3D::GLOB_RANK   = 3;

Line3D::Line3D 
  ( const String&       name,
    const Properties&   conf,
    const Properties&   props )
    : Shape( name )
{
  idx_t         numNodes;
  String        ischemeName;

  Properties myProps = props.getProps( name );
  Properties myConf  = conf.makeProps( name );
  
  myProps.get( numNodes, PROP_NODES );
  myConf .set( PROP_NODES, numNodes );

  Ref<StdLine> stdline = nullptr;
  switch(numNodes)
  {
    case 2: stdline = newInstance<StdLine2>(); break;
    case 3: stdline = newInstance<StdLine3>(); break;
    case 4: stdline = newInstance<StdLine4>(); break;
    default: throw jem::IllegalInputException( getContext(), "given number of nodes not supported!");
  }

  if (!myProps.find( ischemeName, PROP_INT ))
    ischemeName = String::format("Gauss%d", numNodes-1); // reduced integration by default
  myConf .set( PROP_INT, ischemeName );

  Matrix ischeme = StdLine::getIntegrationScheme( ischemeName );

  intLine_ = newInstance<ParametricLine>( "internalLine", ischeme, stdline );
}

void Line3D::getGlobalPoint
  ( const Vector& x,
    const Vector& u,
    const Matrix& c ) const
{
  const Vector shapeFuncs;
  intLine_->evalShapeFunctions( shapeFuncs, u );

  x = matmul( c, shapeFuncs );
}

void Line3D::getShapeGradients
  ( const Matrix& g,
    const Vector& w,
    const Matrix& c ) const
{
  Matrix c1( localRank(), nodeCount() );
  c1( 0,0 ) = 0.;
  for (idx_t iNode = 1; iNode < nodeCount(); iNode++) //LATER get something more elaborate that also includes real shapes for higher order elements
    c1( 0, iNode ) = norm2( c[iNode] - c[iNode-1] );

  // TEST_CONTEXT(c1)

  Cubix     grads ( localRank(), shapeFuncCount(), ipointCount() ); 
  intLine_->getShapeGradients( grads, w, c1 );
  g = grads( 0, ALL, ALL );
}

void Line3D::getRotations
  ( const Cubix& Ri,
    const Matrix& theta ) const
{
  Cubix   Rn            ( globalRank(), globalRank(), nodeCount() );
  Matrix  Lambda_r      ( globalRank(), globalRank() );
  Matrix  node_psi      ( globalRank(), nodeCount() );
  Matrix  ip_psi        ( globalRank(), ipointCount() );

  for (idx_t inode = 0; inode<nodeCount(); inode++) expVec( Rn[inode], theta[inode] );

  // get the Rotaion matrices and the relative rotations between them 
  getNodeRotVecs_( node_psi, Lambda_r, Rn );
  
  // get the _local_ rotations in the integration points
  Matrix  shapeFuncs    = getShapeFunctions();

  // get the rotation vectors associated with the local rotations
  ip_psi  = matmul( node_psi, shapeFuncs );
  
  // construc the local rotation matrices
  for (idx_t iIp = 0; iIp < ipointCount(); iIp++)
  {
    expVec( Ri[iIp], ip_psi[iIp] );
    Ri[iIp] = matmul( Lambda_r, Ri[iIp] );
  }
}

void Line3D::getRotations
  ( const Cubix& Ri,
    const Cubix& Rn, 
    const Matrix& theta ) const
{
  getRotations( Ri, theta );

  for (idx_t ip = 0; ip < ipointCount(); ip++) Ri[ip] = matmul( Ri[ip], Rn[ip] );
}

void Line3D::getXi
  ( const Quadix& Xi,
    const Vector& w,
    const Matrix& u,
    const Matrix& c ) const
{
  JEM_ASSERT2 ( Xi.size(0) == 6 && Xi.size(1) == 6 && Xi.size(2) == nodeCount() && Xi.size(3) == ipointCount(), "Xi size does not match the expected size" );
  Matrix  shapes  = getShapeFunctions();
  Matrix  grads   ( shapeFuncCount(), ipointCount() );
  getShapeGradients( grads, w, c );

  Matrix  phiP    = matmul( (Matrix)(c+u), grads );

  Xi = 0.;

  for (idx_t ip = 0; ip < ipointCount(); ip++)
    for (idx_t iNode = 0; iNode < nodeCount(); iNode++)
    {
      for (idx_t i = 0; i < 6; i++)   Xi(i, i, iNode, ip) = grads( iNode, ip );
      Xi( SliceFrom(3), SliceTo(3), iNode, ip ) = -1. * shapes(iNode, ip) * skew(phiP[ip]);
    }
}

void Line3D::getPsi
  ( const Quadix& Psi,
    const Vector& w,
    const Matrix& c ) const
{
  JEM_ASSERT2 ( Psi.size(0) == 6 && Psi.size(1) == 9 && Psi.size(2) == nodeCount() && Psi.size(3) == ipointCount(), "Psi size does not match the expected size" );
  Matrix  shapes  = getShapeFunctions();
  Matrix  grads   ( shapeFuncCount(), ipointCount() );
  getShapeGradients( grads, w, c );

  Psi = 0.;

  for (idx_t ip = 0; ip < ipointCount(); ip++)
  {
    for (idx_t iNode = 0; iNode < nodeCount(); iNode++)
    {
      for (idx_t i = 0; i < 6; i++) Psi(i, i, iNode, ip) = grads( iNode, ip );
      for (idx_t i = 0; i < 3; i++) Psi(i+3, i+6, iNode, ip ) = shapes( iNode, ip );
    }
  }    
}

void Line3D::getPi
    ( const Cubix& Pi,
      const Matrix& theta ) const
{
  const Cubix Ri  ( globalRank(), globalRank(), ipointCount() );

  getRotations( Ri, theta );

  Pi  = 0.;
  for (idx_t ip = 0; ip < ipointCount(); ip++)
  {
    Pi[ip](SliceTo(3), SliceTo(3))      = Ri[ip];
    Pi[ip](SliceFrom(3), SliceFrom(3))  = Ri[ip];
  }  
}

void Line3D::getPi
    ( const Cubix& Pi,
      const Cubix& Rn,
      const Matrix& theta ) const
{
  const Cubix Ri  ( globalRank(), globalRank(), ipointCount() );

  getRotations( Ri, Rn, theta );

  Pi  = 0.;
  for (idx_t ip = 0; ip < ipointCount(); ip++)
  {
    Pi[ip](SliceTo(3), SliceTo(3))      = Ri[ip];
    Pi[ip](SliceFrom(3), SliceFrom(3))  = Ri[ip];    
  } 
}

void Line3D::getRotationGradients
  ( const Cubix& LambdaP,
    const Vector& w,
    const Matrix& c,
    const Matrix& theta ) const
{
  JEM_ASSERT2 ( LambdaP.size(0) == globalRank() && LambdaP.size(1) == globalRank() && LambdaP.size(2) == ipointCount(), "LambdaP size does not match the expected size" );

  Cubix   Rn            ( globalRank(), globalRank(), nodeCount() );
  Matrix  Lambda_r      ( globalRank(), globalRank() );
  Matrix  node_psi      ( globalRank(), nodeCount() );
  Matrix  ip_psi        ( globalRank(), ipointCount() );
  
  for (idx_t inode = 0; inode<nodeCount(); inode++) expVec( Rn[inode], theta[inode] );

  // get the Rotaion matrices and the relative rotations between them 
  getNodeRotVecs_( node_psi, Lambda_r, Rn );
    
  Matrix  shapes  = getShapeFunctions();
  Matrix  grads   ( shapeFuncCount(), ipointCount() );
  getShapeGradients( grads, w, c );

  Matrix  psi     ( globalRank(), ipointCount() );
  Matrix  psiP    ( globalRank(), ipointCount() );
  psi     = matmul( node_psi, shapes );
  psiP    = matmul( node_psi, grads );

  for (idx_t ip = 0; ip < ipointCount(); ip++)
  {
    expVecP( LambdaP[ip], psi[ip], psiP[ip] );
    LambdaP[ip]   = matmul( Lambda_r, LambdaP[ip] );
  } 
}

void Line3D::getRotationGradients
  ( const Cubix& LambdaP,
    const Vector& w,
    const Cubix& Rn,
    const Matrix& c,
    const Matrix& theta ) const
{
  getRotationGradients( LambdaP, w, c, theta );

  for (idx_t ip = 0; ip < ipointCount(); ip++) LambdaP[ip] = matmul( LambdaP[ip], Rn[ip] );
}


//------------------------------------------------------
// private helper functions
//------------------------------------------------------
void Line3D::getRefRot_
  ( const Matrix Lambda_r,
    const Cubix Rn ) const
{
  // first construct the reference rotation (use 0-based numbers instead of Crisfield/Jelenic 1-based numbers)
  const idx_t I   = idx_t(0.5*(nodeCount()-1)); 
  const idx_t J   = idx_t(0.5*(nodeCount()+0));
  const double c  = 0.5;

  Vector phi_IJ   ( globalRank() );

  logMat( phi_IJ, matmul( Rn[I].transpose(), Rn[J] ) );
  phi_IJ = c*phi_IJ;
  expVec( Lambda_r, phi_IJ );
  Lambda_r = matmul( Rn[I], Lambda_r );
}

void Line3D::getNodeRotVecs_
  ( const Matrix psi,
    const Matrix Lambda_r,
    const Cubix Rn ) const
{
  getRefRot_( Lambda_r, Rn );

  for (idx_t iNode = 0; iNode < nodeCount(); iNode++)
    logMat( psi[iNode], matmul( Lambda_r.transpose(), Rn[iNode] ) ); 
}