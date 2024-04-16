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

  findPossibleBeams_(beamsA, beamsB, disp);

  for (idx_t i = 0; i < beamsA.size(); i++)
  {
    findPossibleElements_(elementsA, elementsB, beamsA[i], beamsB[i], disp);
  }
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
  double overlap = 1.02;

  // store the possible contact Pairs
  ArrayBuffer<idx_t> beamACandidates;
  ArrayBuffer<idx_t> beamBCandidates;

  // Get the boxes of the rods
  Matrix elemBoxA(3, 2);
  Matrix elemBoxB(3, 2);
  Matrix possA(3, 2);
  IdxVector dofsA(2);
  Matrix possB(3, 2);
  IdxVector dofsB(2);
  bool cornerCheck = false;

  possB.resize(3, rodList_[0].getNodeIndices().size());
  dofsB.resize(rodList_[0].getNodeIndices().size());

  allNodes_.getSomeCoords(possB, rodList_[0].getNodeIndices());
  for (idx_t idof = 0; idof < 3; idof++)
  {
    dofs_->getDofIndices(dofsB, rodList_[0].getNodeIndices(), idof);
    possB(idof, ALL) += disp[dofsB];
  }

  elemBoxB(0, 0) = min(possB(0, ALL)) - radius * overlap;
  elemBoxB(1, 0) = min(possB(1, ALL)) - radius * overlap;
  elemBoxB(2, 0) = min(possB(2, ALL)) - radius * overlap;
  elemBoxB(0, 1) = max(possB(0, ALL)) + radius * overlap;
  elemBoxB(1, 1) = max(possB(1, ALL)) + radius * overlap;
  elemBoxB(2, 1) = max(possB(2, ALL)) + radius * overlap;

  for (idx_t iRod = 0; iRod < rodList_.size(); iRod++)
  {
    elemBoxA = elemBoxB.clone();

    for (idx_t jRod = rodList_.size() - 1; jRod > iRod; jRod--)
    {
      possB.resize(3, rodList_[jRod].getNodeIndices().size());
      dofsB.resize(rodList_[jRod].getNodeIndices().size());

      allNodes_.getSomeCoords(possB, rodList_[jRod].getNodeIndices());
      for (idx_t idof = 0; idof < 3; idof++)
      {
        dofs_->getDofIndices(dofsB, rodList_[jRod].getNodeIndices(), idof);
        possB(idof, ALL) += disp[dofsB];
      }

      elemBoxB(0, 0) = min(possB(0, ALL)) - radius * overlap;
      elemBoxB(1, 0) = min(possB(1, ALL)) - radius * overlap;
      elemBoxB(2, 0) = min(possB(2, ALL)) - radius * overlap;
      elemBoxB(0, 1) = max(possB(0, ALL)) + radius * overlap;
      elemBoxB(1, 1) = max(possB(1, ALL)) + radius * overlap;
      elemBoxB(2, 1) = max(possB(2, ALL)) + radius * overlap;

      cornerCheck = elemBoxA(0, 1) >= elemBoxB(0, 0) && elemBoxB(0, 1) >= elemBoxA(0, 0);
      cornerCheck &= elemBoxA(1, 1) >= elemBoxB(1, 0) && elemBoxB(1, 1) >= elemBoxA(1, 0);
      cornerCheck &= elemBoxA(2, 1) >= elemBoxB(2, 0) && elemBoxB(2, 1) >= elemBoxA(2, 0);

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

  // store the possible contact Pairs
  ArrayBuffer<idx_t> beamACandidates;
  ArrayBuffer<idx_t> beamBCandidates;

  // Get the boxes of the rods
  IdxVector nodesA(2);
  IdxVector nodesB(2);
  Matrix elemBoxA(3, 2);
  Matrix elemBoxB(3, 2);
  Matrix possA(3, 2);
  IdxVector dofsA(2);
  Matrix possB(3, 2);
  IdxVector dofsB(2);
  bool cornerCheck = false;

  for (idx_t iElemA : rodList_[beamA].getIDs())
  {
    nodesA.resize(allElems_.getElemNodeCount(iElemA));
    possA.resize(3, allElems_.getElemNodeCount(iElemA));
    dofsA.resize(allElems_.getElemNodeCount(iElemA));

    allElems_.getElemNodes(nodesA, iElemA);
    allNodes_.getSomeCoords(possA, nodesA);
    for (idx_t idof = 0; idof < 3; idof++)
    {
      dofs_->getDofIndices(dofsA, nodesA, idof);
      possA(idof, ALL) += disp[dofsA];
    }

    elemBoxA(0, 0) = min(possA(0, ALL)) - radius;
    elemBoxA(1, 0) = min(possA(1, ALL)) - radius;
    elemBoxA(2, 0) = min(possA(2, ALL)) - radius;
    elemBoxA(0, 1) = max(possA(0, ALL)) + radius;
    elemBoxA(1, 1) = max(possA(1, ALL)) + radius;
    elemBoxA(2, 1) = max(possA(2, ALL)) + radius;

    for (idx_t iElemB : rodList_[beamB].getIDs())
    {
      nodesB.resize(allElems_.getElemNodeCount(iElemB));
      possB.resize(3, allElems_.getElemNodeCount(iElemB));
      dofsB.resize(allElems_.getElemNodeCount(iElemB));

      allElems_.getElemNodes(nodesB, iElemB);
      allNodes_.getSomeCoords(possB, nodesB);
      for (idx_t idof = 0; idof < 3; idof++)
      {
        dofs_->getDofIndices(dofsB, nodesB, idof);
        possB(idof, ALL) += disp[dofsB];
      }

      elemBoxB(0, 0) = min(possB(0, ALL)) - radius;
      elemBoxB(1, 0) = min(possB(1, ALL)) - radius;
      elemBoxB(2, 0) = min(possB(2, ALL)) - radius;
      elemBoxB(0, 1) = max(possB(0, ALL)) + radius;
      elemBoxB(1, 1) = max(possB(1, ALL)) + radius;
      elemBoxB(2, 1) = max(possB(2, ALL)) + radius;

      cornerCheck = elemBoxA(0, 1) >= elemBoxB(0, 0) && elemBoxB(0, 1) >= elemBoxA(0, 0);
      cornerCheck &= elemBoxA(1, 1) >= elemBoxB(1, 0) && elemBoxB(1, 1) >= elemBoxA(1, 0);
      cornerCheck &= elemBoxA(2, 1) >= elemBoxB(2, 0) && elemBoxB(2, 1) >= elemBoxA(2, 0);

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
  // TODO Compute the contact effects
  jem::System::out() << " > > > > Computing contacts (only Force)\n"
                     << elementsA << "\n"
                     << elementsB << "\n";
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
  jem::System::out() << " > > > > Computing contacts (only Force)\n"
                     << elementsA << "\n"
                     << elementsB << "\n";
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
