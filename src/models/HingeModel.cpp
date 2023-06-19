/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *
 * This class implements hinges for geometrically nonlinear rods
 *
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: July 21
 *
 */

#include "HingeModel.h"
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

  myProps.get(elName_, "elements");

  // Get the elements and nodes from the global database.
  elems_ = ElementSet::get(globdat, getContext()); // all the elements
  nodes_ = elems_.getNodes();                      // all the nodes

  egroup_ = ElementGroup::find(elName_, elems_, globdat); // only the desired group
  if (!egroup_)
    egroup_ = createHinges_(elName_, globdat);

  myConf.set("elements", elName_);

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

  // get iteration settings
  maxIter_ = 3;
  iiter_ = 0;
  myProps.find(maxIter_, jive::implict::PropNames::MAX_ITER);
  myProps.set(jive::implict::PropNames::MAX_ITER, maxIter_);

  prec_ = jive::solver::Solver::PRECISION;
  myProps.find(prec_, jive::implict::PropNames::PRECISION);
  myConf.set(jive::implict::PropNames::PRECISION, prec_);
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
    // !!! THIS NEEDS TO BE DONE LAST IN THE MULTI-MODEL !!!
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

    if (accepted)
      jem::System::info(myName_) << " ...Plastic patterns accepted (it " << iiter_ << " for " << elName_ << ")\n";
    else
      jem::System::info(myName_) << " ...Plastic patterns rejected (it " << iiter_ << " for " << elName_ << ")\n";

    iiter_++;
    return true;
  }

  if (action == Actions::COMMIT)
  {
    intForcesOld_ = intForces_;
    iiter_ = 0;
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------
//   init_
//-----------------------------------------------------------------------

void hingeModel::init_(const Properties &globdat)
{
  dofs_ = DofSpace::get(globdat, getContext()); // all the dofs
  cons_ = Constraints::get(dofs_, globdat);     // all the constraints

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
    {
      if (ielem == egroup_.size() - 1)
      {
        cons_->addConstraint(dofsA[idof], -1. * plasticDisp_[ielem][idof], dofsB[idof], 1.);
      }
      else
        cons_->addConstraint(dofsB[idof], plasticDisp_[ielem][idof], dofsA[idof], 1.);
    }
  }

  cons_->compress();
}

//-----------------------------------------------------------------------
//   updForces_
//-----------------------------------------------------------------------

void hingeModel::updForces_(const Vector &fint)
{
  IdxVector inodes(2);
  idx_t jdofA, jdofB;
  const idx_t hingeCount = egroup_.size();

  for (idx_t ielem = 0; ielem < hingeCount; ielem++)
  {
    elems_.getElemNodes(inodes, egroup_.getIndex(ielem));
    for (idx_t idof = 0; idof < jtypes_.size(); idof++)
    {
      jdofA = dofs_->getDofIndex(inodes[0], idof);
      jdofB = dofs_->getDofIndex(inodes[1], idof);

      if (ielem == 0)
        intForces_[ielem][idof] = -1 * fint[jdofB];
      else
        intForces_[ielem][idof] = fint[jdofA];
    }
  }
}

//-----------------------------------------------------------------------
//   evalPlastic_
//-----------------------------------------------------------------------

bool hingeModel::evalPlastic_(const Vector &disp) // todo: transform to matrial coordinates
{
  bool checked = true;
  double f_old, f_trial;
  double deltaFlow;
  Vector deriv(jtypes_.size());
  Vector critForces(jtypes_.size());
  double change = 0;

  for (idx_t ielem = 0; ielem < egroup_.size(); ielem++)
  {
    f_trial = yieldCond_->getValue(intForces_[ielem].addr());
    if (f_trial > 0 && !jem::isTiny(f_trial))
    {
      checked = false;

      f_old = yieldCond_->getValue(intForcesOld_[ielem].addr());
      critForces = intForcesOld_[ielem] - (intForces_[ielem] - intForcesOld_[ielem]) * f_old / (f_trial - f_old);

      for (idx_t idof = 0; idof < jtypes_.size(); idof++)
        deriv[idof] = yieldCond_->getDeriv(idof, critForces.addr());

      TEST_CONTEXT(deriv)

      deltaFlow = dotProduct(deriv, intForces_[ielem] - critForces) / dotProduct(deriv, matmul(material_->getMaterialStiff(), deriv));

      plasticDisp_[ielem] += ell_[ielem] * deltaFlow * deriv;
      change += norm2(ell_[ielem] * deltaFlow * deriv);
    }
  }

  return checked || iiter_ > maxIter_ || change < prec_;
}

