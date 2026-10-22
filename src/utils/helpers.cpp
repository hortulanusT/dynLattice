/**
 * @file helpers.cpp
 * @author Til Gärtner
 * @brief file containing some basic helper functions
 *
 */
#include "utils/helpers.h"

namespace
{
typedef jem::Tuple<double, 3> Vec3;
typedef jem::Tuple<double, 3, 3> Mat3;

// fill a skew-symmetric matrix from a 3-vector
inline void skew3(Mat3 &S, const Vec3 &v)
{
  S = 0.;
  S(0, 1) = -1 * v[2];
  S(0, 2) = 1 * v[1];
  S(1, 2) = -1 * v[0];
  S(1, 0) = 1 * v[2];
  S(2, 0) = -1 * v[1];
  S(2, 1) = 1 * v[0];
}
} // namespace

namespace jive_helpers
{
Vector funcGrad(const Ref<Function> func, const Vector &args)
{
  Vector res(func->argCount());
  res = 0.;

  for (idx_t i = 0; i < func->argCount(); i++)
  {
    res[i] = func->getDeriv(i, args.addr());
  }

  return res;
}

Vector evalFuncs(const FuncUtils::FuncArray funcs, const Vector &args)
{
  Vector res(funcs.size());
  res = 0.;

  for (idx_t i = 0; i < funcs.size(); i++)
  {
    res[i] = funcs[i]->getValue(args.addr());
  }

  return res;
}

Matrix funcHessian(const Ref<Function> func, const Vector &args)
{
  const idx_t argc = func->argCount();

  Matrix res(argc, argc);
  Vector g1(argc), g2(argc);
  Vector x1(argc), x2(argc);
  double dx;

  for (idx_t iarg = 0; iarg < argc; iarg++)
  {
    dx = 1.0e-6 * args[iarg];

    if (jem::isTiny(dx))
    {
      res[iarg] = 0.0;
      continue;
    }

    x1 = args;
    x2 = args;
    x1[iarg] -= dx;
    x2[iarg] += dx;
    dx = x2[iarg] - x1[iarg];

    g1 = funcGrad(func, x1);
    g2 = funcGrad(func, x2);

    res[iarg] = (g2 - g1) / dx;
  }

  return res;
}

Matrix gradFuncs(const FuncUtils::FuncArray funcs, const Vector &args)
{
  const idx_t argc = funcs.size();

  Matrix res(argc, argc);

  for (idx_t iarg = 0; iarg < argc; iarg++)
  {
    for (idx_t jarg = 0; jarg < argc; jarg++)
    {
      res(iarg, jarg) = funcs[iarg]->getDeriv(jarg, args.addr());
    }
  }

  return res;
}

Matrix eye(const idx_t dim)
{
  Matrix ret(dim, dim);
  ret = 0.0;

  for (idx_t i = 0; i < dim; i++)
    ret(i, i) = 1.0;

  return ret;
}

void logMat(const Vector &rv, const Matrix &R)
{
  double theta;                                 // angle of rotation
  double tr_R = trace(R) <= 3. ? trace(R) : 3.; // trace of the matrix

  theta = acos((tr_R - 1.) / 2.);

  // axial vector of R - R^T
  rv[0] = (R(2, 1) - R(1, 2)) - (R(1, 2) - R(2, 1));
  rv[1] = (R(0, 2) - R(2, 0)) - (R(2, 0) - R(0, 2));
  rv[2] = (R(1, 0) - R(0, 1)) - (R(0, 1) - R(1, 0));
  rv /= 2.;

  if (jem::isTiny(theta))
    rv *= 1 / 2.; // infinitesimal rotation
  else
    rv *= theta / (2. * sin(theta));
}

void vec2mat(const Matrix &mat, const Vector &vec)
{
  const idx_t rows = mat.size(0);
  const idx_t cols = mat.size(1);
  JEM_ASSERT2(rows * cols == vec.size(),
              "Vector and Matrix not of the same size!");

  for (idx_t irow = 0; irow < rows; irow++)
  {
    mat(irow, ALL) = vec[SliceFromTo(irow * cols, (irow + 1) * cols)];
  }
}

void mat2vec(const Vector &vec, const Matrix &mat)
{
  const idx_t rows = mat.size(0);
  const idx_t cols = mat.size(1);
  JEM_ASSERT2(rows * cols == vec.size(),
              "Vector and Matrix not of the same size!");

  for (idx_t irow = 0; irow < rows; irow++)
  {
    vec[SliceFromTo(irow * cols, (irow + 1) * cols)] = mat(irow, ALL);
  }
}

void mul3(const Matrix &C, const Matrix &A, const Matrix &B)
{
  Mat3 T;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      T(i, j) = A(i, 0) * B(0, j) + A(i, 1) * B(1, j) + A(i, 2) * B(2, j);
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      C(i, j) = T(i, j);
}

void expVec(const Matrix &Exp, const Vector &psi)
{
  const double theta = norm2(psi);
  const Vec3 p = {psi[0], psi[1], psi[2]};

  if (jem::isTiny(theta))
  {
    Mat3 S;
    skew3(S, p);
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        Exp(i, j) = (i == j ? 1. : 0.) + S(i, j); // infinitesimal rotation
  }
  else
  {
    const Vec3 k = {p[0] / theta, p[1] / theta, p[2] / theta};
    Mat3 K, KK;
    skew3(K, k);
    jem::numeric::matmul(KK, K, K);

    const double s = sin(theta);
    const double c = 1 - cos(theta);
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
      {
        double e = (i == j ? 1. : 0.);
        e += s * K(i, j);
        e += c * KK(i, j);
        Exp(i, j) = e;
      }
  }
}

void expVecP(const Matrix &ExpP, const Vector &psi, const Vector &psiP)
{
  const double theta = norm2(psi);
  const Vec3 p = {psi[0], psi[1], psi[2]};
  const Vec3 pP = {psiP[0], psiP[1], psiP[2]};

  if (theta < TINY)
  {
    Mat3 S;
    skew3(S, pP);
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        ExpP(i, j) = S(i, j);
    return;
  }

  // derivative of norm
  const double thetaP = dot(psi, psiP) / theta;

  Vec3 k, kP;
  for (int i = 0; i < 3; i++)
  {
    k[i] = p[i] / theta;
    // Quotient rule
    kP[i] = (pP[i] * theta - p[i] * thetaP) / theta / theta;
  }

  Mat3 K, KP, KK, KKP, KPK;
  skew3(K, k);
  skew3(KP, kP);
  jem::numeric::matmul(KK, K, K);
  jem::numeric::matmul(KKP, K, KP);
  jem::numeric::matmul(KPK, KP, K);

  const double s = sin(theta);
  const double c = 1 - cos(theta);
  const double cosThetaP = cos(theta) * thetaP;
  const double sinThetaP = s * thetaP;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
    {
      double e = 0.;
      e += cosThetaP * K(i, j);
      e += s * KP(i, j);
      e += sinThetaP * KK(i, j);
      e += c * KKP(i, j);
      e += c * KPK(i, j);
      ExpP(i, j) = e;
    }
}

double trace(const Matrix &mat)
{
  const idx_t rank = mat.size(0);
  double ret = 0.;

  for (idx_t i = 0; i < rank; i++)
  {
    ret += mat(i, i);
  }
  return ret;
}

idx_t factorial(const idx_t n)
{
  JEM_PRECHECK2(n >= 0,
                "Cannot calculate the factorial of a negative number");

  if (n == 0)
    return 1;
  else
    return n * factorial(n - 1);
}

idx_t binom(const idx_t n, const idx_t k)
{
  JEM_PRECHECK2(
      n >= k,
      "Cannot compute the binomial coefficient for n smaller than k");

  return factorial(n) / factorial(k) / factorial(n - k);
}

double matrixNorm2(const Matrix &mat)
{
  double sum = 0;

  for (idx_t i = 0; i < mat.size(0); i++)
    for (idx_t j = 0; j < mat.size(1); j++)
      sum += mat(i, j) * mat(i, j);

  return sqrt(sum);
}

Matrix skew(const Vector &vec)
{
  Matrix res(3, 3);
  res = 0.;

  res(0, 1) = -1 * vec[2];
  res(0, 2) = 1 * vec[1];
  res(1, 2) = -1 * vec[0];
  res(1, 0) = 1 * vec[2];
  res(2, 0) = -1 * vec[1];
  res(2, 1) = 1 * vec[0];

  return res;
};

Vector unskew(const Matrix &mat)
{
#ifndef NDEBUG
  if (fabs(mat(0, 0) + mat(1, 1) + mat(2, 2)) > TINY * matrixNorm2(mat) && fabs(mat(0, 0) + mat(1, 1) + mat(2, 2)) > TINY)
    TEST_NO_CONTEXT(mat)
#endif
  JEM_ASSERT2(fabs(mat(2, 1) + mat(1, 2) + mat(0, 2) + mat(2, 0) +
                   mat(1, 0) + mat(0, 1)) <= TINY * matrixNorm2(mat) ||
                  fabs(mat(2, 1) + mat(1, 2) + mat(0, 2) + mat(2, 0) +
                       mat(1, 0) + mat(0, 1)) <= TINY,
              "Matrix not skew-symmetric");
  JEM_ASSERT2(fabs(mat(0, 0) + mat(1, 1) + mat(2, 2)) <= TINY * matrixNorm2(mat) || fabs(mat(0, 0) + mat(1, 1) + mat(2, 2)) <= TINY,
              "Matrix trace not zero");

  Vector res(3);
  res = 0.;

  res[0] = mat(2, 1) - mat(1, 2);
  res[1] = mat(0, 2) - mat(2, 0);
  res[2] = mat(1, 0) - mat(0, 1);

  res /= 2.;

  return res;
}

void setDofValue(jive::util::Constraints &cons, const idx_t idof, const double rval)
{
  if (cons.isSlaveDof(idof) && cons.masterDofCount(idof) == 0 && cons.hasRvalue(idof))
    cons.setRvalue(idof, rval);
  else
    cons.addConstraint(idof, rval);
}

void linkDofs(jive::util::Constraints &cons, const idx_t idof, const idx_t jdof, const double coeff)
{
  // a prescribed master: JIVE would store idof = coeff * value of jdof
  if (cons.isSlaveDof(jdof) && cons.masterDofCount(jdof) == 0 && cons.hasRvalue(jdof))
  {
    setDofValue(cons, idof, coeff * cons.getRvalue(jdof));
    return;
  }

  if (cons.isSlaveDof(idof) && cons.masterDofCount(idof) == 1 && !cons.hasRvalue(idof))
  {
    double rval;
    IdxVector jdofs(1);
    Vector coeffs(1);
    cons.getConstraint(rval, jdofs, coeffs, idof);

    if (jdofs[0] == jdof && jem::numeric::abs(coeffs[0] - coeff) <= 0.)
      return;
  }

  cons.addConstraint(idof, jdof, coeff);
}
} // namespace jive_helpers
