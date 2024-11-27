#include "models/SpringMassModel.h"
#include "utils/testing.h"
#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(SpringMassModel);

//=======================================================================
//   class SpringMassModel
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------
const char *SpringMassModel::TYPE_NAME = "SpringMass";
const char *SpringMassModel::BOUNDARY_PROP = "boundary";
const char *SpringMassModel::SPRING_NAMES = "springs";
const char *SpringMassModel::EXTENT_VECTOR_PROP = "extentVector";
const char *SpringMassModel::N_ELEM_PROP = "nElem";
const char *SpringMassModel::P_ELEM_PROP = "pElem";

//-----------------------------------------------------------------------
//   constructors & destructor
//-----------------------------------------------------------------------
SpringMassModel::SpringMassModel

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat) : Super(name)
{
  ArrayBuffer<Ref<Model>> childBuffer;
  idx_t nElem, pElem;

  // Get the Properties associated with this model
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  XNodeSet allNodes = XNodeSet::get(globdat, getContext());
  XElementSet allElems = XElementSet::get(globdat, getContext());

  const idx_t rank = allNodes.rank();
  Vector position(rank);
  Vector step(rank);
  position = 0.;
  step = 0.;

  // get the boundary
  String boundary_name;
  myProps.get(boundary_name, BOUNDARY_PROP);
  boundary_ = NodeGroup::get(boundary_name, allNodes, globdat, getContext());
  myConf.set(BOUNDARY_PROP, boundary_name);

  Matrix boundaryCoords(rank, boundary_.size());

  boundary_.getCoords(boundaryCoords);

  for (idx_t i = 0; i < rank; i++)
  {
    position[i] = jem::sum(boundaryCoords(i, ALL)) / boundary_.size();
  }

  // get the spring properties
  StringVector springNames;
  myProps.get(springNames, SPRING_NAMES);
  myConf.set(SPRING_NAMES, springNames);

  edgeNodes_.resize(springNames.size(), 2);

  // construct the spring models
  for (idx_t i = 0; i < springNames.size(); i++)
  {
    Properties springProps = myProps.getProps(springNames[i]);
    Properties springConf = myConf.makeProps(springNames[i]);
    springProps.get(step, EXTENT_VECTOR_PROP);
    springProps.get(nElem, N_ELEM_PROP);
    springProps.get(pElem, P_ELEM_PROP);

    IdxVector iElems(nElem);
    IdxVector iNodes(pElem + 1);

    step /= double(nElem * pElem);

    // construct the nodes
    iNodes[pElem] = allNodes.addNode(position);
    for (idx_t iElem = 0; iElem < nElem; iElem++)
    {
      iNodes[0] = iNodes[pElem];
      for (idx_t iNode = 1; iNode < pElem + 1; iNode++)
      {
        position += step;
        iNodes[iNode] = allNodes.addNode(position);
        jem::System::debug(myName_) << " ...Created Node " << iNodes[iNode] << " at " << position << "\n";
      }

      iElems[iElem] = allElems.addElement(iNodes);
      jem::System::debug(myName_) << " ...Created Element " << iElems[iElem] << " with nodes " << iNodes << "\n";

      if (iElem == 0)
        edgeNodes_(i, 0) = iNodes[0];
      if (iElem == nElem - 1)
        edgeNodes_(i, 1) = iNodes[pElem];
    }

    ElementGroup elemGroup = jive::fem::newElementGroup(iElems, allElems);
    elemGroup.store(springNames[i], globdat);
    NodeGroup nodeGroup = jive::fem::newNodeGroup(iNodes[jem::SliceFrom(pElem)], allNodes);
    nodeGroup.store(springNames[i] + "_bot", globdat);

    // create the spring model
    jem::System::debug(myName_) << " ...Creating Model for Spring '" << springNames[i] << "'\n";
    childBuffer.pushBack(ModelFactory::newInstance(springNames[i], myConf, myProps, globdat));

    springConf.set(EXTENT_VECTOR_PROP, Vector(step * double(nElem * pElem)));
    springConf.set(N_ELEM_PROP, nElem);
    springConf.set(P_ELEM_PROP, pElem);
  }

  children_.resize(childBuffer.size());
  children_ = childBuffer.toArray();
}

SpringMassModel::~SpringMassModel() {}

//-----------------------------------------------------------------------
//  takeAction
//-----------------------------------------------------------------------
bool SpringMassModel::takeAction

    (const String &action,
     const Properties &params,
     const Properties &globdat)
{
  bool ret_val = false;

  if (action == Actions::INIT || action == Actions::GET_CONSTRAINTS)
  {
    ret_val |= applyConstraints_(globdat);
  }

  for (Ref<Model> &child : children_)
  {
    ret_val |= child->takeAction(action, params, globdat);
  }

  return ret_val;
}

//-----------------------------------------------------------------------
//   applyConstraints_
//-----------------------------------------------------------------------
bool SpringMassModel::applyConstraints_

    (const Properties &globdat) const
{
  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());
  Ref<Constraints> cons = Constraints::get(dofs, globdat);

  IdxVector iBoundNodes = boundary_.getIndices();

  // fix the boundary to the first node
  idx_t mDof;
  IdxVector bDofs(boundary_.size());
  for (idx_t iDofType = 0; iDofType < dofs->typeCount(); iDofType++)
  {
    dofs->getDofIndices(bDofs, iBoundNodes, iDofType);
    mDof = dofs->getDofIndex(edgeNodes_(0, 0), iDofType);
    for (idx_t bDof : bDofs)
    {
      cons->addConstraint(bDof, mDof, 1.0);
    }
  }

  // fix the Springs to each other
  idx_t sDof;
  for (idx_t iSpring = 1; iSpring < edgeNodes_.size(0); iSpring++)
  {
    for (idx_t iDofType = 0; iDofType < dofs->typeCount(); iDofType++)
    {
      sDof = dofs->getDofIndex(edgeNodes_(iSpring - 1, 1), iDofType);
      mDof = dofs->getDofIndex(edgeNodes_(iSpring, 0), iDofType);
      cons->addConstraint(sDof, mDof, 1.0);
    }
  }

  // fix the last spring to the ground
  for (idx_t iDofType = 0; iDofType < dofs->typeCount(); iDofType++)
  {
    sDof = dofs->getDofIndex(edgeNodes_(edgeNodes_.size(0) - 1, 1), iDofType);
    cons->addConstraint(sDof, 0.);
  }

  cons->compress();

  return true;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------
Ref<Model> SpringMassModel::makeNew

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)
{
  return jem::newInstance<Self>(name, conf, props, globdat);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------
void SpringMassModel::declare()
{
  ModelFactory::declare(TYPE_NAME, &makeNew);
  ModelFactory::declare(CLASS_NAME, &makeNew);
}