//-----------------------------------------------------------------------
//   createHinges_
//-----------------------------------------------------------------------

ElementGroup hingeModel::createHinges_(const String &elementName, const Properties &globdat)
{
  IdxVector inodesA, inodesB, inodesNew;
  Matrix coordsA, coordsB;
  double lA, lB;
  idx_t elemB, newNode, newHinge;
  IdxBuffer newElems;
  jem::util::ArrayBuffer<double> elemLengths;

  Assignable<XNodeSet> xnodes = XNodeSet::get(globdat, getContext());
  Assignable<XElementSet> xelems = XElementSet::get(globdat, getContext());
  Assignable<ElementGroup> motherElems = ElementGroup::get(jem::util::StringUtils::split(elementName, '.')[0], xelems, globdat, getContext());

  const idx_t nodeCount = xelems.getElemNodeCount(motherElems.getIndex(0));
  const idx_t rank = xnodes.rank();

  inodesA.resize(nodeCount);
  inodesB.resize(nodeCount);
  inodesNew.resize(nodeCount);
  coordsA.resize(rank, nodeCount);
  coordsB.resize(rank, nodeCount);

  inodesA = 0.;
  inodesB = 0.;
  inodesNew = 0.;
  coordsA = 0.;
  coordsB = 0.;
  lA = 0.;
  lB = 0.;
  elemB = motherElems.getIndex(0);

  jem::System::info(myName_) << " ...Creating " << elementName << "\n";

  for (idx_t ielem = 0; ielem < motherElems.size(); ielem++)
  {
    inodesA = inodesB;
    coordsA = coordsB;
    lA = lB;

    elemB = motherElems.getIndex(ielem);

    xelems.getElemNodes(inodesB, elemB);
    xnodes.getSomeCoords(coordsB, inodesB);
    lB = norm2(coordsB[nodeCount - 1] - coordsB[0]);

    newNode = xnodes.addNode(coordsB[0]);
    jem::System::info(myName_) << " ...Duplicated node " << inodesB[0] << " with coords " << coordsB[0] << " into node " << newNode << "\n";

    inodesNew = inodesB;
    inodesNew[0] = newNode;
    xelems.setElemNodes(elemB, inodesNew);
    jem::System::info(myName_) << " ...Changed nodes of element " << elemB << " to " << inodesNew << "\n";

    newHinge = xelems.addElement(IdxVector({inodesB[0], newNode}));
    jem::System::info(myName_) << " ...Created new hinge element " << newHinge << " with nodes " << IdxVector({inodesB[0], newNode}) << "\n";

    newElems.pushBack(newHinge);
    elemLengths.pushBack(lB / 2. + lA / 2.);
  }

  // create additional end at the end of the rod
  inodesA = 0.;
  coordsA = 0.;
  lA = 0.;

  newNode = xnodes.addNode(coordsB[nodeCount - 1]);
  jem::System::info(myName_) << " ...Duplicated node " << inodesB[nodeCount - 1] << " with coords " << coordsB[nodeCount - 1] << " into node " << newNode << "\n";

  inodesNew[nodeCount - 1] = newNode;
  xelems.setElemNodes(elemB, inodesNew);
  jem::System::info(myName_) << " ...Changed nodes of element " << elemB << " to " << inodesNew << "\n";

  newHinge = xelems.addElement(IdxVector({newNode, inodesB[nodeCount - 1]}));
  jem::System::info(myName_) << " ...Created new hinge element " << newHinge << " with nodes " << IdxVector({newNode, inodesB[nodeCount - 1]}) << "\n";

  newElems.pushBack(newHinge);
  elemLengths.pushBack(lB / 2. + lA / 2.);

  jem::System::info(myName_)
      << "\n";
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
