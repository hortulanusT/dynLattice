/**
 * @file Line3D.h
 * @author Til Gärtner
 * @brief Shape function for 1D line elements in 3D space.
 */

#pragma once

#include "utils/helpers.h"
#include <jem/util/Properties.h>
#include <jive/Array.h>
#include <jive/geom/Shape.h>
#include <jive/geom/ParametricLine.h>
#include <jive/geom/StdShape.h>

using jem::idx_t;
using jem::String;
using jem::util::Properties;
using jive::Cubix;
using jive::Matrix;
using jive::Vector;
using jive::geom::ParametricLine;
using jive::geom::Shape;
using jive_helpers::Quadix;

/**
 * @brief Shape function for 1D line elements in 3D space.
 *
 * Provides shape functions and geometric operations for line elements embedded
 * in 3D space. Supports 2, 3, and 4-node line elements with configurable
 * integration schemes and rotational control functions.
 *
 * @section Supported Elements
 * - 2-node linear line (StdLine2)
 * - 3-node quadratic line (StdLine3)
 * - 4-node cubic line (StdLine4)
 *
 * @author Til Gärtner
 * @see [Line Shapes](https://jive-manual.dynaflow.com/group__LineShapes.html)
 */
class Line3D : public Shape
{
public:
  static const String PROP_NODES; ///< Property name for number of nodes
  static const String PROP_INT;   ///< Property name for integration scheme
  static const String PROP_LOCAL; ///< Property name for local coordinates
  static const idx_t GLOB_RANK;   ///< Global rank (always 3 for 3D space)

  /**
   * @brief Constructs a new Line3D shape object.
   *
   * @param[in] name Shape instance name for identification
   * @param[out] conf Configuration properties container for output
   * @param[in] props Input properties containing shape parameters
   */
  explicit Line3D(const String &name,
                  const Properties &conf,
                  const Properties &props);

  /**
   * @brief Get the global rank (spatial dimension).
   * @return Global rank (always 3 for 3D space)
   */
  inline idx_t globalRank() const override;
  inline idx_t localRank() const override;
  inline idx_t nodeCount() const override;
  inline idx_t vertexCount() const override;
  inline idx_t ipointCount() const override;
  inline idx_t shapeFuncCount() const override;

  inline Matrix getVertexCoords() const override;
  inline Matrix getIntegrationScheme() const override;

  inline bool containsLocalPoint(const Vector &u) const override;

  /**
   * @brief Compute integration weights for numerical integration.
   *
   * @param[out] w Integration weights at quadrature points
   * @param[in] c Node coordinates matrix
   */
  void getIntegrationWeights(const Vector &w,
                             const Matrix &c) const override;

  inline Matrix getVertexFunctions() const override;

  inline Matrix getShapeFunctions() const override;

  /**
   * @brief Transform local coordinates to global coordinates.
   *
   * @param[out] x Global coordinates vector
   * @param[in] u Local coordinates vector
   * @param[in] c Node coordinates matrix
   */
  void getGlobalPoint(const Vector &x,
                      const Vector &u,
                      const Matrix &c) const override;

  /**
   * @brief get the gradients of the shape functions at the integration points
   * @param[out] g gradient of the shape functions at the integration points g(i,j) i-th shape function at the j-th int point
   * @param[out] w weights of the integration points
   * @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
   */
  void getShapeGradients(const Matrix &g,
                         const Vector &w,
                         const Matrix &c) const;

  /**
   * @brief return the rotations at the integration points
   * @param[out] Ri Ri(.,.,i) rotational matrix at the integration points
   * @param[in] Rn rotation at the nodes. Rn(.,i) rotational displacement at the nodes
   */
  void getRotations(const Cubix &Ri,
                    const Cubix &Rn) const;

  /**
   * @brief Get the Xi at the integration points
   *
   * @param[out] Xi Xi(.,.,j,i) where j are the nodes and i are the integration points
   * @param[out] w weights of the integration points
   * @param[in] u displacements of the nodes, c(i,j) is the i-th displacement of the j-th node
   * @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
   */
  void getXi(const Quadix &Xi,
             const Vector &w,
             const Matrix &u,
             const Matrix &c) const;

  /**
   * @brief Get the Psi at the integration points
   *
   * @param[out] Psi Psi(.,.,j,i) where j are the nodes and i are the integration points
   * @param[out] w weights of the integration points
   * @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
   */
  void getPsi(const Quadix &Psi,
              const Vector &w,
              const Matrix &c) const;
  /**
   * @brief Get the Pi at the integration points
   *
   * @param[out] Pi Pi(.,.,i) where i are the integration points
   * @param[out] Ri Ri(.,.,i) rotation at the integration points
   * @param[in] Rn rotations at the nodes.
   */
  void getPi(const Cubix &Pi,
             const Cubix &Ri,
             const Cubix &Rn) const;

