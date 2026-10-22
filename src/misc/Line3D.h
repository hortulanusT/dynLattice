/**
 * @file Line3D.h
 * @author Til Gärtner
 * @brief Shape function for 1D line elements in 3D space.
 */

#pragma once

#include "utils/helpers.h"
#include <jem/numeric/algebra/MatmulChain.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/geom/ParametricLine.h>
#include <jive/geom/Shape.h>
#include <jive/geom/StdShape.h>

using jem::idx_t;
using jem::String;
using jem::util::Properties;
using jem::numeric::MatmulChain;
using jive::Cubix;
using jive::Matrix;
using jive::Vector;
using jive::geom::ParametricLine;
using jive::geom::Shape;
using jive_helpers::Quadix;

/// @brief Shape function for 1D line elements in 3D space.
///
/// Provides shape functions and geometric operations for line elements embedded
/// in 3D space. Supports 2, 3, and 4-node line elements with configurable
/// integration schemes and rotational control functions.
///
/// @section SupportedElements Supported Elements
/// - 2-node linear line (StdLine2)
/// - 3-node quadratic line (StdLine3)
/// - 4-node cubic line (StdLine4)
///
/// @author Til Gärtner
/// @see [Line Shapes](https://jive-manual.dynaflow.com/group__LineShapes.html)
class Line3D : public Shape
{
 public:
  static const String PROP_NODES; ///< Property name for number of nodes
  static const String PROP_INT;   ///< Property name for integration scheme
  static const String PROP_LOCAL; ///< Property name for local coordinates
  static const idx_t GLOB_RANK;   ///< Global rank (always 3 for 3D space)

  /// @brief Constructs a new Line3D shape object.
  ///
  /// @param[in] name Shape instance name for identification
  /// @param[out] conf Configuration properties container for output
  /// @param[in] props Input properties containing shape parameters
  /// @throws jem::IllegalInputException if unsupported number of nodes is specified
  explicit Line3D(const String &name,
                  const Properties &conf,
                  const Properties &props);

  /// @brief Get the global rank (spatial dimension).
  /// @return Global rank (always 3 for 3D space)
  inline idx_t globalRank() const override;

  /// @brief Get the local rank of the line element.
  /// @return Local coordinate dimension
  inline idx_t localRank() const override;

  /// @brief Get the number of nodes in the line element.
  /// @return Number of nodes
  inline idx_t nodeCount() const override;

  /// @brief Get the number of vertices in the line element.
  /// @return Number of vertices
  inline idx_t vertexCount() const override;

  /// @brief Get the number of integration points.
  /// @return Number of integration points
  inline idx_t ipointCount() const override;

  /// @brief Get the number of shape functions.
  /// @return Number of shape functions
  inline idx_t shapeFuncCount() const override;

  /// @brief Get the local coordinates of the vertices.
  /// @return Matrix of vertex coordinates
  inline Matrix getVertexCoords() const override;

  /// @brief Get the integration scheme.
  /// @return Matrix describing the integration scheme
  inline Matrix getIntegrationScheme() const override;

  /// @brief Check whether a local point is inside the element.
  /// @param[in] u Local coordinate vector
  /// @return Whether the point is contained in the element
  inline bool containsLocalPoint(const Vector &u) const override;

  /// @brief Compute integration weights for numerical integration.
  /// @param[out] w Integration weights at the integration points
  /// @param[in] c Node coordinates matrix, c(i,j) is the i-th coordinate of the j-th node
  void getIntegrationWeights(const Vector &w,
                             const Matrix &c) const override;

  /// @brief Get shape function values at the integration points.
  /// @return Matrix of shape function values
  inline Matrix getVertexFunctions() const override;

  /// @brief Get shape function values at the integration points.
  /// @return Matrix of shape function values
  inline Matrix getShapeFunctions() const override;

  /// @brief Transform local coordinates to global coordinates.
  /// @param[out] x Global coordinate vector
  /// @param[in] u Local coordinate vector
  /// @param[in] c Coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
  void getGlobalPoint(const Vector &x,
                      const Vector &u,
                      const Matrix &c) const override;

