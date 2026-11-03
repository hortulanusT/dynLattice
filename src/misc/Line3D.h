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
using jem::numeric::MatmulChain;
using jem::util::Properties;
using jive::Cubix;
using jive::Matrix;
using jive::Vector;
using jive::geom::ParametricLine;
using jive::geom::Shape;
using jive_helpers::Quadix;

/// @brief Shape function for 1D line elements in 3D space, with the rotation interpolation of the rod elements.
///
/// Provides shape functions and geometric operations for line elements embedded
/// in 3D space. Supports 2, 3, and 4-node line elements with configurable
/// integration schemes (default: reduced Gauss integration, nodeCount - 1 points).
///
/// @section RotationInterpolation Rotation interpolation
/// Interpolation only; the rod theory is in SpecialCosseratRodModel. References: SVQ = Simo & Vu-Quoc (1986), CMAME
/// 58; CJ = Crisfield & Jelenic (1999), Proc. R. Soc. A 455; JC = Jelenic & Crisfield (1999), CMAME 171.
/// Notation: shape functions N_K (I^i in JC); from JC: Lambda_r, phi, psi_K (Psi_K^l), It_K (I-tilde^i).
///  - Rotations (CJ Eqs. 5.6-5.8, 6.2; JC Eqs. 3.8-3.12): Lambda_r = Lambda_I exp(phi/2), phi = log(Lambda_I^T Lambda_J),
///    I = (n-1)/2, J = n/2 (0-based, I = J for odd n); psi_K = log(Lambda_r^T Lambda_K), psi = sum_K N_K psi_K,
///    Lambda = Lambda_r exp(psi); see getRotationsAndGradients().
///  - Spins (JC Eqs. 3.17-3.19): delta theta = sum_K It_K delta theta_K, the trial functions of the tangent; the test
///    functions are N_K (JC Eq. 3.1, SVQ Eq. 5.1); see getGeneralisedShapeFunctions().
/// T(v) = jive_helpers::expTangent (spin of exp(v) per dv), the inverse of T in JC Eqs. 2.4-2.5.
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

  /// @brief Rotations Lambda and their derivatives Lambda' at the integration points (allocation free)
  /// @param[out] Ri rotations Lambda, 3 x 3 x ipointCount
  /// @param[out] LambdaP their derivatives Lambda' along the line, 3 x 3 x ipointCount
  /// @param[in] grads shape gradients, shapeFuncCount x ipointCount (see getShapeGradients)
  /// @param[in] nodeLambda nodal rotations, nodeLambda(.,.,i) is the 3 x 3 rotation matrix at the i-th node
  void getRotationsAndGradients(const Cubix &Ri,
                                const Cubix &LambdaP,
                                const Matrix &grads,
                                const Cubix &nodeLambda) const;

  /// @brief Generalised shape functions It_K and It'_K at the integration points (JC Eqs. 3.13-3.19)
  /// @details The spin of Lambda(s) for nodal spins delta theta_K is
  ///   delta theta(s) = delta theta_r + Lambda_r T(psi) delta psi(s)                        (JC Eq. 3.13)
  ///   delta psi_K = T^-1(psi_K) Lambda_r^T (delta theta_K - delta theta_r)                  (JC Eq. 3.14)
  ///   delta theta_r = Lambda_r (r_I Lambda_r^T delta theta_I + r_J Lambda_r^T delta theta_J) (JC Eq. 3.15)
  /// with r_J = 1/2 T(phi/2) T^-1(phi) = (I + exp[phi/2 x])^-1 and r_I = I - r_J (v_J and v_I of JC; r_I = I for
  /// I = J). Written in the frame of Lambda_r: It_K = Lambda_r X_K Lambda_r^T, X_K = a_K + a_r r_K with
  ///   a_K = N_K T(psi) T^-1(psi_K),  a_r = I - sum_K a_K,
  /// and It'_K the same with a'_K = (N_K dT(psi; psi') + N'_K T(psi)) T^-1(psi_K), a'_r = -sum_K a'_K (JC Eq. 3.19).
  /// If all nodes have the same rotation, It_K = N_K I.
  /// @param[out] genShapes generalised shape functions, genShapes(.,.,K,i) for node K at the i-th integration point
  /// @param[out] genShapesP their derivatives along the line, same shape
  /// @param[in] grads shape gradients, shapeFuncCount x ipointCount
  /// @param[in] nodeLambda nodal rotations, 3 x 3 x nodeCount
  void getGeneralisedShapeFunctions(const Quadix &genShapes,
                                    const Quadix &genShapesP,
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
  /// @brief Reference rotation refRotation_, relRotIJ_, halfRelRotIJ_ and the local nodal rotation vectors nodeLocalRotVecs_
  /// @param[in] Rn nodal rotations, 3 x 3 x nodeCount
  void getLocalNodeRotVecs_(const Cubix &Rn) const;

  /// @brief Rotation vectors ipLocalRotVecs_ = psi(s) and their derivatives ipLocalRotVecsP_ = psi'(s) at the integration points
  /// @param[in] grads shape gradients, shapeFuncCount x ipointCount
  void getLocalIpRotVecs_(const Matrix &grads) const;

 protected:
  jem::Ref<ParametricLine> intLine_; ///< internal line element for some standard functions
  idx_t refNodeI_;                   ///< first reference node I of Lambda_r (0-based)
  idx_t refNodeJ_;                   ///< second reference node J of Lambda_r (I = J for odd node counts)

  // Work arrays and matrix product of the functions that run for every element
  // (see the note in utils/helpers.h): created once in the constructor and
  // overwritten in every call. The object is not thread safe.
  // The result of matmul_ is only valid until its next call.
  mutable MatmulChain<double, 2> matmul_; ///< matrix products without allocations
  mutable Matrix refRotation_;            ///< reference rotation Lambda_r, 3 x 3
  mutable Vector relRotIJ_;               ///< relative rotation phi = log(Lambda_I^T Lambda_J)
  mutable Vector halfRelRotIJ_;           ///< phi / 2
  mutable Matrix nodeLocalRotVecs_;       ///< local nodal rotation vectors psi_K, 3 x nodeCount
  mutable Matrix ipLocalRotVecs_;         ///< psi at the integration points, 3 x ipointCount
  mutable Matrix ipLocalRotVecsP_;        ///< psi' at the integration points, 3 x ipointCount
  mutable Matrix rotWork_;                ///< 3 x 3 work matrix for relative rotations

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
