/**
 * @file Line3D.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief shape function for implementing a 1D line function in a 3D space
 * @version 0.1
 * @date 2021-10-28
 * 
 * @copyright Copyright (C) 2021 TU Delft. All rights reserved.
 * 
 */

// LATER implement function for norming along a dimension (cubix->matrix, matrix->vector, vector->double)

#include <jem/base/IllegalInputException.h>
#include <jem/base/Array.h>
#include <jem/util/Properties.h>
#include <jem/numeric/algebra/utilities.h>
#include <jem/numeric/algebra/matmul.h>

#include <jive/geom/StdLine.h>
#include <jive/geom/ParametricLine.h>
#include <jive/geom/ShapeFactory.h>

#include "testing.h"
#include "helpers.h"

using namespace jive_helpers;

using jem::Ref;
using jem::newInstance;
using jem::ALL;
using jem::END;
using jem::BEGIN;
using jem::idx_t;
using jem::String;
using jem::SliceTo;
using jem::SliceFrom;
using jem::util::Properties;
using jem::numeric::matmul;
using jem::numeric::norm2;
using jem::numeric::MatmulChain;

using jive::Vector;
using jive::Matrix;
using jive::Cubix;
using jive::geom::Shape;
using jive::geom::ParametricLine;
using jive::geom::StdLine;
using jive::geom::StdLine2;
using jive::geom::StdLine3;
using jive::geom::StdLine4;

/**
 * @brief shape class for a line element in a 3D space
 * 
 * This class provides the shape opportunities needed for a line element in a 3D space,
 * it also provides some functions in order to better control rotational functions
 * along the line.
 * 
 */
class Line3D : public Shape
{
 public:
  static const String PROP_NODES;
  static const String PROP_INT;
  static const String PROP_LOCAL;
  static const idx_t  GLOB_RANK;  

  explicit     Line3D 
  ( const String&       name,
    const Properties&   conf,
    const Properties&   props );

  inline idx_t globalRank() const override;
  inline idx_t localRank() const override;
  inline idx_t nodeCount() const override;
  inline idx_t vertexCount() const override;
  inline idx_t ipointCount() const override;
  inline idx_t shapeFuncCount() const override;

  inline Matrix getVertexCoords() const override;
  inline Matrix getIntegrationScheme() const override;

  inline bool containsLocalPoint
    ( const Vector& u ) const override;

  inline void getIntegrationWeights
    ( const Vector& w,
      const Matrix& c ) const override;

  inline Matrix getVertexFunctions() const override;

  /**
   * @brief get the values of the shape functions at the integration points 
   * @return a matrix h(i,j), where i is the shape function index and j the integration points 
   */
  inline Matrix getShapeFunctions() const override;

  void getGlobalPoint
    ( const Vector& x,
      const Vector& u,
      const Matrix& c ) const override;

  /**
   * @brief get the gradients of the shape functions at the integration points 
   * @param[out] g gradient of the shape functions at the integration points g(i,j) i-th shape function at the j-th int point
   * @param[out] w weights of the integration points
   * @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
   */
  void getShapeGradients
    ( const Matrix& g,
      const Vector& w,
      const Matrix& c ) const;

  /**
   * @brief return the rotations at the integration points  
   * @param[out] Ri Ri(.,.,i) rotational matrix at the integration points 
   * @param[in] Rn rotation at the nodes. Rn(.,.,i) rotational matrix at the nodes
   */
  void getRotations
    ( const Cubix& Ri,
      const Cubix& Rn ) const;

  /**
   * @brief Get the Xi at the integration points
   * 
   * @param[out] Xi Xi(.,.,j,i) where j are the nodes and i are the integration points
   * @param[out] w weights of the integration points
   * @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
   */
  void getXi
    ( const Quadix& Xi,
      const Vector& w,
      const Matrix& c ) const;

  /**
   * @brief Get the Psi at the integration points
   * 
   * @param[out] Psi Psi(.,.,j,i) where j are the nodes and i are the integration points
   * @param[out] w weights of the integration points
   * @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
   */
  void getPsi
    ( const Quadix& Psi,
      const Vector& w,
      const Matrix& c ) const;

  /**
   * @brief Get the rotation gradients at the integration points from the rotations (Crisfield/Jelenic)
   * 
   * @param[out] LambdaP curvature at the integration points
   * @param[out] w integration weights
   * @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
   * @param[in] Rn rotation at the nodes. Rn(.,.,i) rotational matrix at the nodes
   */
  void getRotationGradients
    ( const Cubix& LambdaP,
      const Vector& w,
      const Matrix& c,
      const Cubix& Rn ) const;

//------------------------------------------------
// local location functions
//------------------------------------------------
  inline void evalShapeFunctions
    ( const Vector& h,
      const Vector& u ) const override;

 private:
  /**
   * @brief Get the reference rotation according to Crisfield/Jelenic
   * 
   * @param[out] Lambda_r reference rotation matrix 
   * @param[in] Rn rotation matrices at the nodes 
   */
  void getRefRot_
    ( const Matrix Lambda_r,
      const Cubix Rn ) const;

  /**
   * @brief Get the local node rotation vectors according to Crisfield/Jelenic
   * 
   * @param[out] psi node rotation vectors
   * @param[out] Lambda_r reference rotation matrix 
   * @param[in] Rn rotation matrices at the nodes 
   */
  void getNodeRotVecs_
    ( const Matrix psi,
      const Matrix Lambda_r,
      const Cubix Rn ) const;

 protected:
  Ref<ParametricLine>   intLine_; ///< internal line element for some standard functions
};

//=====================================================================
//    inline implementations
//=====================================================================
idx_t Line3D::globalRank() const
{
  return GLOB_RANK;
}

idx_t Line3D::localRank() const
{
  return intLine_->localRank();
}

idx_t Line3D::nodeCount() const
{
  return intLine_->vertexCount();
}

idx_t Line3D::vertexCount() const
{
  return intLine_->vertexCount();
}

idx_t Line3D::ipointCount() const
{
  return intLine_->ipointCount();
}

idx_t Line3D::shapeFuncCount() const
{
  return intLine_->shapeFuncCount();
}

Matrix Line3D::getVertexCoords() const
{
  return intLine_->getVertexCoords();
}

Matrix Line3D::getIntegrationScheme() const
{
  return intLine_->getIntegrationScheme();
}

bool Line3D::containsLocalPoint
  ( const Vector& u ) const
{
  return intLine_->containsLocalPoint( u );
}

void Line3D::getIntegrationWeights
  ( const Vector& w,
    const Matrix& c ) const
{
  intLine_->getIntegrationWeights( w, c );
}

Matrix Line3D::getVertexFunctions() const
{
  return intLine_->getVertexFunctions();
}

void Line3D::evalShapeFunctions
  ( const Vector& h,
    const Vector& u ) const
{
  JEM_ASSERT2 ( u.size() == localRank(), "internal coordinates do not match the rank" );
  intLine_->evalShapeFunctions( h, u );
}

Matrix Line3D::getShapeFunctions() const
{
  return intLine_->getShapeFunctions();
}