  /// @brief get the gradients of the shape functions at the integration points
  /// @param[out] g gradient of the shape functions at the integration points g(i,j) i-th shape function at the j-th int point
  /// @param[out] w weights of the integration points
  /// @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
  void getShapeGradients(const Matrix &g,
                         const Vector &w,
                         const Matrix &c) const;

  /// @brief return the rotations at the integration points
  /// @param[out] Ri 3 x 3 Ri(.,.,i) rotational matrix at the i-th integration point
  /// @param[in] Rn nodal rotations, Rn(.,.,i) is the 3 x 3 rotation matrix at the i-th node
  void getRotations(const Cubix &Ri,
                    const Cubix &Rn) const;

  /// @brief Get the Xi at the integration points
  /// @param[out] Xi 6 x 6 Xi(.,.,j,i), where j are the nodes and i are the integration points
  /// @param[out] w weights of the integration points
  /// @param[in] u displacements of the nodes, u(i,j) is the i-th displacement of the j-th node
  /// @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
  void getXi(const Quadix &Xi,
             const Vector &w,
             const Matrix &u,
             const Matrix &c) const;

  /// @brief Xi from precomputed shape gradients and tangent (allocation free)
  /// @param[out] Xi  Xi matrices, 6 x 6 x nodeCount x ipointCount
  /// @param[in] grads  Shape gradients, shapeFuncCount x ipointCount
  /// @param[in] phiP  Tangent of the deformed line at the integration points, 3 x ipointCount
  void getXi(const Quadix &Xi,
             const Matrix &grads,
             const Matrix &phiP) const;

  /// @brief Get the Psi at the integration points
  /// @param[out] Psi 6 x 9 Psi(.,.,j,i), where j are the nodes and i are the integration points
  /// @param[out] w weights of the integration points
  /// @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
  void getPsi(const Quadix &Psi,
              const Vector &w,
              const Matrix &c) const;

  /// @brief Get the Pi at the integration points
  /// @param[out] Pi 6 x 6 Pi(.,.,i) where i are the integration points
  /// @param[out] Ri 3 x 3 Ri(.,.,i) rotation at the i-th integration point
  /// @param[in] Rn nodal rotations, Rn(.,.,i) is the 3 x 3 rotation matrix at the i-th node
  void getPi(const Cubix &Pi,
             const Cubix &Ri,
             const Cubix &Rn) const;

  /// @brief Get the Pi at the integration points
  /// @param[out] Pi 6 x 6 Pi(.,.,i) where i are the integration points
  /// @param[in] Rn nodal rotations, Rn(.,.,i) is the 3 x 3 rotation matrix at the i-th node
  void getPi(const Cubix &Pi,
             const Cubix &Rn) const;

  /// @brief Get the rotation gradients at the integration points from the rotations (Crisfield/Jelenic)
  /// @param[out] LambdaP 3 x 3 curvature at the integration points, LambdaP(.,.,i) is the gradient at the i-th point
  /// @param[out] w integration weights
  /// @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
  /// @param[in] nodeLambda nodal rotations, nodeLambda(.,.,i) is the 3 x 3 rotation matrix at the i-th node
  void getRotationGradients(const Cubix &LambdaP,
                            const Vector &w,
                            const Matrix &c,
                            const Cubix &nodeLambda) const;

  /// @brief Get the Pi, the rotations and the rotation gradients at the integration points using one evaluation of the node rotation vectors
  /// @param[out] Pi 6 x 6 Pi(.,.,i) where i are the integration points
  /// @param[out] Ri 3 x 3 Ri(.,.,i) rotation at the i-th integration point
  /// @param[out] LambdaP 3 x 3 curvature at the integration points, LambdaP(.,.,i) is the gradient at the i-th point
  /// @param[out] w integration weights
  /// @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
  /// @param[in] nodeLambda nodal rotations, nodeLambda(.,.,i) is the 3 x 3 rotation matrix at the i-th node
  void getPiAndRotationGradients(const Cubix &Pi,
                                 const Cubix &Ri,
                                 const Cubix &LambdaP,
                                 const Vector &w,
                                 const Matrix &c,
                                 const Cubix &nodeLambda) const;

