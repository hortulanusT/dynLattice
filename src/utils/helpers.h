/**
 * @file helpers.h
 * @author Til Gärtner
 * @brief file containing some basic helper functions
 *
 */
#pragma once

#include "utils/testing.h"

#include <jem/base/Array.h>
#include <jem/base/CString.h>
#include <jem/base/tuple/Tuple.h>
#include <jem/numeric/Quaternion.h>
#include <jem/numeric/algebra.h>
#include <jem/numeric/func/Function.h>

#include <jive/Array.h>
#include <jive/util/Constraints.h>
#include <jive/util/FuncUtils.h>

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

/// @brief namespace for all helper functions
namespace jive_helpers
{
typedef Array<double, 4> Quadix;
typedef Array<IdxVector, 2> IdxVectorMatrix;

const Vector e1 = {1., 0., 0.};
const Vector e2 = {0., 1., 0.};
const Vector e3 = {0., 0., 1.};
const Vector vec0 = {0., 0., 0.};
const double TINY = __DBL_EPSILON__ * 1e6;

/// @brief gets the gradient of a multivariate function.
///
/// @param[in] func  function of which to evaluate the gradient
/// @param[in] args  point to evaluate the gradient
/// @returns gradient
Vector funcGrad(const Ref<Function> func, const Vector &args);

/// @brief evaluates an array of multivariate functions.
///
/// @param[in] funcs  array of functions to evaluate
/// @param[in] args  point to evaluate at
/// @returns results of the evaluation
Vector evalFuncs(const FuncUtils::FuncArray funcs, const Vector &args);

/// @brief gets the Hessian of a multivariate function.
///
/// @param[in] func  function of which to evaluate the Hessian
/// @param[in] args  point to evaluate the Hessian
/// @returns Hessian
Matrix funcHessian(const Ref<Function> func, const Vector &args);

/// @brief gets the gradients of an array of multivariate functions.
///
/// @param[in] funcs  array of functions of which to evaluate the gradient
/// @param[in] args  point to evaluate the gradient
/// @returns Matrix of the gradients
Matrix gradFuncs(const FuncUtils::FuncArray funcs, const Vector &args);

/// @brief generates an identity matrix of the given dimension.
///
/// @param[in] dim  dimension of the identity matrix (default 3)
/// @returns identity matrix
Matrix eye(const idx_t dim = 3);

/// @brief compute rotational vector from rotation matrix.
///
/// This function computes the logarithmic map of a rotation matrix
/// as a member of the Lie group SO(3) to the Lie algebra \f$\mathfrak{so}(3)\f$.
/// It returns the axial vector of the rotation matrix.
/// @see [Wikipedia](https://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation#Log_map_from_SO(3)_to_%F0%9D%94%B0%F0%9D%94%AC(3))
///
/// The angle lies in [0, pi]; close to pi (cos(theta) < -0.9) the angle comes from atan2 and the axis from the
/// symmetric part of R, which keeps full precision up to theta = pi.
///
/// @param[out] rv  3-component rotational vector
/// @param[in] R  3 x 3 rotational matrix
void logMat(const Vector &rv, const Matrix &R);

/// @brief rearrange a vector into a matrix.
///
/// @param[out] mat  Matrix populated from the vector values by row
/// @param[in] vec  Vector whose size matches the number of matrix entries
void vec2mat(const Matrix &mat, const Vector &vec);

/// @brief rearrange a matrix into a vector.
///
/// @param[out] vec  Vector populated with the matrix values by row
/// @param[in] mat  Matrix whose entry count matches the vector size
void mat2vec(const Vector &vec, const Matrix &mat);

// -----------------------------------------------------------------------
// Temporary arrays: why and how we avoid them
//
// A JEM array (Vector, Matrix, Cubix) is a small handle that points to a
// block of numbers. Copying a handle ("Vector a = b;") is cheap and does NOT
// copy the numbers: a and b then show the same numbers. Creating a new array,
// or writing "x = matmul(A, v)", asks the memory manager for a fresh block of
// numbers and gives it back afterwards. For the 3 to 6 numbers of a rod
// element this bookkeeping costs more than the arithmetic itself, and in the
// force assembly it happens millions of times per run. Therefore the code that
// is executed for every element and every time step ("hot code") follows
// three rules; everywhere else (set-up, output, rarely used code) the short
// forms matmul(A, v) and skew(v) stay, because they are easier to read.
//
//  1. Work arrays: arrays that live as (mutable) members of the class, are
//     created once in the constructor/configure and overwritten in every
//     call. A work array is always written completely before it is read
//     within the same call, and it is never used to carry values from one
//     call to the next. A class with work arrays must not be used by several
//     threads at once.
//  2. 3 x 3 products: mul3(C, A, B) and mul3(y, A, x). The result may be one
//     of the factors.
//  3. All other matrix products: a MatmulChain member (jem::numeric), which
//     keeps its result array and reuses it. The result is only valid until
//     the next call of the same chain, so use it directly or copy it first.
// -----------------------------------------------------------------------

/// @brief C = A * B for 3 x 3 matrices without heap allocation.
///
/// The product is formed on the stack first, so C may alias A or B (unlike
/// jem::numeric::matmul(C, A, B)); the 3 x 3 sizes are fixed, which makes it
/// faster than the general matmul. Use it for every 3 x 3 product in hot code.
/// @param[out] C  3 x 3 result
/// @param[in] A  3 x 3 left factor
/// @param[in] B  3 x 3 right factor
void mul3(const Matrix &C, const Matrix &A, const Matrix &B);

/// @brief y = A * x for a 3 x 3 matrix without heap allocation.
///
/// x is copied first, so y may alias x. The sums are formed in the same order
/// as in jem::numeric::matmul (starting from zero), so the results are the same.
/// @param[out] y  3 vector result
/// @param[in] A  3 x 3 matrix (may be a transposed view)
/// @param[in] x  3 vector
void mul3(const Vector &y, const Matrix &A, const Vector &x);

/// @brief compute the exponential of a axial vector.
///
/// It computes the matrix exponential of the skew-symmetric matrix of this
/// vector so when \f$\Theta=[\theta\times]\f$ this function will return
/// \f$\exp(\Theta)\f$.
/// @see [Wikipedia](https://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation#Exponential_map_from_%F0%9D%94%B0%F0%9D%94%AC(3)_to_SO(3))
///
/// @param[out] Exp  3 x 3 matrix exponential
/// @param[in] v  3-component axial rotation vector
void expVec(const Matrix &Exp, const Vector &v);

/// @brief derivative of jive_helpers::expVec.
///
/// @see jive_helpers::expVec
/// @see [Differentiation rules](https://en.wikipedia.org/wiki/Differentiation_rules)
///
/// @param[out] ExpP  Derivative of the 3 x 3 matrix exponential
/// @param[in] v  3-component axial rotation vector
/// @param[in] vP  Derivative of the axial rotation vector
void expVecP(const Matrix &ExpP, const Vector &v, const Vector &vP);

/// @brief tangent operator T(v) of the exponential map.
///
/// If the rotation vector v of \f$\Lambda = \exp(\hat v)\f$ changes by dv, the rotation changes by the spin
/// (infinitesimal rotation in the fixed frame) T(v) dv, i.e. \f$d\Lambda\,\Lambda^T = \widehat{T(v)\,dv}\f$, with
/// \f$T(v) = I + \frac{1-\cos\theta}{\theta^2}\hat v + \frac{\theta-\sin\theta}{\theta^3}\hat v^2\f$, θ = |v|
/// (also called the left Jacobian of SO(3)). This is \f$T^{-1}\f$ of Jelenic & Crisfield (1999, CMAME), Eq. 2.5
/// (their Eq. 2.4: \f$\Delta\Theta = T(\Theta)\,\Delta\vartheta\f$), which they write with the dyad
/// \f$\Theta\Theta^T\f$ instead of \f$\hat\Theta^2\f$:
/// \f$T^{-1}(\Theta) = \frac{\sin\theta}{\theta} I + \frac{1}{\theta^2}\left(1-\frac{\sin\theta}{\theta}\right)\Theta\Theta^T
/// + \frac{1-\cos\theta}{\theta^2}\hat\Theta\f$; both forms are equal because
/// \f$\hat\Theta^2 = \Theta\Theta^T - \theta^2 I\f$. Small angles use the power series (see tangentCoeffs in
/// helpers.cpp). No heap allocation.
/// @param[out] T  3 x 3 tangent operator
/// @param[in] v  rotation vector
void expTangent(const Matrix &T, const Vector &v);

/// @brief inverse of jive_helpers::expTangent.
///
/// \f$T^{-1}(v) = I - \frac12\hat v + \frac{1}{\theta^2}\left(1-\frac{\theta}{2}\cot\frac{\theta}{2}\right)\hat v^2\f$
/// (singular at θ = 2π). This is T of Jelenic & Crisfield (1999, CMAME), Eq. 2.5, written there as
/// \f$T(\Theta) = \frac{\theta/2}{\tan(\theta/2)} I + \frac{1}{\theta^2}\left(1-\frac{\theta/2}{\tan(\theta/2)}\right)
/// \Theta\Theta^T - \frac12\hat\Theta\f$ (equal by \f$\hat\Theta^2 = \Theta\Theta^T - \theta^2 I\f$). Small angles use
/// the power series. No heap allocation.
/// @param[out] Tinv  3 x 3 inverse tangent operator
/// @param[in] v  rotation vector
void expTangentInv(const Matrix &Tinv, const Vector &v);

/// @brief directional derivative of jive_helpers::expTangent: dT = d/dε T(v + ε w) at ε = 0.
/// @param[out] dT  3 x 3 derivative
/// @param[in] v  rotation vector
/// @param[in] w  direction
void expTangentD(const Matrix &dT, const Vector &v, const Vector &w);

/// @brief calculate the trace of a matrix.
///
/// @param[in] mat  Matrix to get the trace of
/// @returns the trace of the given matrix
double trace(const Matrix &mat);

/// @brief calculates the factorial of an integer.
///
/// @param[in] n  Integer to calculate the factorial from
/// @returns the calculated factorial
idx_t factorial(const idx_t n);

/// @brief calculate the binomial coefficient.
///
/// @param[in] n  Parameter n
/// @param[in] k  Parameter k
/// @returns binomial coefficient nCk
idx_t binom(const idx_t n, const idx_t k);

/// @brief calculates the 2-norm of a matrix.
///
/// @param[in] mat  Matrix
/// @returns resulting 2-norm
double matrixNorm2(const Matrix &mat);

/// @brief construct a skew symetric matrix from a given vector.
///
/// Convenient form for set-up code; it creates a new matrix. In hot code use
/// the form below with a work matrix.
/// @param[in] vec  Vector to be changed to a skew-symmetric matrix
/// @returns skew symmetric matrix
Matrix skew(const Vector &vec);

/// @brief fill an existing 3 x 3 matrix with the skew symmetric matrix of a vector
///
/// Does not create a new matrix, so it is the form for hot code.
/// @param[out] S  3 x 3 skew symmetric matrix
/// @param[in] vec  Vector
void skew(const Matrix &S, const Vector &vec);

/// @brief construct a axial vector from a given skew-symmetric
/// matrix
///
/// @param[in] mat  Skew-symmetric matrix
/// @returns axial vector
Vector unskew(const Matrix &mat);

/// @brief prescribe the value of a dof, i.e. constrain idof = rval.
///
/// @param[in,out] cons  constraints to modify
/// @param[in] idof  dof index to prescribe
/// @param[in] rval  prescribed value
void setDofValue(jive::util::Constraints &cons, const idx_t idof, const double rval);

/// @brief link a slave dof to a master dof, i.e. constrain idof = coeff * jdof.
///
/// @param[in,out] cons  constraints to modify
/// @param[in] idof  slave dof index
/// @param[in] jdof  master dof index
/// @param[in] coeff  coefficient of the master dof
void linkDofs(jive::util::Constraints &cons, const idx_t idof, const idx_t jdof, const double coeff);
}; // namespace jive_helpers