  /**
   * @brief Get the Pi at the integration points
   *
   * @param[out] Pi Pi(.,.,i) where i are the integration points
   * @param[in] Rn rotations at the nodes.
   */
  void getPi(const Cubix &Pi,
             const Cubix &Rn) const;

  /**
   * @brief Get the rotation gradients at the integration points from the rotations (Crisfield/Jelenic)
   *
   * @param[out] LambdaP curvature at the integration points
   * @param[out] w integration weights
   * @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
   * @param[in] nodeLambda rotation at the nodes.
   */
  void getRotationGradients(const Cubix &LambdaP,
                            const Vector &w,
                            const Matrix &c,
                            const Cubix &nodeLambda) const;

  /**
   * @brief Get the rotation gradients at the integration points from the rotations (Crisfield/Jelenic)
   *
   * @param[out] LambdaP curvature at the integration points
   * @param[out] w integration weights
   * @param[in] Rn initial rotations at the integration points. Rn(.,.,i) initial rotation at the integration points
   * @param[in] c coordinates of the nodes, c(i,j) is the i-th coordinate of the j-th node
   * @param[in] theta rotation at the nodes. theta(.,i) rotational displacement at the nodes
   */
  void getRotationGradients(const Cubix &LambdaP,
                            const Vector &w,
                            const Cubix &Rn,
                            const Matrix &c,
                            const Matrix &theta) const;

  //------------------------------------------------
  // local location functions
  //------------------------------------------------
  /**
   * @brief Evaluate shape functions at given local coordinates.
   * @param[out] h Shape function values
   * @param[in] u Local coordinates
   */
  inline void evalShapeFunctions(const Vector &h,
                                 const Vector &u) const override;

  /**
   * @brief Evaluate shape functions and their gradients at local coordinates.
   * @param[out] h Shape function values
   * @param[out] g Shape function gradients
   * @param[in] u Local coordinates
   */
  inline void evalShapeGradients(const Vector &h,
                                 const Vector &g,
                                 const Vector &u) const;

  /**
   * @brief Evaluate shape functions, gradients, and second derivatives.
   * @param[out] h Shape function values
   * @param[out] g Shape function gradients
   * @param[out] gg Shape function second derivatives
   * @param[in] u Local coordinates
   */
  inline void evalShapeGradGrads(const Vector &h,
                                 const Vector &g,
                                 const Vector &gg,
                                 const Vector &u) const;

private:
  /**
   * @brief Get the reference rotation according to Crisfield/Jelenic
   *
   * @param[out] Lambda_r Reference rotation matrix
   * @param[in] Rn Rotation matrices at the nodes
   */
  void getRefRot_(const Matrix Lambda_r,
                  const Cubix Rn) const;

  /**
   * @brief Get the local node rotation vectors according to Crisfield/Jelenic
   *
   * @param[out] psi Node rotation vectors
   * @param[out] Lambda_r Reference rotation matrix
   * @param[in] Rn Rotation matrices at the nodes
   */
  void getNodeRotVecs_(const Matrix psi,
                       const Matrix Lambda_r,
                       const Cubix Rn) const;

protected:
  jem::Ref<ParametricLine> intLine_; ///< internal line element for some standard functions
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
  return intLine_->nodeCount();
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

bool Line3D::containsLocalPoint(const Vector &u) const
{
  return intLine_->containsLocalPoint(u);
}

Matrix Line3D::getVertexFunctions() const
{
  return intLine_->getVertexFunctions();
}

void Line3D::evalShapeFunctions(const Vector &h,
                                const Vector &u) const
{
  JEM_ASSERT2(u.size() == localRank(), "internal coordinates do not match the rank");
  intLine_->evalShapeFunctions(h, u);
}

void Line3D::evalShapeGradients(const Vector &h,
                                const Vector &g,
                                const Vector &u) const
{
  JEM_ASSERT2(u.size() == localRank(), "internal coordinates do not match the rank");
  Matrix g_dummy(1, g.size());
  intLine_->getSShape()->evalShapeGradients(h, g_dummy, u);
  g = g_dummy(0, ALL);
}

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

Matrix Line3D::getShapeFunctions() const
{
  return intLine_->getShapeFunctions();
}
