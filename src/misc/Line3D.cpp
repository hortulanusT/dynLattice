/**
 * @file Line3D.cpp
 * @author Til Gärtner
 * @brief Implementation of shape functions for 1D line elements in 3D space.
 *
 */

#include "misc/Line3D.h"

#include "utils/testing.h"

#include <jem/base/Array.h>
#include <jem/base/ClassTemplate.h>
#include <jem/base/IllegalInputException.h>
#include <jem/numeric/algebra/MatmulChain.h>
#include <jem/numeric/algebra/matmul.h>
#include <jem/numeric/algebra/utilities.h>

#include <jive/geom/ShapeFactory.h>
#include <jive/geom/StdLine.h>

using jive_helpers::expTangent;
using jive_helpers::expTangentD;
using jive_helpers::expTangentInv;
using jive_helpers::expVec;
using jive_helpers::expVecP;
using jive_helpers::logMat;
using jive_helpers::mul3;

using jem::ALL;
using jem::newInstance;
using jem::numeric::matmul;
using jem::numeric::MatmulChain;
using jem::numeric::norm2;
using jive::geom::StdLine;
using jive::geom::StdLine2;
using jive::geom::StdLine3;
using jive::geom::StdLine4;

const String Line3D::PROP_NODES = "numPoints";
const String Line3D::PROP_INT = "intScheme";
const idx_t Line3D::GLOB_RANK = 3;

Line3D::Line3D(const String &name,
               const Properties &conf,
               const Properties &props)
    : Shape(name)
{
  idx_t numNodes;
  String ischemeName;

  Properties myProps = props.getProps(name);
  Properties myConf = conf.makeProps(name);

  myProps.get(numNodes, PROP_NODES);
  myConf.set(PROP_NODES, numNodes);

  Ref<StdLine> stdline;
  switch (numNodes)
  {
  case 2:
    stdline = newInstance<StdLine2>();
    break;
  case 3:
    stdline = newInstance<StdLine3>();
    break;
  case 4:
    stdline = newInstance<StdLine4>();
    break;
  default:
    throw jem::IllegalInputException(getContext(), "given number of nodes not supported!");
  }

  if (!myProps.find(ischemeName, PROP_INT))
    ischemeName = String::format("Gauss%d", numNodes - 1); // reduced integration by default
  myConf.set(PROP_INT, ischemeName);

  Matrix ischeme = StdLine::getIntegrationScheme(ischemeName);

  intLine_ = newInstance<ParametricLine>("internalLine", ischeme, stdline);

  // reference nodes of Lambda_r (CJ Eq. 6.2, 0-based)
  refNodeI_ = (nodeCount() - 1) / 2;
  refNodeJ_ = nodeCount() / 2;

  refRotation_.resize(GLOB_RANK, GLOB_RANK);
  relRotIJ_.resize(GLOB_RANK);
  halfRelRotIJ_.resize(GLOB_RANK);
  nodeLocalRotVecs_.resize(GLOB_RANK, nodeCount());
  ipLocalRotVecs_.resize(GLOB_RANK, ipointCount());
  ipLocalRotVecsP_.resize(GLOB_RANK, ipointCount());
  rotWork_.resize(GLOB_RANK, GLOB_RANK);
}

void Line3D::getGlobalPoint(const Vector &x,
                            const Vector &u,
                            const Matrix &c) const
{
  const Vector shapeFuncs(nodeCount());
  intLine_->evalShapeFunctions(shapeFuncs, u);

  x = matmul(c, shapeFuncs);
}

void Line3D::getIntegrationWeights(const Vector &w,
                                   const Matrix &c) const
{
  Matrix c1(localRank(), nodeCount());
  c1(0, 0) = 0.;
  for (idx_t iNode = 1; iNode < nodeCount(); iNode++) // LATER get something more elaborate that also includes real shapes for higher order elements
    c1[iNode][0] = norm2(c[iNode] - c[iNode - 1]) + c1[iNode - 1][0];

  intLine_->getIntegrationWeights(w, c1);
}