  /// @brief As above, with precomputed shape gradients (allocation free)
  /// @param[in] grads  Shape gradients, shapeFuncCount x ipointCount
  void getPiAndRotationGradients(const Cubix &Pi,
                                 const Cubix &Ri,
                                 const Cubix &LambdaP,
                                 const Matrix &grads,
                                 const Cubix &nodeLambda) const;

  //------------------------------------------------
  // local location functions
  //------------------------------------------------
  /// @brief Evaluate shape functions at given local coordinates.
  /// @param[out] h Shape function value vector
  /// @param[in] u Local coordinate vector
  inline void evalShapeFunctions(const Vector &h,
                                 const Vector &u) const override;

  /// @brief Evaluate shape functions and their gradients at local coordinates.
  /// @param[out] h Shape function value vector
  /// @param[out] g Shape function gradient vector
  /// @param[in] u Local coordinate vector
  inline void evalShapeGradients(const Vector &h,
                                 const Vector &g,
                                 const Vector &u) const;

  /// @brief Evaluate shape functions, gradients, and second derivatives.
  /// @param[out] h Shape function value vector
  /// @param[out] g Shape function gradient vector
  /// @param[out] gg Shape function second derivative vector
  /// @param[in] u Local coordinate vector
  inline void evalShapeGradGrads(const Vector &h,
                                 const Vector &g,
                                 const Vector &gg,
                                 const Vector &u) const;

 private:
  /// @brief Get the rotations at the integration points from the node rotation vectors
  /// @param[out] Ri Rotation matrices at the integration points
  /// @param[in] node_psi Nodal rotation vectors
  /// @param[in] Lambda_r Reference rotation matrix
  void getRotations_(const Cubix &Ri,
                     const Matrix &node_psi,
                     const Matrix &Lambda_r) const;

  /// @brief Get the rotation gradients at the integration points from the node rotation vectors
  /// @param[out] LambdaP Rotation gradients at the integration points
  /// @param[out] w Integration weights
  /// @param[in] c Coordinates of the nodes
  /// @param[in] node_psi Nodal rotation vectors
  /// @param[in] Lambda_r Reference rotation matrix
  void getRotationGradients_(const Cubix &LambdaP,
                             const Matrix &grads,
                             const Matrix &node_psi,
                             const Matrix &Lambda_r) const;

  /// @brief Get the reference rotation according to Crisfield/Jelenic
  /// @param[out] Lambda_r Reference rotation matrix
  /// @param[in] Rn Nodal rotation matrices
  void getRefRot_(const Matrix Lambda_r,
                  const Cubix Rn) const;

  /// @brief Get the local node rotation vectors according to Crisfield/Jelenic
  /// @param[out] psi Nodal rotation vectors
  /// @param[out] Lambda_r Reference rotation matrix
  /// @param[in] Rn Nodal rotation matrices
  void getNodeRotVecs_(const Matrix psi,
                       const Matrix Lambda_r,
                       const Cubix Rn) const;

 protected:
  jem::Ref<ParametricLine> intLine_; ///< internal line element for some standard functions

  // Work arrays and matrix product of the functions that run for every element
  // (see the note in utils/helpers.h): created once in the constructor and
  // overwritten in every call. The object is not thread safe.
  // The result of matmul_ is only valid until its next call.
  mutable Matrix nodePsi_; ///< relative rotation vectors of the nodes, 3 x nodeCount
  mutable Matrix lambdaR_; ///< reference rotation, 3 x 3
  mutable Matrix ipPsi_;   ///< rotation vectors at the integration points, 3 x ipointCount
  mutable MatmulChain<double, 2> matmul_; ///< matrix products without allocations
  mutable Matrix relRot_;  ///< relative rotation, 3 x 3
  mutable Matrix skewP_;   ///< skew symmetric matrix of the tangent, 3 x 3
  mutable Vector phiIJ_;   ///< relative rotation vector between the reference nodes

