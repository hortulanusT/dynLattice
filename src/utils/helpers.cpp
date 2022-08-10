#include "utils/helpers.h"

namespace jive_helpers
{
  Matrix eye(const idx_t dim)
  {
    Matrix ret(dim, dim);
    ret = 0.0;

    for (idx_t i = 0; i < dim; i++)
      ret(i, i) = 1.0;

    return ret;
  }

  void logMat(const Vector &rv,
              const Matrix &R)
  {
    double theta;                                 // angle of rotation
    double tr_R = trace(R) <= 3. ? trace(R) : 3.; // trace of the matrix

    theta = acos((tr_R - 1.) / 2.);
    rv = unskew((Matrix)(R - R.transpose()));

    if (jem::isTiny(theta))
      rv *= 1.; // infinitesimal rotation
    else
      rv *= theta / (2. * sin(theta));
  }

  void vec2mat(const Matrix &mat,
               const Vector &vec)
  {
    const idx_t rows = mat.size(0);
    const idx_t cols = mat.size(1);
    JEM_ASSERT2(rows * cols == vec.size(), "Vector and Matrix not of the same size!");

    for (idx_t irow = 0; irow < rows; irow++)
    {
      mat(irow, ALL) = vec[SliceFromTo(irow * cols, (irow + 1) * cols)];
    }
  }

  void expVec(const Matrix &Exp,
              const Vector &psi)
  {
    Exp = eye();
    const double theta = norm2(psi);

    if (jem::isTiny(theta))
      Exp += skew(psi); // infinitesimal rotation
    else
    {
      const Vector k(psi.size());
      const Matrix K(psi.size(), psi.size());

      k = psi / theta;
      K = skew(k);

      Exp += sin(theta) * K;
      Exp += (1 - cos(theta)) * matmul(K, K);
    }
  }

  void expVecP(const Matrix &ExpP,
               const Vector &psi,
               const Vector &psiP)
  {
    const double theta = norm2(psi);

    if (theta < TINY)
    {
      ExpP = skew(psiP);
      return;
    }
    // TEST_NO_CONTEXT(skew(psiP))

    // derivative of norm
    const double thetaP = dot(psi, psiP) / theta;

    const Vector k(psi.size());
    const Vector kP(psi.size());
    const Matrix K(psi.size(), psi.size());
    const Matrix KP(psi.size(), psi.size());

    k = psi / theta;
    // Quotient rule
    kP = (psiP * theta - psi * thetaP) / theta / theta;

    K = skew(k);
    KP = skew(kP);

    // Exp  = eye();
    ExpP = 0;
    // TEST_NO_CONTEXT(ExpP)
    // Exp   += sin(theta) * K;
    ExpP += cos(theta) * thetaP * K;
    ExpP += sin(theta) * KP;
    // TEST_NO_CONTEXT(ExpP)
    // Exp   += (1-cos(theta)) * matmul ( K, K );
    ExpP += sin(theta) * thetaP * matmul(K, K);
    ExpP += (1 - cos(theta)) * matmul(K, KP);
    ExpP += (1 - cos(theta)) * matmul(KP, K);
    // TEST_NO_CONTEXT(ExpP)
  }

  // void getTangentOp
  //   ( const Matrix& T,
  //     const Vector& psi )
  // {
  //   const double theta    = norm2(psi);
  //   const Matrix K        = skew (psi);

  //   T = eye() + 0.5*K;

  //   if (theta < TINY) return;

  //   T += (1. - theta*sin(theta)/2/(1-cos(theta))) / pow(theta, 2) * matmul(K, K);

  //   // const Matrix K        ( psi.size(), psi.size() );
  //   // K       = skew ( psi );

  //   // double a = sin(theta) / theta;
  //   // double b = (1 - a) / pow(theta, 2);

  //   // T      -= 0.5 * K;
  //   // T      += (1 - a / 2 / b) / pow(theta, 2) * matmul(K, K);
  // }

  void rotMat2Quat(const Vector &q,
                   const Matrix &Q)
  {
    double tr = trace(Q) <= 3. ? trace(Q) : 3.; // trace of the matrix;

    if (Q(0, 0) > tr)
    {
      q[1] = sqrt(1. / 2. * Q(0, 0) + 1. / 4. * (1. - tr));
      q[0] = 1. / 4. * (Q(2, 1) - Q(1, 2)) / q[1];
      q[2] = 1. / 4. * (Q(1, 0) + Q(0, 1)) / q[1];
      q[3] = 1. / 4. * (Q(2, 0) + Q(0, 2)) / q[1];
    }
    else if (Q(1, 1) > tr)
    {
      q[2] = sqrt(1. / 2. * Q(1, 1) + 1. / 4. * (1. - tr));
      q[0] = 1. / 4. * (Q(0, 2) - Q(2, 0)) / q[2];
      q[1] = 1. / 4. * (Q(0, 1) + Q(1, 0)) / q[2];
      q[3] = 1. / 4. * (Q(2, 1) + Q(1, 2)) / q[2];
    }
    else if (Q(2, 2) > tr)
    {
      q[3] = sqrt(1. / 2. * Q(2, 2) + 1. / 4. * (1. - tr));
      q[0] = 1. / 4. * (Q(1, 0) - Q(0, 1)) / q[3];
      q[1] = 1. / 4. * (Q(0, 2) + Q(2, 0)) / q[3];
      q[2] = 1. / 4. * (Q(1, 2) + Q(2, 1)) / q[3];
    }
    else
    {
      q[0] = 1. / 2. * sqrt(1. + tr);
      q[1] = 1. / 4. * (Q(2, 1) - Q(1, 2)) / q[0];
      q[2] = 1. / 4. * (Q(0, 2) - Q(2, 0)) / q[0];
      q[3] = 1. / 4. * (Q(1, 0) - Q(0, 1)) / q[0];
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
    JEM_PRECHECK2(n >= 0, "Cannot calculate the factorial of a negative number");

    if (n == 0)
      return 1;
    else
      return n * factorial(n - 1);
  }

  idx_t binom(const idx_t n,
              const idx_t k)
  {
    JEM_PRECHECK2(n >= k, "Cannot compute the binomial coefficient for n smaller than k");

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
    JEM_ASSERT2(fabs(mat(2, 1) + mat(1, 2) + mat(0, 2) + mat(2, 0) + mat(1, 0) + mat(0, 1)) <= TINY * matrixNorm2(mat), "Matrix not skew-symmetric");
    JEM_ASSERT2(fabs(mat(0, 0) + mat(1, 1) + mat(2, 2)) <= TINY, "Matrix trace not zero");

    Vector res(3);
    res = 0.;

    res[0] = mat(2, 1) - mat(1, 2);
    res[1] = mat(0, 2) - mat(2, 0);
    res[2] = mat(1, 0) - mat(0, 1);

    res /= 2.;

    return res;
  }
}