void Line3D::getShapeGradients(const Matrix &g,
                               const Vector &w,
                               const Matrix &c) const
{
  if (gradMemoValid_ && gradMemoCoords_.size(0) == c.size(0) &&
      gradMemoCoords_.size(1) == c.size(1) &&
      jem::testall(gradMemoCoords_ == c))
  {
    g = gradMemoGrads_;
    w = gradMemoWeights_;
    return;
  }

  Matrix c1(localRank(), nodeCount());
  c1(0, 0) = 0.;
  for (idx_t iNode = 1; iNode < nodeCount(); iNode++) // LATER get something more elaborate that also includes real shapes for higher order elements
    c1[iNode][0] = norm2(c[iNode] - c[iNode - 1]) + c1[iNode - 1][0];

  Cubix g1(localRank(), shapeFuncCount(), ipointCount());
  intLine_->getShapeGradients(g1, w, c1);

  g = g1(0, ALL, ALL);

  gradMemoCoords_.resize(c.shape());
  gradMemoCoords_ = c;
  gradMemoGrads_.resize(g.shape());
  gradMemoGrads_ = g;
  gradMemoWeights_.resize(w.size());
  gradMemoWeights_ = w;
  gradMemoValid_ = true;
}

void Line3D::getRotationsAndGradients(const Cubix &Ri,
                                      const Cubix &LambdaP,
                                      const Matrix &grads,
                                      const Cubix &nodeLambda) const
{
  JEM_ASSERT2(LambdaP.size(0) == globalRank() && LambdaP.size(1) == globalRank() && LambdaP.size(2) == ipointCount(), "LambdaP size does not match the expected size");

  getLocalNodeRotVecs_(nodeLambda);
  getLocalIpRotVecs_(grads);

  // Lambda = Lambda_r exp(psi), Lambda' = Lambda_r exp(psi)' (CJ Eq. 5.6, JC Eq. 3.8)
  for (idx_t ip = 0; ip < ipointCount(); ip++)
  {
    expVec(Ri[ip], ipLocalRotVecs_[ip]);
    mul3(Ri[ip], refRotation_, Ri[ip]);
    expVecP(LambdaP[ip], ipLocalRotVecs_[ip], ipLocalRotVecsP_[ip]);
    mul3(LambdaP[ip], refRotation_, LambdaP[ip]);
  }
}

