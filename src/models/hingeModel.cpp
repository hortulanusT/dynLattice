/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *
 * This class implements hinges for geometrically nonlinear rods
 *
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: July 21
 *
 */

#include "hingeModel.h"
//=======================================================================
//    class specialCosseratRodModel -- implementation
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *hingeModel::TYPE_NAME = "rigidHinge";
const char *hingeModel::LIMIT_LOADS = "limitLoads";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------

hingeModel::hingeModel

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat) :

                                  Model(name)

{
  // Get the Properties associated with this model
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  String elementName;
  myProps.get(elementName, "elements");

  // Get the elements and nodes from the global database.
  elems_ = ElementSet::get(globdat, getContext()); // all the elements
  nodes_ = elems_.getNodes();                      // all the nodes

  egroup_ = ElementGroup::find(elementName, elems_, globdat); // only the desired group
  if (!egroup_)
    egroup_ = createHinges_(elementName, globdat);

  myConf.set("elements", elementName);

  // get the limit loads
  Vector limits;
  myProps.get(limits, LIMIT_LOADS);
  myConf.set(LIMIT_LOADS, limits);

  limits_.resize(limits.size(), egroup_.size());
  for (idx_t i = 0; i < limits.size(); i++)
  {
    limits_(i, ALL) = limits[i];
  }
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------

// Performs an action requested by a module or a parent model.

bool hingeModel::takeAction

    (const String &action,
     const Properties &params,
     const Properties &globdat)

{
  using jive::model::ActionParams;
  using jive::model::Actions;
  using jive::model::StateVector;

  if (action == Actions::INIT)
  {
    init_(globdat);
    return true;
  }

  if (action == Actions::GET_CONSTRAINTS)
  {
    getCons_();
    return true;
  }

  if (action == Actions::GET_MATRIX0 || action == Actions::GET_INT_VECTOR)
  {
    Vector fint;
    params.get(fint, ActionParams::INT_VECTOR);
    updForces_(fint);

    return true;
  }

  if (action == Actions::CHECK_COMMIT)
  {
    Vector disp;
    StateVector::get(disp, dofs_, globdat);
    bool accepted = evalPlastic_(disp);
    params.set(ActionParams::ACCEPT, accepted);

    return true;
  }

  if (action == Actions::COMMIT)
  {
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------
//   init_
//-----------------------------------------------------------------------

void hingeModel::init_(const Properties &globdat)
{
  dofs_ = DofSpace::get(globdat, getContext());    // all the dofs
  constraints_ = Constraints::get(dofs_, globdat); // all the constraints

  jtypes_.resize(dofs_->typeCount());
  for (idx_t i = 0; i < jtypes_.size(); i++)
    jtypes_[i] = dofs_->getTypeIndex(dofs_->getTypeNames()[i]);

  JEM_PRECHECK2(jtypes_.size() == limits_.size(0), "DOF types and limit types do not match!");

  intForces_.resize(jtypes_.size(), egroup_.size());
  plasticDisp_.resize(jtypes_.size(), egroup_.size());

  intForces_ = 0.;
  plasticDisp_ = 0.;
}

//-----------------------------------------------------------------------
//   getCons_
//-----------------------------------------------------------------------

void hingeModel::getCons_()
{
  IdxVector dofsA(jtypes_.size());
  IdxVector dofsB(jtypes_.size());
  IdxVector inodes(2);

  for (idx_t ielem = 0; ielem < egroup_.size(); ielem++)
  {
    elems_.getElemNodes(inodes, egroup_.getIndex(ielem));

    dofs_->getDofIndices(dofsA, inodes[0], jtypes_);
    dofs_->getDofIndices(dofsB, inodes[1], jtypes_);

    for (idx_t idof = 0; idof < jtypes_.size(); idof++)
      constraints_->addConstraint(dofsA[idof], plasticDisp_[ielem][idof], dofsB[idof], 1.);
  }
}

//-----------------------------------------------------------------------
//   updForces_
//-----------------------------------------------------------------------

void hingeModel::updForces_(const Vector &fint)
{
  IdxVector inodes(2);
  idx_t jdofA, jdofB;

  for (idx_t ielem = 0; ielem < egroup_.size(); ielem++)
  {
    elems_.getElemNodes(inodes, egroup_.getIndex(ielem));
    for (idx_t idof = 0; idof < jtypes_.size(); idof++)
    {
      jdofA = dofs_->getDofIndex(inodes[0], idof);
      jdofB = dofs_->getDofIndex(inodes[1], idof);

      intForces_[ielem][idof] = fint[jdofA];
    }
  }
}

//-----------------------------------------------------------------------
//   evalPlastic_
//-----------------------------------------------------------------------

bool hingeModel::evalPlastic_(const Vector &disp)
{
  idx_t dofA, dofB;
  IdxVector inodes(2);

  bool checked = true;

  for (idx_t ielem = 0; ielem < egroup_.size(); ielem++)
    for (idx_t idof = 0; idof < jtypes_.size(); idof++)
      if (fabs(intForces_[ielem][idof]) > limits_[ielem][idof])
      {
        checked = false;

        elems_.getElemNodes(inodes, egroup_.getIndex(ielem));

        dofA = dofs_->getDofIndex(inodes[0], jtypes_[idof]);
        dofB = dofs_->getDofIndex(inodes[1], jtypes_[idof]);

        plasticDisp_[ielem][idof] -= (fabs(intForces_[ielem][idof]) - limits_[ielem][idof]) / 10.;
        limits_[ielem][idof] = fabs(intForces_[ielem][idof]);
      }

  return checked;
}

//-----------------------------------------------------------------------
//   createHinges_
//-----------------------------------------------------------------------

ElementGroup hingeModel::createHinges_(const String &elementName, const Properties &globdat) const
{
  IdxVector inodes_base, inodes_moving, newnodes_moving;
  idx_t newNode, newElem;
  Vector coords(nodes_.rank());
  IdxBuffer newElems;

  Assignable<XNodeSet>
      xnodes = XNodeSet::get(globdat, getContext());
  Assignable<XElementSet> xelems = XElementSet::get(globdat, getContext());
  Assignable<ElementGroup> motherElems = ElementGroup::get(jem::util::StringUtils::split(elementName, '.')[0], xelems, globdat, getContext());

  jem::System::info(myName_) << " ...Creating " << myName_ << "\n";

  for (idx_t ielem_base : motherElems.getIndices())
  {
    inodes_base.resize(xelems.getElemNodeCount(ielem_base));
    xelems.getElemNodes(inodes_base, ielem_base);

    for (idx_t inode_base : inodes_base)
      for (idx_t ielem_moving : motherElems.getIndices())
      {
        if (ielem_moving == ielem_base)
          continue;

        inodes_moving.resize(xelems.getElemNodeCount(ielem_moving));
        newnodes_moving.resize(xelems.getElemNodeCount(ielem_moving));
        xelems.getElemNodes(inodes_moving, ielem_moving);

        for (idx_t imoving = 0; imoving < inodes_moving.size(); imoving++)
          if (inodes_moving[imoving] == inode_base)
          {
            xnodes.getNodeCoords(coords, inodes_moving[imoving]);
            newNode = xnodes.addNode(coords);
            jem::System::info(myName_) << " ...Duplicated node " << inodes_moving[imoving] << " with coords " << coords << " into node " << newNode << "\n";

            newnodes_moving = inodes_moving.clone();
            newnodes_moving[imoving] = newNode;

            xelems.setElemNodes(ielem_moving, newnodes_moving);
            newElem = xelems.addElement(IdxVector({inode_base, newNode}));
            jem::System::info(myName_) << " ...Created new hinge " << newElem << " with nodes " << IdxVector({inode_base, newNode}) << "\n";

            newElems.pushBack(newElem);
          }
      }
  }

  jem::System::info(myName_) << "\n";
  return jive::fem::newElementGroup(newElems.toArray(), xelems);
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Model> hingeModel::makeNew

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)

{
  return newInstance<hingeModel>(name, conf, props, globdat);
}

//-----------------------------------------------------------------------
//   declare hingeModel
//-----------------------------------------------------------------------

// Registers the hingeModel class with the ModelFactory.

void hingeModel::declare()
{
  using jive::model::ModelFactory;

  ModelFactory::declare(TYPE_NAME, &makeNew);
}
