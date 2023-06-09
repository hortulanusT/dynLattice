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
const char *hingeModel::YIELD_PROP = "yieldCond";

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

  StringVector namesT(specialCosseratRodModel::TRANS_DOF_COUNT);
  StringVector namesR(specialCosseratRodModel::ROT_DOF_COUNT);
  conf.get(namesT, specialCosseratRodModel::TRANS_DOF_NAMES);
  conf.get(namesR, specialCosseratRodModel::ROT_DOF_NAMES);
  String args = jem::util::StringUtils::join(namesT, ", ") + ", " + jem::util::StringUtils::join(namesR, ", ");

  // get the limit loads
  FuncUtils::configFunc(yieldCond_, args, YIELD_PROP, myProps, globdat);
  FuncUtils::getConfig(myConf, yieldCond_, YIELD_PROP);

  // get the mother material
  myProps.get(material_, "material");
  myConf.set("material", material_);
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

    TEST_CONTEXT(accepted);
    TEST_CONTEXT(plasticDisp_);

    return true;
  }

  if (action == Actions::COMMIT)
  {
    intForcesOld_ = intForces_;
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
  jnames_.resize(dofs_->typeCount());
  jnames_ = dofs_->getTypeNames();
  for (idx_t i = 0; i < jtypes_.size(); i++)
    jtypes_[i] = dofs_->getTypeIndex(jnames_[i]);

  intForces_.resize(jtypes_.size(), egroup_.size());
  intForcesOld_.resize(jtypes_.size(), egroup_.size());
  plasticDisp_.resize(jtypes_.size(), egroup_.size());

  intForces_ = 0.;
  intForcesOld_ = 0.;
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
      constraints_->addConstraint(dofsB[idof], plasticDisp_[ielem][idof], dofsA[idof], 1.);
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
  bool checked = true;
  double f_old, f_trial;
  double deltaFlow;
  Vector deriv(jtypes_.size());
  Vector critForces(jtypes_.size());

  for (idx_t ielem = 0; ielem < egroup_.size(); ielem++)
  {
    f_trial = yieldCond_->getValue(intForces_[ielem].addr());
    TEST_CONTEXT(f_trial)
    if (f_trial > 0 && !jem::isTiny(f_trial))
    {
      f_old = yieldCond_->getValue(intForcesOld_[ielem].addr());
      checked = false;
      critForces = intForcesOld_[ielem] - (intForces_[ielem] - intForcesOld_[ielem]) * f_old / (f_trial - f_old);

      for (idx_t idof = 0; idof < jtypes_.size(); idof++)
        deriv[idof] = yieldCond_->getDeriv(idof, critForces.addr());

      deltaFlow = dotProduct(deriv, intForces_[ielem] - critForces) / dotProduct(deriv, matmul(material_->getMaterialStiff(), deriv));

      plasticDisp_[ielem] += ell_[ielem] * deltaFlow * deriv;
    }
  }

  return checked;
}

//-----------------------------------------------------------------------
//   createHinges_
//-----------------------------------------------------------------------

ElementGroup hingeModel::createHinges_(const String &elementName, const Properties &globdat)
{
  IdxVector inodes_base, inodes_moving, newnodes_moving;
  Matrix coords_base, coords_moving;
  double base_l, moving_l;
  idx_t newNode, newElem;
  IdxBuffer newElems;
  jem::util::ArrayBuffer<double> elemLengths;

  Assignable<XNodeSet> xnodes = XNodeSet::get(globdat, getContext());
  Assignable<XElementSet> xelems = XElementSet::get(globdat, getContext());
  Assignable<ElementGroup> motherElems = ElementGroup::get(jem::util::StringUtils::split(elementName, '.')[0], xelems, globdat, getContext());

  jem::System::info(myName_) << " ...Creating " << elementName << "\n";

  for (idx_t ielem_base : motherElems.getIndices())
  {
    inodes_base.resize(xelems.getElemNodeCount(ielem_base));
    coords_base.resize(xnodes.rank(), inodes_base.size());

    xelems.getElemNodes(inodes_base, ielem_base);
    xnodes.getSomeCoords(coords_base, inodes_base);
    base_l = 0;
    for (idx_t i = 1; i < inodes_base.size(); i++)
      base_l += norm2(coords_base[i] - coords_base[i - 1]);

    for (idx_t inode_base = 0; inode_base < inodes_base.size(); inode_base += inodes_base.size() - 1)
      for (idx_t ielem_moving : motherElems.getIndices())
      {
        if (ielem_moving == ielem_base)
          continue;

        inodes_moving.resize(xelems.getElemNodeCount(ielem_moving));
        newnodes_moving.resize(xelems.getElemNodeCount(ielem_moving));
        coords_moving.resize(xnodes.rank(), inodes_moving.size());
        xelems.getElemNodes(inodes_moving, ielem_moving);
        xnodes.getSomeCoords(coords_moving, inodes_moving);
        moving_l = 0;
        for (idx_t i = 1; i < inodes_moving.size(); i++)
          moving_l += norm2(coords_moving[i] - coords_moving[i - 1]);

        for (idx_t imoving = 0; imoving < inodes_moving.size(); imoving += inodes_moving.size() - 1)
          if (inodes_moving[imoving] == inodes_base[inode_base])
          {
            newNode = xnodes.addNode(coords_base[inode_base]);
            jem::System::info(myName_) << " ...Duplicated node " << inodes_moving[imoving] << " with coords " << coords_base[inode_base] << " into node " << newNode << "\n";

            newnodes_moving = inodes_moving.clone();
            newnodes_moving[imoving] = newNode;

            xelems.setElemNodes(ielem_moving, newnodes_moving);
            newElem = xelems.addElement(IdxVector({inodes_base[inode_base], newNode}));
            jem::System::info(myName_) << " ...Created new hinge " << newElem << " with nodes " << IdxVector({inodes_base[inode_base], newNode}) << "\n";

            newElems.pushBack(newElem);
            elemLengths.pushBack(base_l / 4. + moving_l / 4.);
          }
      }
  }

  jem::System::info(myName_) << "\n";
  ell_.resize(elemLengths.size());
  ell_ = elemLengths.toArray();
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