void Line3D::getGeneralisedShapeFunctions(const Quadix &genShapes,
                                          const Quadix &genShapesP,
                                          const Matrix &grads,
                                          const Cubix &nodeLambda) const
{
  JEM_ASSERT2(genShapes.size(2) == nodeCount() && genShapes.size(3) == ipointCount() && genShapesP.size(2) == nodeCount() && genShapesP.size(3) == ipointCount(), "genShapes size does not match the expected size");

  // only used for the tangent stiffness (not hot code), so the work arrays are local
  const idx_t I = refNodeI_;
  const idx_t J = refNodeJ_;
  const Matrix shapes = getShapeFunctions();
  Cubix nodeExpTangentInv(GLOB_RANK, GLOB_RANK, nodeCount()); // T^-1(psi_K) of the nodes
  Matrix refSpinShareI(GLOB_RANK, GLOB_RANK);                 // r_I, share of node I in the spin of Lambda_r
  Matrix refSpinShareJ(GLOB_RANK, GLOB_RANK);                 // r_J
  Cubix localGenShapes(GLOB_RANK, GLOB_RANK, nodeCount());    // X_K = Lambda_r^T It_K Lambda_r
  Cubix localGenShapesP(GLOB_RANK, GLOB_RANK, nodeCount());   // X'_K = Lambda_r^T It'_K Lambda_r
  Matrix localRefWeight(GLOB_RANK, GLOB_RANK);                // a_r, weight of the spin of Lambda_r
  Matrix localRefWeightP(GLOB_RANK, GLOB_RANK);               // a'_r
  Matrix ipExpTangent(GLOB_RANK, GLOB_RANK);                  // T(psi) at the integration point
  Matrix ipExpTangentP(GLOB_RANK, GLOB_RANK);                 // dT(psi; psi')
  Matrix work(GLOB_RANK, GLOB_RANK);
  Matrix work2(GLOB_RANK, GLOB_RANK);

  getLocalNodeRotVecs_(nodeLambda);
  getLocalIpRotVecs_(grads);

  for (idx_t iNode = 0; iNode < nodeCount(); iNode++)
    expTangentInv(nodeExpTangentInv[iNode], nodeLocalRotVecs_[iNode]);

  // r_I, r_J: shares of the reference nodes in the spin of Lambda_r (JC Eq. 3.15, v_I and v_J there)
  if (I != J)
  {
    expTangent(work, halfRelRotIJ_);
    expTangentInv(work2, relRotIJ_);
    mul3(refSpinShareJ, work, work2);
    for (idx_t i = 0; i < 3; i++)
      for (idx_t j = 0; j < 3; j++)
      {
        refSpinShareJ(i, j) *= 0.5;
        refSpinShareI(i, j) = (i == j ? 1. : 0.) - refSpinShareJ(i, j);
      }
  }

  for (idx_t ip = 0; ip < ipointCount(); ip++)
  {
    expTangent(ipExpTangent, ipLocalRotVecs_[ip]);
    expTangentD(ipExpTangentP, ipLocalRotVecs_[ip], ipLocalRotVecsP_[ip]);

    // a_K, a'_K and a_r, a'_r (JC Eqs. 3.13, 3.14, 3.19)
    for (idx_t i = 0; i < 3; i++)
      for (idx_t j = 0; j < 3; j++)
      {
        localRefWeight(i, j) = (i == j ? 1. : 0.);
        localRefWeightP(i, j) = 0.;
      }
    for (idx_t iNode = 0; iNode < nodeCount(); iNode++)
    {
      const double N = shapes(iNode, ip);
      const double dN = grads(iNode, ip);
      mul3(localGenShapes[iNode], ipExpTangent, nodeExpTangentInv[iNode]);
      localGenShapes[iNode] *= N;
      work = N * ipExpTangentP + dN * ipExpTangent;
      mul3(localGenShapesP[iNode], work, nodeExpTangentInv[iNode]);
      localRefWeight -= localGenShapes[iNode];
      localRefWeightP -= localGenShapesP[iNode];
    }

    // X_K = a_K + a_r r_K (JC Eqs. 3.18, 3.19; r_K = 0 for K other than I, J; r_I = I for I = J)
    if (I == J)
    {
      localGenShapes[I] += localRefWeight;
      localGenShapesP[I] += localRefWeightP;
    }
    else
    {
      mul3(work, localRefWeight, refSpinShareI);
      localGenShapes[I] += work;
      mul3(work, localRefWeight, refSpinShareJ);
      localGenShapes[J] += work;
      mul3(work, localRefWeightP, refSpinShareI);
      localGenShapesP[I] += work;
      mul3(work, localRefWeightP, refSpinShareJ);
      localGenShapesP[J] += work;
    }

    // It_K = Lambda_r X_K Lambda_r^T and It'_K = Lambda_r X'_K Lambda_r^T
    for (idx_t iNode = 0; iNode < nodeCount(); iNode++)
    {
      mul3(work, refRotation_, localGenShapes[iNode]);
      mul3(genShapes[ip][iNode], work, refRotation_.transpose());
      mul3(work, refRotation_, localGenShapesP[iNode]);
      mul3(genShapesP[ip][iNode], work, refRotation_.transpose());
    }
  }
}

//------------------------------------------------------
// private helper functions
//------------------------------------------------------
void Line3D::getLocalNodeRotVecs_(const Cubix &Rn) const
{
  // Lambda_r = Lambda_I exp(phi/2), phi = log(Lambda_I^T Lambda_J) (CJ Eq. 5.8, JC Eqs. 3.9, 3.10)
  mul3(rotWork_, Rn[refNodeI_].transpose(), Rn[refNodeJ_]);
  logMat(relRotIJ_, rotWork_);
  halfRelRotIJ_ = 0.5 * relRotIJ_;
  expVec(refRotation_, halfRelRotIJ_);
  mul3(refRotation_, Rn[refNodeI_], refRotation_);

  // psi_K = log(Lambda_r^T Lambda_K) (JC Eq. 3.12)
  for (idx_t iNode = 0; iNode < nodeCount(); iNode++)
  {
    mul3(rotWork_, refRotation_.transpose(), Rn[iNode]);
    logMat(nodeLocalRotVecs_[iNode], rotWork_);
  }
}

void Line3D::getLocalIpRotVecs_(const Matrix &grads) const
{
  // psi = sum_K N_K psi_K (JC Eq. 3.11), psi' = sum_K N'_K psi_K (copied: matmul_ reuses its result)
  ipLocalRotVecs_ = matmul_.matmul(nodeLocalRotVecs_, getShapeFunctions());
  ipLocalRotVecsP_ = matmul_.matmul(nodeLocalRotVecs_, grads);
}
