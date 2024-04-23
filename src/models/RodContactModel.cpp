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
  myProps.makeProps("shape").set("numPoints", allElems_.maxElemNodeCount()); // LATER: option for different number of points in different rod elements?
  shape_ = newInstance<Line3D>("shape", myConf, myProps);
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
    params.get(fint, ActionParams::INT_VECTOR);

    // Get the current displacements.
    StateVector::get(disp, dofs_, globdat);

    // Find the contacts
    findContacts_(elemsA, elemsB, disp);

    // Compute the contact effects
    if (action == Actions::GET_MATRIX0)
    {
      computeContacts_(*mbld, fint, elemsA, elemsB, disp);
    }
    else // action == Actions::GET_INT_VECTOR
    {
      computeContacts_(fint, elemsA, elemsB, disp);
    }

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
  // HACK
  double radius = 0.025;
  // END HACK

  // store the possible contact Pairs
  ArrayBuffer<idx_t> beamACandidates;
  ArrayBuffer<idx_t> beamBCandidates;

  // Get the boxes of the rods
  Matrix elemBoxA(shape_->globalRank(), 2);
  Matrix elemBoxB(shape_->globalRank(), 2);
  // IdxVector dofsA(shape_->nodeCount());
  IdxVector dofsB(shape_->nodeCount());
  // Matrix possA(shape_->globalRank(), 0);
  Matrix possB(shape_->globalRank(), 0);
  bool cornerCheck = false;

  possB.resize(shape_->globalRank(), rodList_[0].getNodeIndices().size());
  allNodes_.getSomeCoords(possB, rodList_[0].getNodeIndices());

  dofsB.resize(rodList_[0].getNodeIndices().size());
  for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
  {
    dofs_->getDofIndices(dofsB, rodList_[0].getNodeIndices(), idof);
    possB(idof, ALL) += disp[dofsB];
  }

  for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
  {
    elemBoxB(idof, 0) = min(possB(idof, ALL)) - radius;
    elemBoxB(idof, 1) = max(possB(idof, ALL)) + radius;
  }

  for (idx_t iRod = 0; iRod < rodList_.size(); iRod++)
  {
    elemBoxA = elemBoxB.clone();

    for (idx_t jRod = rodList_.size() - 1; jRod > iRod; jRod--)
    {
      possB.reshape(shape_->globalRank(), rodList_[jRod].getNodeIndices().size());
      allNodes_.getSomeCoords(possB, rodList_[jRod].getNodeIndices());

      dofsB.resize(rodList_[jRod].getNodeIndices().size());
      for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
      {
        dofs_->getDofIndices(dofsB, rodList_[jRod].getNodeIndices(), idof);
        possB(idof, ALL) += disp[dofsB];
      }

      for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
      {
        elemBoxB(idof, 0) = min(possB(idof, ALL)) - radius;
        elemBoxB(idof, 1) = max(possB(idof, ALL)) + radius;
      }

      cornerCheck = true;
      for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
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
  // HACK
  double radius = 0.025;
  // END HACK

  // store the possible contact Pairs
  ArrayBuffer<idx_t> beamACandidates;
  ArrayBuffer<idx_t> beamBCandidates;

  // Get the boxes of the rods
  Matrix elemBoxA(shape_->globalRank(), 2);
  Matrix elemBoxB(shape_->globalRank(), 2);
  IdxVector nodesA(shape_->nodeCount());
  IdxVector nodesB(shape_->nodeCount());
  IdxVector dofsA(shape_->nodeCount());
  IdxVector dofsB(shape_->nodeCount());
  Matrix possA(shape_->globalRank(), shape_->nodeCount());
  Matrix possB(shape_->globalRank(), shape_->nodeCount());
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

    for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
    {
      elemBoxA(idof, 0) = min(possA(idof, ALL)) - radius;
      elemBoxA(idof, 1) = max(possA(idof, ALL)) + radius;
    }

    for (idx_t iElemB : rodList_[beamB].getIDs())
    {
      allElems_.getElemNodes(nodesB, iElemB);
      allNodes_.getSomeCoords(possB, nodesB);
      for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
      {
        dofs_->getDofIndices(dofsB, nodesB, idof);
        possB(idof, ALL) += disp[dofsB];
      }

      for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
      {
        elemBoxB(idof, 0) = min(possB(idof, ALL)) - radius;
        elemBoxB(idof, 1) = max(possB(idof, ALL)) + radius;
      }

      cornerCheck = true;
      for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
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
  // HACK
  double penalty = 1e9;
  double radius = 0.025;
  // END HACK

  // jem::System::out() << " > > > > Computing contacts (incl Matrix)\n"
  //                    << "between " << elementsA << " and " << elementsB << "\n";

  double uA = 0;
  double uB = 0; // local coordinates between the lines
  Vector distance_vect(shape_->globalRank());
  double penetration = 0.;
  Vector contact_force(shape_->globalRank());
  Vector NA(shape_->shapeFuncCount());
  Vector NB(shape_->shapeFuncCount());

  IdxVector nodesA(shape_->nodeCount());
  IdxVector nodesB(shape_->nodeCount());
  IdxVector dofsA(shape_->nodeCount());
  IdxVector dofsB(shape_->nodeCount());
  Matrix possA(shape_->globalRank(), shape_->nodeCount());
  Matrix possB(shape_->globalRank(), shape_->nodeCount());

  for (idx_t iContact = 0; iContact < elementsA.size(); iContact++)
  {
    allElems_.getElemNodes(nodesA, elementsA[iContact]);
    allNodes_.getSomeCoords(possA, nodesA);
    allElems_.getElemNodes(nodesB, elementsB[iContact]);
    allNodes_.getSomeCoords(possB, nodesB);
    for (idx_t idof = 0; idof < 3; idof++)
    {
      dofs_->getDofIndices(dofsA, nodesA, idof);
      possA(idof, ALL) += disp[dofsA];
      dofs_->getDofIndices(dofsB, nodesB, idof);
      possB(idof, ALL) += disp[dofsB];
    }

    findClosestPoints_(uA, uB, distance_vect, possA, possB);

    if (!shape_->containsLocalPoint(Vector({uA})))
      continue;
    if (!shape_->containsLocalPoint(Vector({uB})))
      continue;
    if (norm2(distance_vect) >= 2 * radius)
      continue;

    penetration = 2 * radius - norm2(distance_vect);
    contact_force = penalty * penetration * distance_vect / norm2(distance_vect);

    SUBHEADER2(elementsA[iContact], uA)
    SUBHEADER2(elementsB[iContact], uB)
    TEST_CONTEXT(contact_force)

    shape_->evalShapeFunctions(NA, Vector({uA}));
    shape_->evalShapeFunctions(NB, Vector({uB}));

    for (idx_t iNode = 0; iNode < shape_->nodeCount(); iNode++)
    {
      for (idx_t idof = 0; idof < shape_->globalRank(); idof++)
      {
        // fint[dofs_->getDofIndex(nodesA[iNode], idof)] -= NA[iNode] * contact_force[idof];
        // fint[dofs_->getDofIndex(nodesB[iNode], idof)] += NB[iNode] * contact_force[idof];

        // TODO torque from the shift of the forces
      }
    }

    // TODO stiffness matrix??
  }
}

//-----------------------------------------------------------------------
//   computeContacts
//-----------------------------------------------------------------------
void RodContactModel::computeContacts_

    (const Vector &fint,
     const IdxVector &elementsA,
     const IdxVector &elementsB,
     const Vector &disp) const
{
  // TODO Compute the contact effects
  // jem::System::out() << " > > > > Computing contacts (only Force)\n"
  //                    << "between " << elementsA << " and " << elementsB << "\n";
}

//-----------------------------------------------------------------------
//   findClosestPoints
//-----------------------------------------------------------------------
void RodContactModel::findClosestPoints_

    (double &uA,
     double &uB,
     const Vector &distance_vect,
     const Matrix &possA,
     const Matrix &possB) const
{
  Vector eA(shape_->globalRank());
  Vector eB(shape_->globalRank());
  double distance;

  switch (shape_->nodeCount())
  {
  case 2:
    // https://math.stackexchange.com/questions/2213165/find-shortest-distance-between-lines-in-3d
    // local cordinates go from  -1 to 1, thus the 0 point is in the middle of the element
    eA = (possA(ALL, 1) - possA(ALL, 0)) / 2;
    eB = (possB(ALL, 1) - possB(ALL, 0)) / 2;
    distance_vect = matmul(skew(eA), eB);
    distance = dotProduct(distance_vect, possB(ALL, 0) + eB - possA(ALL, 0) - eA) / jem::numeric::norm2(distance_vect);

    uA = dotProduct(matmul(skew(eB), distance_vect), possB(ALL, 0) + eB - possA(ALL, 0) - eA) / dotProduct(distance_vect, distance_vect);
    uB = dotProduct(matmul(skew(eA), distance_vect), possB(ALL, 0) + eB - possA(ALL, 0) - eA) / dotProduct(distance_vect, distance_vect);

    distance_vect = distance_vect / jem::numeric::norm2(distance_vect) * distance;
    break;

  case 3:
    /* code */
    throw jem::Error(JEM_FUNC, "Quadratic Elements not implemented yet");
    break;

  case 4:
    /* code */
    throw jem::Error(JEM_FUNC, "Cubic Elements not implemented yet");
    break;

  default:
    throw jem::Error(JEM_FUNC, "Invalid number of nodes in the element");
  }
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
