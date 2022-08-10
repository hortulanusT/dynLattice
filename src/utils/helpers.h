/**
 * @file helpers.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief file containing some basic helper functions
 * @version 0.1
 * @date 2021-10-25
 *
 * @copyright (C) 2021 TU Delft. All rights reserved.
 *
 */
#pragma once

#include <jem/base/Array.h>
#include <jem/base/CString.h>
#include <jem/numeric/algebra.h>
#include <jem/numeric/Quaternion.h>
#include <jive/Array.h>

#include "utils/testing.h"

#define WARN(key) jem::System::warn(getContext()) << key << "\n";

using jem::ALL;
using jem::Array;
using jem::idx_t;
using jem::SliceFromTo;
using jem::numeric::dotProduct;
using jem::numeric::matmul;
using jem::numeric::MatmulChain;
using jem::numeric::norm2;
using jem::numeric::Quaternion;

using jive::Cubix;
using jive::IdxVector;
using jive::Matrix;
using jive::Vector;

namespace jive_helpers
{
  typedef Array<double, 4> Quadix;
  typedef Array<IdxVector, 2> IdxVectorMatrix;

  const Vector e1 = {1., 0., 0.};
  const Vector e2 = {0., 1., 0.};
  const Vector e3 = {0., 0., 1.};
  const Vector vec0 = {0., 0., 0.};
  const double TINY = __DBL_EPSILON__ * 1e6;

  /**
   * @brief generates an identity matrix of the given dimension
   *
   * @param dim dimension of the identity matrix (default 3)
   * @return identity matrix
   */
  Matrix eye(const idx_t dim = 3);

  /**
   * @brief compute rotational vector from rotation matrix
   *
   * https://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation#Exponential_map_from_%F0%9D%94%B0%F0%9D%94%AC(3)_to_SO(3)
   * @param[out] rv rotational vector
   * @param[in] R rotational matrix
   */
  void logMat(const Vector &rv,
              const Matrix &R);

  /**
   * @brief rearrange a vector into a matrix
   *
   * @param[out] mat matrix of given dimensions
   * @param[in] vec vector (needs to have fitting dimension)
   */
  void vec2mat(const Matrix &mat,
               const Vector &vec);

  /**
   * It computes the matrix exponential of the skew-symmetric matrix of this vector
   * so when \f$\Theta=[\theta\times]\f$ this function will return \f$\exp(\Theta)\f$.
   * https://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation#Exponential_map_from_%F0%9D%94%B0%F0%9D%94%AC(3)_to_SO(3)
   *
   * @brief compute the exponential of a axial vector
   *
   * @param[out] Exp resulting matrix exponential
   * @param[in] v axial rotation vector
   */
  void expVec(const Matrix &Exp,
              const Vector &v);

  /**
   * @brief derivative of jive_helpers::expVec
   *
   * @param[out] ExpP resulting matrix exponential
   * @param[in] v axial rotation vector
   * @param[in] vP derivative of the axial rotation vector
   */
  void expVecP(const Matrix &ExpP,
               const Vector &v,
               const Vector &vP);

  // /**
  //  * It computes inverse tangent operator for an exponential mapping
  //  * (see DOI: 10.1007/bf00370057 formula A.21) //LATER direkt update as in Holzinger/Gerstenmeyer
  //  *
  //  * @brief compute the inverste tangent operator
  //  *
  //  * @param[out] T_1 inverse tangent operator
  //  * @param[in] v axial rotation vector
  //  */
  // void getTangentOp
  // ( const Matrix& T_1,
  //   const Vector& v );

  /**
   * @brief turn a rotation matrix into a quaterion
   *
   * @param[out] q quaterion
   * @param[in] Q rotation matrix
   */
  void rotMat2Quat(const Vector &q,
                   const Matrix &Q);

  /**
   * @brief calculate the trace of a matrix
   *
   * @param mat matrix to get the trace of
   * @return the trace of the given matrix
   */
  double trace(const Matrix &mat);

  /**
   * @brief calculates the factorial of an integer
   *
   * @param n integer to calculate the factorial from
   * @return the calculated factorial
   */
  idx_t factorial(const idx_t n);

  /**
   * @brief calculate the binomial coefficient
   *
   * @param n parameter n
   * @param k parameter k
   * @return binomial coefficient nCk
   */
  idx_t binom(const idx_t n,
              const idx_t k);

  /**
   * @brief calculates the 2-norm of a matrix
   *
   * @param mat matrix
   * @return resulting 2-norm
   */
  double matrixNorm2(const Matrix &mat);

  /**
   * @brief funciton to construct a skew symetric matrix from a given vector
   *
   * @param vec vector to be changed to skew symmetric matrix
   * @returns skew symmetric matrix
   */
  Matrix skew(const Vector &vec);

  /**
   * @brief funciton to construct a axial vector from a given skew-symmetric matrix
   *
   * @param mat skew symmetric matrix
   * @returns axial vector
   */
  Vector unskew(const Matrix &mat);
};