  /// @name Memo of the last getShapeGradients() call
  /// @details The gradients only depend on the node coordinates, and the same
  /// coordinates are requested several times in a row (strains, Xi, rotation
  /// gradients of one element). A hit needs bit-identical coordinates, so the
  /// results are exactly those of a fresh evaluation. Not thread safe.
  /// @{
  mutable bool gradMemoValid_ = false;
  mutable Matrix gradMemoCoords_;
  mutable Matrix gradMemoGrads_;
  mutable Vector gradMemoWeights_;
  /// @}
};

//=====================================================================
//    inline implementations
//=====================================================================
/// @brief Get the global rank (spatial dimension).
/// @return Global rank
idx_t Line3D::globalRank() const
{
  return GLOB_RANK;
}

/// @brief Get the local rank of the line element.
/// @return Local coordinate dimension
idx_t Line3D::localRank() const
{
  return intLine_->localRank();
}

/// @brief Get the number of nodes in the line element.
/// @return Number of nodes
idx_t Line3D::nodeCount() const
{
  return intLine_->nodeCount();
}

/// @brief Get the number of vertices in the line element.
/// @return Number of vertices
idx_t Line3D::vertexCount() const
{
  return intLine_->vertexCount();
}

/// @brief Get the number of integration points.
/// @return Number of integration points
idx_t Line3D::ipointCount() const
{
  return intLine_->ipointCount();
}

/// @brief Get the number of shape functions.
/// @return Number of shape functions
idx_t Line3D::shapeFuncCount() const
{
  return intLine_->shapeFuncCount();
}

/// @brief Get the local coordinates of the vertices.
/// @return Matrix of vertex coordinates
Matrix Line3D::getVertexCoords() const
{
  return intLine_->getVertexCoords();
}

/// @brief Get the integration scheme.
/// @return Matrix describing the integration scheme
Matrix Line3D::getIntegrationScheme() const
{
  return intLine_->getIntegrationScheme();
}

/// @brief Check whether a local point is inside the element.
/// @param[in] u Local coordinate vector
/// @return Whether the point is contained in the element
bool Line3D::containsLocalPoint(const Vector &u) const
{
  return intLine_->containsLocalPoint(u);
}

/// @brief Get the vertex shape function values at the integration points.
/// @return Matrix of shape function values
Matrix Line3D::getVertexFunctions() const
{
  return intLine_->getVertexFunctions();
}

/// @brief Evaluate shape functions at given local coordinates.
/// @param[out] h Shape function value vector
/// @param[in] u Local coordinate vector
void Line3D::evalShapeFunctions(const Vector &h,
                                const Vector &u) const
{
  JEM_ASSERT2(u.size() == localRank(), "internal coordinates do not match the rank");
  intLine_->evalShapeFunctions(h, u);
}

/// @brief Evaluate shape functions and their gradients at local coordinates.
/// @param[out] h Shape function value vector
/// @param[out] g Shape function gradient vector
/// @param[in] u Local coordinate vector
void Line3D::evalShapeGradients(const Vector &h,
                                const Vector &g,
                                const Vector &u) const
{
  JEM_ASSERT2(u.size() == localRank(), "internal coordinates do not match the rank");
  Matrix g_dummy(1, g.size());
  intLine_->getSShape()->evalShapeGradients(h, g_dummy, u);
  g = g_dummy(0, ALL);
}

/// @brief Evaluate shape functions, gradients, and second derivatives.
/// @param[out] h Shape function value vector
/// @param[out] g Shape function gradient vector
/// @param[out] gg Shape function second derivative vector
/// @param[in] u Local coordinate vector
void Line3D::evalShapeGradGrads(const Vector &h,
                                const Vector &g,
                                const Vector &gg,
                                const Vector &u) const
{
  JEM_ASSERT2(u.size() == localRank(), "internal coordinates do not match the rank");
  Matrix g_dummy(1, g.size());
  Matrix gg_dummy(1, gg.size());
  intLine_->getSShape()->evalShapeGradGrads(h, g_dummy, gg_dummy, u);
  g = g_dummy(0, ALL);
  gg = gg_dummy(0, ALL);
}

/// @brief Get shape function values at the integration points.
/// @return Matrix of shape function values
Matrix Line3D::getShapeFunctions() const
{
  return intLine_->getShapeFunctions();
}
