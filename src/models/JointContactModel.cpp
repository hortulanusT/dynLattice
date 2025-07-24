/**
 * @file JointContactModel.cpp
 * @author Til Gärtner
 * @brief contact model for joints (assumed to be two spheres)
 *
 *
 */

#include "models/JointContactModel.h"
#include <jem/base/ClassTemplate.h>

#include "utils/testing.h"

JEM_DEFINE_CLASS(JointContactModel);

//=======================================================================
//   class JointContactModel
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------
const char *JointContactModel::TYPE_NAME = "JointContact";
const char *JointContactModel::PENALTY_PROP = "penalty";
const char *JointContactModel::RADIUS_PROP = "radius";
const char *JointContactModel::VERBOSE_PROP = "verbose";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------
JointContactModel::JointContactModel

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat) : Model(name)
{
  using jive::implict::PropNames;

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
  String jointNames = "points"; // LATER make more general
  Assignable<ElementGroup> joints = ElementGroup::get(jointNames, allElems_, globdat, getContext());
  jointList_.resize(joints.getNodeIndices().size());
  jointList_ = joints.getNodeIndices();

  // get the penalty parameter
  myProps.get(penalty_, PENALTY_PROP);
  myConf.set(PENALTY_PROP, penalty_);

  // get the radius
  myProps.get(radius_, RADIUS_PROP);
  myConf.set(RADIUS_PROP, radius_);

  // initialize the contact update conditions
  if (myProps.contains(PropNames::UPDATE_COND))
    FuncUtils::configCond(updCond_, PropNames::UPDATE_COND, myProps,
                          globdat);
  else
    updCond_ = FuncUtils::newCond(true);
  FuncUtils::getConfig(myConf, updCond_, PropNames::UPDATE_COND);

  contactsA_.clear();
  contactsB_.clear();

  // get the verbosity
  verbose_ = false;
  myProps.find(verbose_, VERBOSE_PROP);
  myConf.set(VERBOSE_PROP, verbose_);
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------
bool JointContactModel::takeAction

    (const String &action,
     const Properties &params,
     const Properties &globdat)
{
  using jive::model::ActionParams;
  using jive::model::Actions;
  using jive::model::StateVector;

  if (action == Actions::GET_MATRIX0 || action == Actions::GET_INT_VECTOR)
  {
    Ref<MatrixBuilder> mbld;
    Vector fint;
    Vector disp;
    String loadCase = "";

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

    // get the load case
    globdat.find(loadCase, jive::app::PropNames::LOAD_CASE);

    if (FuncUtils::evalCond(*updCond_, globdat) || loadCase != "output")
    {
      // find possible contacts if they need to be updated
      findContacts_(disp);
    }

    if (contactsA_.size() == 0) // skip the computation if no actual contact possible
    {
      return true;
    }

    // Compute the contact effects
    computeContacts_(*mbld, fint, contactsA_.toArray(), contactsB_.toArray(), disp);

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

    // Compute the contact effects
    Ref<MatrixBuilder> mbld;
    mbld = newInstance<NullMatrixBuilder>();

    Vector fint(disp.size());
    fint = 0.;

    if (contactsA_.toArray().size() != 0) // skip the computation if no actual contact possible
    {
      computeContacts_(*mbld, fint, contactsA_.toArray(), contactsB_.toArray(), disp);
    }

    // Add the contact forces to the table
    IdxVector jdofs(jtypes.size());

    // iterate through the nodes
    for (idx_t inode : IdxVector(jem::iarray(allNodes_.size())))
    {
      dofs_->getDofIndices(jdofs, inode, jtypes);
      table->addRowValues(inode, jtypes, Vector(fint[jdofs]));
    }

    weights = -1.;

    return true;
  }

  if (action == Actions::COMMIT)
  {
    // TODO add contact stiffness to potential energy!!
  }

  return false;
}

//-----------------------------------------------------------------------
//   findContacts
//-----------------------------------------------------------------------
void JointContactModel::findContacts_

    (const Vector &disp)
{
  contactsA_.clear();
  contactsB_.clear();
  Vector posA(allNodes_.rank());
  Vector posB(allNodes_.rank());
  IdxVector dofsA(allNodes_.rank());
  IdxVector dofsB(allNodes_.rank());

  // iterate through the joints
  for (idx_t ijointA = 0; ijointA < jointList_.size() - 1; ijointA++)
  {
    // get the positions of the node
    dofs_->getDofIndices(dofsA, jointList_[ijointA], IdxVector({0, 1, 2}));
    allNodes_.getNodeCoords(posA, jointList_[ijointA]);
    posA += disp[dofsA];

    for (idx_t ijointB = ijointA + 1; ijointB < jointList_.size(); ijointB++)
    {
      // get the positions of the node
      dofs_->getDofIndices(dofsB, jointList_[ijointB], IdxVector({0, 1, 2}));
      allNodes_.getNodeCoords(posB, jointList_[ijointB]);
      posB += disp[dofsB];

      // check if the nodes are in contact
      if (norm2(posA - posB) <= 2 * radius_)
      {
        contactsA_.pushBack(ijointA);
        contactsB_.pushBack(ijointB);
      }
    }
  }

  if (verbose_)
  {
    if (contactsA_.size() > 0)
      jem::System::debug(myName_) << " > > > Found contacts between joints " << contactsA_.toArray() << " and " << contactsB_.toArray() << "\n";
    else
      jem::System::debug(myName_) << " > > > No contacts found\n";
  }
}

//-----------------------------------------------------------------------
//   computeContacts
//-----------------------------------------------------------------------
void JointContactModel::computeContacts_

    (MatrixBuilder &mbld,
     const Vector &fint,
     const IdxVector &nodesA,
     const IdxVector &nodesB,
     const Vector &disp)

{
  IdxVector dofsA(allNodes_.rank());
  IdxVector dofsB(allNodes_.rank());
  Vector posA(allNodes_.rank());
  Vector posB(allNodes_.rank());
  Matrix contactStiffness(allNodes_.rank() * 2, allNodes_.rank() * 2);
  Vector contactForce(allNodes_.rank() * 2);

  for (idx_t iContact = 0; iContact < nodesA.size(); iContact++)
  {
    // get the positions of the nodes
    allNodes_.getNodeCoords(posA, nodesA[iContact]);
    allNodes_.getNodeCoords(posB, nodesB[iContact]);

    // compute the contact
    computeContact_(contactStiffness, contactForce, posA, posB);

    // get the dofs
    // LATER make the Dofs more flexible and general
    dofs_->getDofIndices(dofsA, nodesA[iContact], IdxVector({0, 1, 2}));
    dofs_->getDofIndices(dofsB, nodesB[iContact], IdxVector({0, 1, 2}));

    if (verbose_)
    {
      jem::System::debug(myName_) << " > > Contact between joints " << nodesA[iContact] << " and " << nodesB[iContact] << "\n";
      jem::System::debug(myName_) << "     resulting force: " << contactForce[jem::SliceTo(3)] << "\n";
    }

    // add the contact forces to the internal forces
    fint[dofsA] += contactForce[jem::SliceTo(3)];
    fint[dofsB] += contactForce[jem::SliceFrom(3)];

    // add the contact stiffness to the matrix
    mbld.addBlock(dofsA, dofsA, contactStiffness(jem::SliceTo(3), jem::SliceTo(3)));
    mbld.addBlock(dofsA, dofsB, contactStiffness(jem::SliceTo(3), jem::SliceFrom(3)));
    mbld.addBlock(dofsB, dofsA, contactStiffness(jem::SliceFrom(3), jem::SliceTo(3)));
    mbld.addBlock(dofsB, dofsB, contactStiffness(jem::SliceFrom(3), jem::SliceFrom(3)));
  }
}

//-----------------------------------------------------------------------
//   computeContact
//-----------------------------------------------------------------------
void JointContactModel::computeContact_

    (Matrix contactStiffness,
     Vector contactForce,
     Vector posA,
     Vector posB)
{
  // compute the contact force
  Vector normal = Vector(posB - posA);
  normal /= norm2(normal);
  Vector force = normal;
  force *= penalty_ * (2. * radius_ - norm2(posA - posB));

  // compute the contact stiffness
  contactStiffness(jem::SliceTo(3), jem::SliceTo(3)) = -penalty_ * matmul(normal, normal);
  contactStiffness(jem::SliceFrom(3), jem::SliceFrom(3)) = penalty_ * matmul(normal, normal);
  contactStiffness(jem::SliceTo(3), jem::SliceFrom(3)) = penalty_ * matmul(normal, normal);
  contactStiffness(jem::SliceFrom(3), jem::SliceTo(3)) = -penalty_ * matmul(normal, normal);

  // set the contact force
  contactForce[jem::SliceTo(3)] = -force;
  contactForce[jem::SliceFrom(3)] = force;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------
Ref<Model> JointContactModel::makeNew

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)
{
  return newInstance<JointContactModel>(name, conf, props, globdat);
}

// ---------------------------------------------------------------------
//   declare
// ---------------------------------------------------------------------
void JointContactModel::declare()
{
  using jive::model::ModelFactory;

  ModelFactory::declare(JointContactModel::TYPE_NAME, &JointContactModel::makeNew);
}
