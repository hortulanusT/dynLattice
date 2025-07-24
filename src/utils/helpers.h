/**
 * @file helpers.h
 * @author Til Gärtner
 * @brief file containing some basic helper functions
 *
 */
#pragma once

#include <jem/base/Array.h>
#include <jem/base/CString.h>
#include <jem/numeric/Quaternion.h>
#include <jem/numeric/algebra.h>
#include <jem/numeric/func/Function.h>
#include <jive/Array.h>
#include <jive/util/FuncUtils.h>

#include "utils/testing.h"

#define WARN(key) jem::System::warn() << key << "\n";
#define ERR(key) jem::System::err() << key << "\n";

using jem::ALL;
using jem::Array;
using jem::idx_t;
using jem::SliceFromTo;
using jem::String;
using jem::numeric::dotProduct;
using jem::numeric::Function;
using jem::numeric::matmul;
using jem::numeric::MatmulChain;
using jem::numeric::norm2;
using jem::numeric::Quaternion;

using jive::Cubix;
using jive::IdxVector;
using jive::Matrix;
using jive::Ref;
using jive::Vector;
using jive::util::FuncUtils;

/**
 * @brief namespace for all helper functions
 *
 */
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
   * @brief gets the gradient of a multivariate function.
   *
   * @param func function of which to evaluat the gradient
   * @param args point to evaluat the gradient
   * @returns gradient
   */
  Vector funcGrad(const Ref<Function> func, const Vector &args);

  /**
   * @brief evaluates an array of multivariate functions.
   *
   * @param funcs array of functions to evaluate
   * @param args point to evaluat at
   * @returns results of the evaluation
   */
  Vector evalFuncs(const FuncUtils::FuncArray funcs, const Vector &args);

  /**
   * @brief gets the Hessian of a multivariate function.
   *
   * @param func function of which to evaluat the Hessian
   * @param args point to evaluat the Hessian
   * @returns Hessian
   */
  Matrix funcHessian(const Ref<Function> func, const Vector &args);

  /**
   * @brief gets the gradients of an array of multivariate functions.
   *
   * @param funcs array of functions of which to evaluat the gradient
   * @param args point to evaluat the gradient
   * @returns Matrix of the gradients
   */
  Matrix gradFuncs(const FuncUtils::FuncArray funcs, const Vector &args);

  /**
   * @brief generates an identity matrix of the given dimension.
   *
   * @param dim dimension of the identity matrix (default 3)
   * @returns identity matrix
   */
  Matrix eye(const idx_t dim = 3);

  /**
   * @brief compute rotational vector from rotation matrix.
   *
   * This function computes the logarithmic map of a rotation matrix
   * as a member of the Lie group SO(3) to the Lie algebra \f$\mathfrak{so}(3)\f$.
   * It returns the axial vector of the rotation matrix.
   * @see [Wikipedia](https://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation#Log_map_from_SO(3)_to_%F0%9D%94%B0%F0%9D%94%AC(3))
   *
   * @param[out] rv rotational vector
   * @param[in] R rotational matrix
   */
  void logMat(const Vector &rv, const Matrix &R);

  /**
   * @brief rearrange a vector into a matrix.
   *
   * @param[out] mat matrix of given dimensions
   * @param[in] vec vector (needs to have fitting dimension)
   */
  void vec2mat(const Matrix &mat, const Vector &vec);

  /**
   * @brief rearrange a matrix into a vector.
   *
   * @param[out] mat matrix of given dimensions
   * @param[in] vec vector (needs to have fitting dimension)
   */
  void mat2vec(const Vector &vec, const Matrix &mat);

  /**
   * @brief compute the exponential of a axial vector.
   *
   * It computes the matrix exponential of the skew-symmetric matrix of this
   * vector so when \f$\Theta=[\theta\times]\f$ this function will return
   * \f$\exp(\Theta)\f$.
   * @see [Wikipedia](https://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation#Exponential_map_from_%F0%9D%94%B0%F0%9D%94%AC(3)_to_SO(3))
   *
   * @param[out] Exp resulting matrix exponential
   * @param[in] v axial rotation vector
   */
  void expVec(const Matrix &Exp, const Vector &v);

  /**
   * @brief derivative of jive_helpers::expVec.
   *
   * @see jive_helpers::expVec
   * @see [Differentiation rules](https://en.wikipedia.org/wiki/Differentiation_rules)
   *
   * @param[out] ExpP resulting matrix exponential
   * @param[in] v axial rotation vector
   * @param[in] vP derivative of the axial rotation vector
   */
  void expVecP(const Matrix &ExpP, const Vector &v, const Vector &vP);

  /**
   * @brief calculate the trace of a matrix.
   *
   * @param mat matrix to get the trace of
   * @returns the trace of the given matrix
   */
  double trace(const Matrix &mat);

  /**
   * @brief calculates the factorial of an integer.
   *
   * @param n integer to calculate the factorial from
   * @returns the calculated factorial
   */
  idx_t factorial(const idx_t n);

  /**
   * @brief calculate the binomial coefficient.
   *
   * @param n parameter n
   * @param k parameter k
   * @returns binomial coefficient nCk
   */
  idx_t binom(const idx_t n, const idx_t k);

  /**
   * @brief calculates the 2-norm of a matrix.
   *
   * @param mat matrix
   * @returns resulting 2-norm
   */
  double matrixNorm2(const Matrix &mat);

  /**
   * @brief construct a skew symetric matrix from a given vector.
   *
   * @param vec vector to be changed to skew symmetric matrix
   * @returns skew symmetric matrix
   */
  Matrix skew(const Vector &vec);

  /**
   * @brief construct a axial vector from a given skew-symmetric
   * matrix
   *
   * @param mat skew symmetric matrix
   * @returns axial vector
   */
  Vector unskew(const Matrix &mat);
}; // namespace jive_helpers
