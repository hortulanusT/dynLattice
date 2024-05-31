/**
 * @file RodContactModel.cpp
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief implementation of contact model for rods
 * @version 0.1
 * @date 2024-04-16
 *
 * @copyright Copyright (C) 2024 TU Delft. All rights reserved.
 *
 */

#include "models/RodContactModel.h"
#include <jem/base/ClassTemplate.h>

#include "utils/testing.h"

JEM_DEFINE_CLASS(RodContactModel);

//=======================================================================
//   class RodContactModel
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------
const char *RodContactModel::TYPE_NAME = "RodContact";
const char *RodContactModel::PENALTY_PROP = "penalty";
const char *RodContactModel::RADIUS_PROP = "radius";
const char *RodContactModel::VERBOSE_PROP = "verbose";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------
RodContactModel::RodContactModel

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat) : Model(name)
{
  // Get the Properties associated with this model
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  // Get the nodes
  allNodes_ = NodeSet::get(globdat, getContext());

  // Get the elements
  allElems_ = ElementSet::get(globdat, getContext());

  // Get the DOFSpace
  dofs_ = DofSpace::get(globdat, getContext());

  // Get the list of rods
  StringVector rodNames;
  myProps.get(rodNames, LatticeModel::ROD_LIST_PROP);
  rodList_.resize(rodNames.size());
  for (idx_t iRod = 0; iRod < rodNames.size(); iRod++)
  {
    rodList_[iRod] = ElementGroup::get(rodNames[iRod], allElems_, globdat, getContext());
  }

  // Initialize the internal shape.
  myProps.makeProps("shape").set("numPoints", allElems_.maxElemNodeCount());
  shape_ = newInstance<Line3D>("shape", myConf, myProps);

  // get the penalty parameter and rod radius
  myProps.get(penalty_, PENALTY_PROP);
  myProps.get(radius_, RADIUS_PROP);

  myConf.set(PENALTY_PROP, penalty_);
  myConf.set(RADIUS_PROP, radius_);

  // get the verbosity
  verbose_ = false;
  myProps.find(verbose_, VERBOSE_PROP);
  myConf.set(VERBOSE_PROP, verbose_);
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------
bool RodContactModel::takeAction

    (const String &action,
     const Properties &params,
     const Properties &globdat)
{
  if (action == Actions::GET_MATRIX0 || action == Actions::GET_INT_VECTOR)
  {
    Ref<MatrixBuilder> mbld;
    Vector fint;
    Vector disp;
    IdxVector elemsA;
    IdxVector elemsB;

    // Get the action-specific parameters.
    if (action == Actions::GET_MATRIX0)
    {
      params.get(mbld, ActionParams::MATRIX0);
    }
    else
    {
      mbld = newInstance<NullMatrixBuilder>();
    }
    params.get(fint, ActionParams::INT_VECTOR);

    // Get the current displacements.
    StateVector::get(disp, dofs_, globdat);

    // Find the contacts
    findContacts_(elemsA, elemsB, disp);

    if (elemsA.size() == 0) // skip the computation if no actual contact possible
    {
      return true;
    }

    // Compute the contact effects
    computeContacts_(*mbld, fint, elemsA, elemsB, disp);

    return true;
  }

  if (action == Actions::GET_TABLE)
  {
    Ref<XTable> table;
    Vector weights;
    String name;
    // Get the action-specific parameters.
    params.get(table, ActionParams::TABLE);
    params.get(weights, ActionParams::TABLE_WEIGHTS);
    params.get(name, ActionParams::TABLE_NAME);

    IdxVector jtypes(3);

    if (name == "F_contact")
    {
      jtypes = table->addColumns(dofs_->getTypeNames()[specialCosseratRodModel::TRANS_PART]);
    }
    else if (name == "M_contact")
    {
      jtypes = table->addColumns(dofs_->getTypeNames()[specialCosseratRodModel::ROT_PART]);
    }
    else
    {
      return false;
    }

    // Get the current displacements.
    Vector disp;
    StateVector::get(disp, dofs_, globdat);

    // Find the contacts
    IdxVector elemsA;
    IdxVector elemsB;
    findContacts_(elemsA, elemsB, disp);

    // Compute the contact effects
    Ref<MatrixBuilder> mbld;
    mbld = newInstance<NullMatrixBuilder>();

    Vector fint(disp.size());
    fint = 0.;

    if (elemsA.size() != 0) // skip the computation if no actual contact possible
    {
      computeContacts_(*mbld, fint, elemsA, elemsB, disp);
    }

    // Add the contact forces to the table
    IdxVector jdofs(jtypes.size());

    // iterate through the nodes
    for (idx_t inode : IdxVector(jem::iarray(allNodes_.size())))
    {
      dofs_->getDofIndices(jdofs, inode, jtypes);
      table->addRowValues(inode, jtypes, Vector(fint[jdofs]));
    }

    weights = 1.;

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------
//   findContacts
//-----------------------------------------------------------------------
void RodContactModel::findContacts_

    (IdxVector &elementsA,
     IdxVector &elementsB,
     const Vector &disp) const
{
  IdxVector beamsA;
  IdxVector beamsB;
  // store the possible contact Pairs
  ArrayBuffer<idx_t> beamAElements;
  ArrayBuffer<idx_t> beamBElements;

  findPossibleBeams_(beamsA, beamsB, disp);

  for (idx_t i = 0; i < beamsA.size(); i++)
  {
    IdxVector contactAElements;
    IdxVector contactBElements;

    findPossibleElements_(contactAElements, contactBElements, beamsA[i], beamsB[i], disp);

    beamAElements.pushBack(contactAElements.begin(), contactAElements.end());
    beamBElements.pushBack(contactBElements.begin(), contactBElements.end());
  }

  elementsA.ref(beamAElements.toArray());
  elementsB.ref(beamBElements.toArray());
}

//-----------------------------------------------------------------------
//   findPossibleBeams_
//-----------------------------------------------------------------------

void RodContactModel::findPossibleBeams_

    (IdxVector &beamsA,
     IdxVector &beamsB,
     const Vector &disp) const
{
  const idx_t nodeCount = shape_->nodeCount();
  const idx_t globalRank = shape_->globalRank();

  // store the possible contact Pairs
  ArrayBuffer<idx_t> beamACandidates;
  ArrayBuffer<idx_t> beamBCandidates;

  // Get the boxes of the rods
  Matrix elemBoxA(globalRank, 2);
  Matrix elemBoxB(globalRank, 2);
  // IdxVector dofsA(nodeCount);
  IdxVector dofsB(nodeCount);
  // Matrix possA(globalRank, 0);
  Matrix possB(globalRank, 0);
  bool cornerCheck = false;

  possB.resize(globalRank, rodList_[0].getNodeIndices().size());
  allNodes_.getSomeCoords(possB, rodList_[0].getNodeIndices());

  dofsB.resize(rodList_[0].getNodeIndices().size());
  for (idx_t idof = 0; idof < globalRank; idof++)
  {
    dofs_->getDofIndices(dofsB, rodList_[0].getNodeIndices(), idof);
    possB(idof, ALL) += disp[dofsB];
  }

  for (idx_t idof = 0; idof < globalRank; idof++)
  {
    elemBoxB(idof, 0) = min(possB(idof, ALL)) - radius_;
    elemBoxB(idof, 1) = max(possB(idof, ALL)) + radius_;
  }

  for (idx_t iRod = 0; iRod < rodList_.size(); iRod++)
  {
    elemBoxA = elemBoxB.clone();

    for (idx_t jRod = rodList_.size() - 1; jRod > iRod; jRod--)
    {
      possB.reshape(globalRank, rodList_[jRod].getNodeIndices().size());
      allNodes_.getSomeCoords(possB, rodList_[jRod].getNodeIndices());

      dofsB.resize(rodList_[jRod].getNodeIndices().size());
      for (idx_t idof = 0; idof < globalRank; idof++)
      {
        dofs_->getDofIndices(dofsB, rodList_[jRod].getNodeIndices(), idof);
        possB(idof, ALL) += disp[dofsB];
      }

      for (idx_t idof = 0; idof < globalRank; idof++)
      {
        elemBoxB(idof, 0) = min(possB(idof, ALL)) - radius_;
        elemBoxB(idof, 1) = max(possB(idof, ALL)) + radius_;
      }

      cornerCheck = true;
      for (idx_t idof = 0; idof < globalRank; idof++)
      {
        cornerCheck &= elemBoxA(idof, 1) >= elemBoxB(idof, 0) && elemBoxB(idof, 1) >= elemBoxA(idof, 0);
      }

      if (cornerCheck)
      {
        beamACandidates.pushBack(iRod);
        beamBCandidates.pushBack(jRod);
      }
    }
  }

  beamsA.resize(beamACandidates.size());
  beamsA = beamACandidates.toArray();
  beamsB.resize(beamBCandidates.size());
  beamsB = beamBCandidates.toArray();
}

//-----------------------------------------------------------------------
//   findPossibleElements_
//-----------------------------------------------------------------------
void RodContactModel::findPossibleElements_

    (IdxVector &elemsA,
     IdxVector &elemsB,
     const idx_t &beamA,
     const idx_t &beamB,
     const Vector &disp) const
{
  const idx_t nodeCount = shape_->nodeCount();
  const idx_t globalRank = shape_->globalRank();

  // store the possible contact Pairs
  ArrayBuffer<idx_t> beamACandidates;
  ArrayBuffer<idx_t> beamBCandidates;

  // Get the boxes of the rods
  Matrix elemBoxA(globalRank, 2);
  Matrix elemBoxB(globalRank, 2);
  IdxVector nodesA(nodeCount);
  IdxVector nodesB(nodeCount);
  IdxVector dofsA(nodeCount);
  IdxVector dofsB(nodeCount);
  Matrix possA(globalRank, nodeCount);
  Matrix possB(globalRank, nodeCount);
  bool cornerCheck = false;

  for (idx_t iElemA : rodList_[beamA].getIDs())
  {
    allElems_.getElemNodes(nodesA, iElemA);
    allNodes_.getSomeCoords(possA, nodesA);
    for (idx_t idof = 0; idof < 3; idof++)
    {
      dofs_->getDofIndices(dofsA, nodesA, idof);
      possA(idof, ALL) += disp[dofsA];
    }

    for (idx_t idof = 0; idof < globalRank; idof++)
    {
      elemBoxA(idof, 0) = min(possA(idof, ALL)) - radius_;
      elemBoxA(idof, 1) = max(possA(idof, ALL)) + radius_;
    }

    for (idx_t iElemB : rodList_[beamB].getIDs())
    {
      allElems_.getElemNodes(nodesB, iElemB);
      allNodes_.getSomeCoords(possB, nodesB);
      for (idx_t idof = 0; idof < globalRank; idof++)
      {
        dofs_->getDofIndices(dofsB, nodesB, idof);
        possB(idof, ALL) += disp[dofsB];
      }

      for (idx_t idof = 0; idof < globalRank; idof++)
      {
        elemBoxB(idof, 0) = min(possB(idof, ALL)) - radius_;
        elemBoxB(idof, 1) = max(possB(idof, ALL)) + radius_;
      }

      cornerCheck = true;
      for (idx_t idof = 0; idof < globalRank; idof++)
      {
        cornerCheck &= elemBoxA(idof, 1) >= elemBoxB(idof, 0) && elemBoxB(idof, 1) >= elemBoxA(idof, 0);
      }

      if (cornerCheck)
      {
        beamACandidates.pushBack(iElemA);
        beamBCandidates.pushBack(iElemB);
      }
    }
  }

  elemsA.resize(beamACandidates.size());
  elemsA = beamACandidates.toArray();
  elemsB.resize(beamBCandidates.size());
  elemsB = beamBCandidates.toArray();
}

//-----------------------------------------------------------------------
//   computeContacts
//-----------------------------------------------------------------------
void RodContactModel::computeContacts_

    (MatrixBuilder &mbld,
     const Vector &fint,
     const IdxVector &elementsA,
     const IdxVector &elementsB,
     const Vector &disp) const
{
  const idx_t nodeCount = shape_->nodeCount();
  const idx_t globalRank = shape_->globalRank();

  if (verbose_)
  {
    jem::System::debug(myName_) << " > > > > Computing contacts";
    if (mbld.getClass() == NullMatrixBuilder::getType())
      jem::System::debug(myName_) << " without stiffness matrix";
    else
      jem::System::debug(myName_) << " with stiffness matrix";
    jem::System::debug(myName_) << "\n";
  }

  IdxVector nodesA(nodeCount);
  IdxVector nodesB(nodeCount);
  Matrix possA(globalRank, nodeCount);
  Matrix possB(globalRank, nodeCount);
  IdxMatrix dofsA(globalRank, nodeCount);
  IdxMatrix dofsB(globalRank, nodeCount);
  IdxVector dofsAB(globalRank * nodeCount * 2);

  double uA = 0; // local coordinates
  double uB = 0;

  idx_t iNodeA;
  idx_t iNodeB;

  Vector f_contrib;
  Matrix k_contrib;

  bool contact_closed;

  for (idx_t iContact = 0; iContact < elementsA.size(); iContact++)
  {
    allElems_.getElemNodes(nodesA, elementsA[iContact]);
    allNodes_.getSomeCoords(possA, nodesA);
    allElems_.getElemNodes(nodesB, elementsB[iContact]);
    allNodes_.getSomeCoords(possB, nodesB);

    for (idx_t iNode = 0; iNode < nodeCount; iNode++)
    {
      dofs_->getDofIndices(dofsA[iNode], nodesA[iNode], IdxVector({0, 1, 2}));
      possA[iNode] += disp[dofsA[iNode]];
      dofs_->getDofIndices(dofsB[iNode], nodesB[iNode], IdxVector({0, 1, 2}));
      possB[iNode] += disp[dofsB[iNode]];
    }

    findClosestPoints_(uA, uB, possA, possB);

    if (verbose_)
      jem::System::debug(myName_) << " > > Contact detection between elements " << elementsA[iContact] << " and " << elementsB[iContact]
                                  << " at local coordinates " << uA << " and " << uB << " ==} ";

    contact_closed = false;

    if (shape_->containsLocalPoint(Vector({uA})) && shape_->containsLocalPoint(Vector({uB})))
    {
      // LATER generalize for higher order elements
      dofsAB.resize(globalRank * 4);

      f_contrib.resize(2 * nodeCount * globalRank);
      f_contrib = 0.;
      k_contrib.resize(2 * nodeCount * globalRank, 2 * nodeCount * globalRank);
      k_contrib = 0.;

      contact_closed = computeSTS_(f_contrib, k_contrib, possA, possB, uA, uB);

      if (contact_closed && verbose_)
        jem::System::debug(myName_) << "STS contact between elements " << elementsA[iContact] << " and " << elementsB[iContact] << "\n";
      if (!contact_closed && verbose_)
        jem::System::debug(myName_) << "NO contact\n";

      if (!contact_closed)
        continue;

      dofsAB[SliceFromTo(0 * globalRank, 1 * globalRank)] = dofsA[0];
      dofsAB[SliceFromTo(1 * globalRank, 2 * globalRank)] = dofsA[1];
      dofsAB[SliceFromTo(2 * globalRank, 3 * globalRank)] = dofsB[0];
      dofsAB[SliceFromTo(3 * globalRank, 4 * globalRank)] = dofsB[1];
    }
    else if (!shape_->containsLocalPoint(Vector({uA})) && shape_->containsLocalPoint(Vector({uB})))
    {
      // LATER generalize for higher order elements
      dofsAB.resize(globalRank * 3);

      f_contrib.resize((nodeCount + 1) * globalRank);
      f_contrib = 0.;
      k_contrib.resize((nodeCount + 1) * globalRank, (nodeCount + 1) * globalRank);
      k_contrib = 0.;

      if (uA < -1.)
      {
        iNodeA = 0;
      }
      else // uA > 1.
      {
        iNodeA = 1;
      }

      uB = getClosestPoint_(possB[iNodeA], possA);
      if (!(shape_->containsLocalPoint(Vector({uB}))))
      {
        if (verbose_)
          jem::System::debug(myName_) << "NO contact\n";
        continue;
      }

      contact_closed = computeNTS_(f_contrib, k_contrib, possA[iNodeA], possB, uB);

      if (contact_closed && verbose_)
        jem::System::debug(myName_) << "(direct) NTS contact between node " << nodesA[iNodeA] << " and element " << elementsB[iContact] << "\n";
      if (!contact_closed && verbose_)
        jem::System::debug(myName_) << "NO contact\n";

      if (!contact_closed)
        continue;

      dofsAB[SliceFromTo(0 * globalRank, 1 * globalRank)] = dofsA[iNodeA];
      dofsAB[SliceFromTo(1 * globalRank, 2 * globalRank)] = dofsB[0];
      dofsAB[SliceFromTo(2 * globalRank, 3 * globalRank)] = dofsB[1];
    }
    else if (!shape_->containsLocalPoint(Vector({uB})) && shape_->containsLocalPoint(Vector({uA})))
    {
      dofsAB.resize(globalRank * 3);

      f_contrib.resize((nodeCount + 1) * globalRank);
      f_contrib = 0.;
      k_contrib.resize((nodeCount + 1) * globalRank, (nodeCount + 1) * globalRank);
      k_contrib = 0.;

      if (uB < -1.)
      {
        iNodeB = 0;
      }
      else // uB > 1.
      {
        iNodeB = 1;
      }

      uA = getClosestPoint_(possB[iNodeB], possA);
      if (!(shape_->containsLocalPoint(Vector({uA}))))
      {
        if (verbose_)
          jem::System::debug(myName_) << "NO contact\n";
        continue;
      }

      contact_closed = computeNTS_(f_contrib, k_contrib, possB[iNodeB], possA, uA);

      if (contact_closed && verbose_)
        jem::System::debug(myName_) << "(direct) NTS contact between node " << nodesB[iNodeB] << " and element " << elementsA[iContact] << "\n";
      if (!contact_closed && verbose_)
        jem::System::debug(myName_) << "NO contact\n";

      if (!contact_closed)
        continue;

      dofsAB[SliceFromTo(0 * globalRank, 1 * globalRank)] = dofsB[iNodeB];
      dofsAB[SliceFromTo(1 * globalRank, 2 * globalRank)] = dofsA[0];
      dofsAB[SliceFromTo(2 * globalRank, 3 * globalRank)] = dofsA[1];
    }
    else // !shape_->containsLocalPoint(Vector({uA})) && !shape_->containsLocalPoint(Vector({uB}))
    {
      // LATER generalize for higher order elements
      dofsAB.resize(globalRank * 4);

      f_contrib.resize(2 * nodeCount * globalRank);
      f_contrib = 0.;
      k_contrib.resize(2 * nodeCount * globalRank, 2 * nodeCount * globalRank);
      k_contrib = 0.;

      Vector f_contribLocal((nodeCount + 1) * globalRank);
      Matrix k_contribLocal((nodeCount + 1) * globalRank, (nodeCount + 1) * globalRank);

      // check both ends of A against B
      for (idx_t iNodeA : {0, 1})
      {
        uB = getClosestPoint_(possA[iNodeA], possB);
        if (!(shape_->containsLocalPoint(Vector({uB}))))
          continue;

        f_contribLocal = 0.;
        k_contribLocal = 0.;

        contact_closed |= computeNTS_(f_contribLocal, k_contribLocal, possA[iNodeA], possB, uB);

        if (contact_closed && verbose_)
          jem::System::debug(myName_) << "(indirect) NTS contact between node " << nodesA[iNodeA] << " and element " << elementsB[iContact] << "\n";

        if (contact_closed)
        {
          f_contrib[SliceFromTo(iNodeA * globalRank, (iNodeA + 1) * globalRank)] += f_contribLocal[SliceTo(globalRank)];
          f_contrib[SliceFromTo(2 * globalRank, 4 * globalRank)] += f_contribLocal[SliceFrom(globalRank)];
          k_contrib(SliceFromTo(iNodeA * globalRank, (iNodeA + 1) * globalRank), SliceFromTo(iNodeA * globalRank, (iNodeA + 1) * globalRank)) += k_contribLocal(SliceTo(globalRank), SliceTo(globalRank));
          k_contrib(SliceFromTo(iNodeA * globalRank, (iNodeA + 1) * globalRank), SliceFromTo(2 * globalRank, 4 * globalRank)) += k_contribLocal(SliceTo(globalRank), SliceFrom(globalRank));
          k_contrib(SliceFromTo(2 * globalRank, 4 * globalRank), SliceFromTo(iNodeA * globalRank, (iNodeA + 1) * globalRank)) += k_contribLocal(SliceFrom(globalRank), SliceTo(globalRank));
          k_contrib(SliceFromTo(2 * globalRank, 4 * globalRank), SliceFromTo(2 * globalRank, 4 * globalRank)) += k_contribLocal(SliceFrom(globalRank), SliceFrom(globalRank));
        }
      }

      // check both ends of B against A
      for (idx_t iNodeB : {0, 1})
      {
        uA = getClosestPoint_(possB[iNodeB], possA);
        if (!(shape_->containsLocalPoint(Vector({uA}))))
          continue;

        f_contribLocal = 0.;
        k_contribLocal = 0.;

        contact_closed |= computeNTS_(f_contribLocal, k_contribLocal, possB[iNodeB], possA, uA);

        if (contact_closed && verbose_)
          jem::System::debug(myName_) << "(indirect) NTS contact between node " << nodesB[iNodeB] << " and element " << elementsA[iContact] << "\n";

        if (contact_closed)
        {
          f_contrib[SliceFromTo(0 * globalRank, 2 * globalRank)] += f_contribLocal[SliceFrom(globalRank)];
          f_contrib[SliceFromTo((2 + iNodeB) * globalRank, (3 + iNodeB) * globalRank)] += f_contribLocal[SliceTo(globalRank)];

          k_contrib(SliceFromTo(0 * globalRank, 2 * globalRank), SliceFromTo(0 * globalRank, 2 * globalRank)) += k_contribLocal(SliceFrom(globalRank), SliceFrom(globalRank));
          k_contrib(SliceFromTo(0 * globalRank, 2 * globalRank), SliceFromTo((2 + iNodeB) * globalRank, (3 + iNodeB) * globalRank)) += k_contribLocal(SliceFrom(globalRank), SliceTo(globalRank));
          k_contrib(SliceFromTo((2 + iNodeB) * globalRank, (3 + iNodeB) * globalRank), SliceFromTo(0 * globalRank, 2 * globalRank)) += k_contribLocal(SliceTo(globalRank), SliceFrom(globalRank));
          k_contrib(SliceFromTo((2 + iNodeB) * globalRank, (3 + iNodeB) * globalRank), SliceFromTo((2 + iNodeB) * globalRank, (3 + iNodeB) * globalRank)) += k_contribLocal(SliceTo(globalRank), SliceTo(globalRank));
        }
      }

      if (!contact_closed && verbose_)
        jem::System::debug(myName_) << "NO contact\n";

      if (!contact_closed)
        continue;

      dofsAB[SliceFromTo(0 * globalRank, 1 * globalRank)] = dofsA[0];
      dofsAB[SliceFromTo(1 * globalRank, 2 * globalRank)] = dofsA[1];
      dofsAB[SliceFromTo(2 * globalRank, 3 * globalRank)] = dofsB[0];
      dofsAB[SliceFromTo(3 * globalRank, 4 * globalRank)] = dofsB[1];
    }

    fint[dofsAB] += f_contrib;
    mbld.addBlock(dofsAB, dofsAB, k_contrib);
  } // end of loop over contacts

  if (verbose_)
    jem::System::debug(myName_) << " > > > > Done computing contacts\n";
}

//-----------------------------------------------------------------------
//   findClosestPoints
//-----------------------------------------------------------------------
void RodContactModel::findClosestPoints_

    (double &uA,
     double &uB,
     const Matrix &possA,
     const Matrix &possB) const
{
  Vector bA(shape_->globalRank());
  Vector bB(shape_->globalRank());
  Vector tA(shape_->globalRank());
  Vector tB(shape_->globalRank());

  switch (shape_->nodeCount())
  {
  case 2:
    // Wriggers/Zavarise 1997
    bA = possA(ALL, 1) + possA(ALL, 0);
    bB = possB(ALL, 1) + possB(ALL, 0);
    tA = possA(ALL, 1) - possA(ALL, 0);
    tB = possB(ALL, 1) - possB(ALL, 0);

    uA = -1. * dotProduct(bB - bA, tB * dotProduct(tB, tA) - tA * dotProduct(tB, tB)) / (dotProduct(tB, tB) * dotProduct(tA, tA) - dotProduct(tB, tA) * dotProduct(tB, tA));
    uB = +1. * dotProduct(bB - bA, tA * dotProduct(tB, tA) - tB * dotProduct(tA, tA)) / (dotProduct(tB, tB) * dotProduct(tA, tA) - dotProduct(tB, tA) * dotProduct(tB, tA));
    break;

  case 3:
    /* LATER Quadratic Elements */
    throw jem::Error(JEM_FUNC, "Quadratic Elements not implemented yet");
    break;

  case 4:
    /* LATER Cubic Elements */
    throw jem::Error(JEM_FUNC, "Cubic Elements not implemented yet");
    break;

  default:
    throw jem::Error(JEM_FUNC, "Invalid number of nodes in the element");
  }
}

//-----------------------------------------------------------------------
//   getClosestPoint_
//-----------------------------------------------------------------------
double RodContactModel::getClosestPoint_

    (const Vector &posS,
     const Matrix &possM) const
{
  Vector bM(shape_->globalRank());
  Vector tM(shape_->globalRank());

  switch (shape_->nodeCount())
  {
  case 2:
    bM = possM(ALL, 1) + possM(ALL, 0);
    tM = possM(ALL, 1) - possM(ALL, 0);

    return (2 * dotProduct(posS, tM) - dotProduct(tM, bM)) / dotProduct(tM, tM);
    break;

  case 3:
    /* LATER Quadratic Elements */
    throw jem::Error(JEM_FUNC, "Quadratic Elements not implemented yet");
    break;

  case 4:
    /* LATER Cubic Elements */
    throw jem::Error(JEM_FUNC, "Cubic Elements not implemented yet");
    break;

  default:
    throw jem::Error(JEM_FUNC, "Invalid number of nodes in the element");
  }
}

//-----------------------------------------------------------------------
//   computeSTS_
//-----------------------------------------------------------------------
bool RodContactModel::computeSTS_

    (Vector &f_contrib,
     Matrix &k_contrib,
     const Matrix &possA,
     const Matrix &possB,
     const double uA,
     const double uB) const
{
  const idx_t nodeCount = shape_->nodeCount();
  const idx_t globalRank = shape_->globalRank();

  Vector pA(globalRank); // positions
  Vector pB(globalRank);

  shape_->getGlobalPoint(pA, Vector({uA}), possA);
  shape_->getGlobalPoint(pB, Vector({uB}), possB);

  double distance = norm2(pB - pA);

  if (distance > 2. * radius_)
    return false;

  Vector dpA(globalRank); // positions gradients
  Vector dpB(globalRank);
  Vector ddpA(globalRank); // positions second gradients
  Vector ddpB(globalRank);
  Vector contact_normal(globalRank);

  Vector N_A(nodeCount); // shape functions
  Vector N_B(nodeCount);
  Vector dN_A(nodeCount); // shape function gradients
  Vector dN_B(nodeCount);
  Vector ddN_A(nodeCount); // shape function second gradients
  Vector ddN_B(nodeCount);
  Matrix H_A(globalRank, globalRank * nodeCount); // shape function matrix
  Matrix H_B(globalRank, globalRank * nodeCount);
  Matrix dH_A(globalRank, globalRank * nodeCount); // shape function gradient matrix
  Matrix dH_B(globalRank, globalRank * nodeCount);

  Matrix H_tilde(globalRank, globalRank * nodeCount * 2);    // composed shape function matrix
  Matrix H_hat(globalRank * 2, globalRank * nodeCount * 2);  // composed shape function matrix
  Matrix dH_hat(globalRank * 2, globalRank * nodeCount * 2); // composed shape function gradient matrix

  Matrix A(2, 2);
  Matrix B(2, 2 * globalRank);
  Matrix C(2, 2 * globalRank);
  Matrix D(2, 2 * nodeCount * globalRank);
  Matrix E(2 * nodeCount * globalRank, 2 * nodeCount * globalRank);
  // Matrix F(2 * nodeCount * globalRank, 2 * nodeCount * globalRank);
  Matrix G(2 * nodeCount * globalRank, 2 * nodeCount * globalRank);

  contact_normal = (pB - pA) / distance;
  shape_->evalShapeGradGrads(N_A, dN_A, ddN_A, Vector({uA}));
  shape_->evalShapeGradGrads(N_B, dN_B, ddN_B, Vector({uB}));

  dpA = matmul(possA, dN_A);
  dpB = matmul(possB, dN_B);
  ddpA = matmul(possA, ddN_A);
  ddpB = matmul(possB, ddN_B);

  for (idx_t iNode = 0; iNode < nodeCount; iNode++)
  {
    H_A(ALL, SliceFromTo(iNode * globalRank, (iNode + 1) * globalRank)) = N_A[iNode] * eye(globalRank);
    H_B(ALL, SliceFromTo(iNode * globalRank, (iNode + 1) * globalRank)) = N_B[iNode] * eye(globalRank);
    dH_A(ALL, SliceFromTo(iNode * globalRank, (iNode + 1) * globalRank)) = dN_A[iNode] * eye(globalRank);
    dH_B(ALL, SliceFromTo(iNode * globalRank, (iNode + 1) * globalRank)) = dN_B[iNode] * eye(globalRank);
  }
  H_tilde(ALL, SliceTo(globalRank * nodeCount)) = -1. * H_A;
  H_tilde(ALL, SliceFrom(globalRank * nodeCount)) = H_B;

  H_hat = 0.;
  dH_hat = 0.;

  H_hat(SliceTo(globalRank), SliceTo(globalRank * nodeCount)) = H_A;
  H_hat(SliceFrom(globalRank), SliceFrom(globalRank * nodeCount)) = H_B;
  dH_hat(SliceTo(globalRank), SliceTo(globalRank * nodeCount)) = dH_A;
  dH_hat(SliceFrom(globalRank), SliceFrom(globalRank * nodeCount)) = dH_B;

  A(0, 0) = -1. * dotProduct(dpA, dpA) + dotProduct(pB - pA, ddpA);
  A(0, 1) = dotProduct(dpB, dpA);
  A(1, 0) = -1. * dotProduct(dpA, dpB);
  A(1, 1) = dotProduct(dpB, dpB) - dotProduct(pB - pA, ddpB);

  B(0, SliceTo(globalRank)) = dpA;
  B(0, SliceFrom(globalRank)) = -1. * dpA;
  B(1, SliceTo(globalRank)) = dpB;
  B(1, SliceFrom(globalRank)) = -1. * dpB;

  C = 0.;
  C(0, SliceTo(globalRank)) = -1. * (pB - pA);
  C(1, SliceFrom(globalRank)) = -1. * (pB - pA);

  D = matmul(jem::numeric::inverse(A), Matrix(matmul(B, H_hat) + matmul(C, dH_hat)));

  E(SliceTo(globalRank * nodeCount), ALL) = matmul(matmul(dH_A.transpose(), contact_normal), D(0, ALL));
  E(SliceFrom(globalRank * nodeCount), ALL) = matmul(matmul(dH_B.transpose(), contact_normal), D(1, ALL));

  if (norm2(ddpA) + norm2(ddpB) > 1e-12)
  {
    // LATER Implement 'F' for Higher Order Elements
    throw jem::Error(JEM_FUNC, "Higher Order Elements not implemented yet");
  }

  G = matmul(matmul(Matrix(H_tilde.transpose() + matmul(D(1, ALL), dpB) - matmul(D(0, ALL), dpA)), Matrix(eye(globalRank) - matmul(contact_normal, contact_normal))), Matrix(H_tilde + matmul(D(1, ALL), dpB).transpose() - matmul(D(0, ALL), dpA).transpose())) / distance;

  f_contrib += penalty_ * (distance - 2. * radius_) * matmul(H_tilde.transpose(), contact_normal);
  k_contrib += penalty_ * matmul(matmul(H_tilde.transpose(), matmul(contact_normal, contact_normal)), H_tilde);
  k_contrib += penalty_ * (distance - 2. * radius_) * (E + E.transpose() + G);

  return true;
}

//-----------------------------------------------------------------------
//   computeNTS_
//-----------------------------------------------------------------------
bool RodContactModel::computeNTS_

    (Vector &f_contrib,
     Matrix &k_contrib,
     const Vector &possS,
     const Matrix &possM,
     const double uM) const
{
  const idx_t globalRank = shape_->globalRank();

  Vector pM(globalRank); // position

  shape_->getGlobalPoint(pM, Vector({uM}), possM);

  double distance = norm2(pM - possS);

  if (distance > 2. * radius_)
    return false;

  // Wriggers/Simo 1985
  Vector n(globalRank);
  Vector t(globalRank);
  n = (possS - pM) / distance;
  t = (possM[1] - possM[0]) / norm2(possM[1] - possM[0]);

  Vector Ns(3 * globalRank);
  Vector Ts(3 * globalRank);
  Vector N(3 * globalRank);

  Ns[SliceTo(globalRank)] = n;
  Ts[SliceTo(globalRank)] = t;
  N[SliceTo(globalRank)] = 0.;

  Ns[SliceFromTo(globalRank, 2 * globalRank)] = -.5 * (1. - uM) * n; // change since uM is in [-1,1] and not [0,1]
  Ts[SliceFromTo(globalRank, 2 * globalRank)] = -.5 * (1. - uM) * t;
  N[SliceFromTo(globalRank, 2 * globalRank)] = -1. * n;

  Ns[SliceFrom(2 * globalRank)] = -.5 * (1. + uM) * n;
  Ts[SliceFrom(2 * globalRank)] = -.5 * (1. + uM) * t;
  N[SliceFrom(2 * globalRank)] = n;

  f_contrib += penalty_ * (distance - 2. * radius_) * Ns;
  k_contrib += penalty_ * (matmul(Ns, Ns) - (distance - 2. * radius_) / norm2(possM[1] - possM[0]) * (matmul(N, Ts) + matmul(Ts, N) + (distance - 2. * radius_) / norm2(possM[1] - possM[0]) * matmul(N, N)));

  return true;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------
Ref<Model> RodContactModel::makeNew

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)
{
  return newInstance<RodContactModel>(name, conf, props, globdat);
}

// ---------------------------------------------------------------------
//   declare
// ---------------------------------------------------------------------
void RodContactModel::declare()
{
  using jive::model::ModelFactory;

  ModelFactory::declare(RodContactModel::TYPE_NAME, &RodContactModel::makeNew);
}
