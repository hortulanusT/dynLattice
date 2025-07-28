/**
 * @file LatticeModel.cpp
 * @author Til Gärtner
 * @brief Implementation of lattice model for managing collections of rod elements
 */

#include "models/LatticeModel.h"
#include "utils/testing.h"
#include <jem/base/ClassTemplate.h>
#include <jive/app/Names.h>

//=======================================================================
//   class LatticeModel
//=======================================================================

JEM_DEFINE_CLASS(LatticeModel);

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------
const char *LatticeModel::TYPE_NAME = "Lattice";
const char *LatticeModel::CHILD_PROPS = "child";
const char *LatticeModel::ROD_CONTACT_PROP = "contact";
const char *LatticeModel::JOINT_CONTACT_PROP = "jointContact";
const char *LatticeModel::ROD_LIST_PROP = "rodList";
const char *LatticeModel::NAME_PREFIX = "prefix";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------
LatticeModel::LatticeModel

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat) : Model(name)
{
  String prefix;
  Properties childProps, childConf;
  Properties dummyProps, dummyConf;
  Assignable<ElementSet> elems;
  ArrayBuffer<Ref<Model>> childBuffer;
  idx_t ichild = 0;
  String childName;

  // Get the Properties associated with this model
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  // Get the naming pattern of the children
  myProps.get(prefix, NAME_PREFIX);
  myConf.set(NAME_PREFIX, prefix);

  // Get the children Properties
  myProps.get(childProps, CHILD_PROPS);

  // Find the children
  elems = ElementSet::get(globdat, getContext());

  jem::System::info(myName_) << " ...Creating Models for ElementGroups\n";
  while (true)
  {
    childName = prefix + String(++ichild);
    if (!ElementGroup::find(childName, elems, globdat))
      break;

    dummyProps.clear();
    dummyConf.clear();

    jem::System::debug(myName_) << " ...Creating Model for ElementGroup '" << childName << "'\n";

    dummyProps.set(childName, childProps);
    childBuffer.pushBack(ModelFactory::newInstance(childName, dummyConf, dummyProps, globdat));
  }
  childName = prefix + String(--ichild);
  jem::System::info(myName_) << " ..." << ichild << " Models created\n";
  children_.resize(childBuffer.size());
  children_ = childBuffer.toArray();

  // set the children configuration
  dummyConf.get(childConf, childName);
  myConf.set(CHILD_PROPS, childConf);

  JEM_PRECHECK2(children_.size() > 0, jem::makeCString(String::format("No childrens with prefix '%s' found!", prefix)));

  // Get the rod contact model
  if (myProps.contains(ROD_CONTACT_PROP))
  {
    StringVector rodList(ichild);
    for (idx_t iRod = 0; iRod < ichild; iRod++)
    {
      rodList[iRod] = prefix + String(iRod + 1);
    }
    myProps.getProps(ROD_CONTACT_PROP).set(ROD_LIST_PROP, rodList);
    contact_ = ModelFactory::newInstance(ROD_CONTACT_PROP, myConf, myProps, globdat);
  }

  // Get the joint contact model
  if (myProps.contains(JOINT_CONTACT_PROP))
  {
    jointContact_ = ModelFactory::newInstance(JOINT_CONTACT_PROP, myConf, myProps, globdat);
  }
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------
bool LatticeModel::takeAction

    (const String &action,
     const Properties &params,
     const Properties &globdat)
{
  bool actionTaken = false;
  for (Ref<Model> child : children_)
    actionTaken = child->takeAction(action, params, globdat) || actionTaken;
  if (contact_)
    actionTaken = contact_->takeAction(action, params, globdat) || actionTaken;
  if (jointContact_)
    actionTaken = jointContact_->takeAction(action, params, globdat) || actionTaken;

  if (action == Actions::GET_MATRIX2)
  {
    Ref<MatrixBuilder> mbld;

    params.get(mbld, ActionParams::MATRIX2);

    M_ = mbld->getMatrix();
  }

  if (action == Actions::ADVANCE || action == Actions::INIT)
  {
    Properties vars = Globdat::getVariables(globdat);
    vars.set("potentialEnergy", 0.);
    vars.set("dissipatedEnergy", 0.);
    vars.set("kineticEnergy", 0.);
    vars.set("mass", 0.);
  }

  if (action == Actions::GET_TABLE)
  {
    Assignable<jive::fem::NodeSet> allNodes = jive::fem::NodeSet::get(globdat, getContext());
    Ref<XTable> table;
    Vector weights;
    String name;
    // Get the action-specific parameters.
    params.get(table, ActionParams::TABLE);
    params.get(weights, ActionParams::TABLE_WEIGHTS);
    params.get(name, ActionParams::TABLE_NAME);

    if (table->getRowItems() == allNodes.getData())
    {
      if (M_ == nullptr)
      {
        Ref<Model> supermodel = Model::get(globdat, getContext());
        Properties newMatrixParams;
        supermodel->takeAction(Actions::NEW_MATRIX2, newMatrixParams, globdat);
        supermodel->takeAction(Actions::UPD_MATRIX2, newMatrixParams, globdat);
      }

      if (name == "kineticEnergy")
        calc_kin_Energy_(*table, weights, globdat);
      if (name == "mass")
        calc_mass_(*table, weights, globdat);
    }
  }

  if (action == Actions::COMMIT)
  {
    if (M_ == nullptr)
    {
      Ref<Model> supermodel = Model::get(globdat, getContext());
      Properties newMatrixParams;
      supermodel->takeAction(Actions::NEW_MATRIX2, newMatrixParams, globdat);
      supermodel->takeAction(Actions::UPD_MATRIX2, newMatrixParams, globdat);
    }

    Properties vars = Globdat::getVariables(globdat);
    double E_kin = 0.0;
    double m = 0.0;

    vars.find(E_kin, "kineticEnergy");
    vars.find(m, "mass");

    E_kin += calc_kin_Energy_(globdat);
    m += calc_mass_(globdat);

    vars.set("kineticEnergy", E_kin);
    vars.set("mass", m);
  }

  return actionTaken;
}

void LatticeModel::calc_kin_Energy_(XTable &energy_table, const Vector &table_weights, const Properties &globdat) const
{
  const idx_t jCol = energy_table.addColumn("kineticEnergy");
  Assignable<jive::fem::NodeSet> allNodes = jive::fem::NodeSet::get(globdat, getContext());
  Ref<jive::util::DofSpace> dofs = jive::util::DofSpace::get(globdat, getContext());
  Vector velo, temp;
  IdxVector jtypes(dofs->typeCount());
  IdxVector idofs(dofs->typeCount());

  if (StateVector::find(velo, jive::model::STATE1, DofSpace::get(globdat, getContext()), globdat) && M_)
  {
    temp.resize(velo.size());
    M_->matmul(temp, velo);

    for (idx_t idof = 0; idof < dofs->typeCount(); idof++)
    {
      jtypes[idof] = dofs->getTypeIndex(dofs->getTypeName(idof));
    }

    for (idx_t iNode = 0; iNode < allNodes.size(); iNode++)
    {
      dofs->getDofsForItem(idofs, jtypes, iNode);
      energy_table.addValue(iNode, jCol, 0.5 * dotProduct(velo[idofs], temp[idofs]));
      table_weights[iNode] = 1.0;
    }
  }
}

double LatticeModel::calc_kin_Energy_(const Properties &globdat) const
{
  Assignable<jive::fem::NodeSet> allNodes = jive::fem::NodeSet::get(globdat, getContext());
  Ref<jive::util::DofSpace> dofs = jive::util::DofSpace::get(globdat, getContext());
  Vector velo, temp;
  IdxVector jtypes(dofs->typeCount());
  IdxVector idofs(dofs->typeCount());
  double E_kin = 0.0;

  if (StateVector::find(velo, jive::model::STATE1, DofSpace::get(globdat, getContext()), globdat) && M_)
  {
    temp.resize(velo.size());
    M_->matmul(temp, velo);

    for (idx_t idof = 0; idof < dofs->typeCount(); idof++)
    {
      jtypes[idof] = dofs->getTypeIndex(dofs->getTypeName(idof));
    }

    for (idx_t iNode = 0; iNode < allNodes.size(); iNode++)
    {
      dofs->getDofsForItem(idofs, jtypes, iNode);
      E_kin += 0.5 * dotProduct(velo[idofs], temp[idofs]);
    }
  }

  return E_kin;
}

void LatticeModel::calc_mass_(XTable &mass_table, const Vector &table_weights, const Properties &globdat) const
{
  const idx_t jCol = mass_table.addColumn("mass");
  Assignable<jive::fem::NodeSet> allNodes = jive::fem::NodeSet::get(globdat, getContext());
  Ref<jive::util::DofSpace> dofs = jive::util::DofSpace::get(globdat, getContext());
  const idx_t jtype = dofs->getTypeIndex(dofs->getTypeName(0));

  if (M_)
  {
    Vector tempA(dofs->dofCount());
    Vector tempB(dofs->dofCount());
    tempA = 0.;
    tempB = 0.;

    IdxVector iNodes(allNodes.size());
    IdxVector idofs(allNodes.size());

    for (idx_t iNode = 0; iNode > allNodes.size(); iNode++)
      iNodes[iNode] = iNode;

    dofs->getDofsForType(idofs, iNodes, jtype);

    tempA[idofs] = 1.;

    M_->matmul(tempB, tempA);

    for (idx_t iNode = 0; iNode < allNodes.size(); iNode++)
    {
      mass_table.addValue(iNode, jCol, tempB[idofs[iNode]]);
      table_weights[iNode] = 1.;
    }
  }
}

double LatticeModel::calc_mass_(const Properties &globdat) const
{
  Assignable<jive::fem::NodeSet> allNodes = jive::fem::NodeSet::get(globdat, getContext());
  Ref<jive::util::DofSpace> dofs = jive::util::DofSpace::get(globdat, getContext());
  const idx_t jtype = dofs->getTypeIndex(dofs->getTypeName(0));
  double m = 0.0;

  if (M_)
  {
    Vector tempA(dofs->dofCount());
    Vector tempB(dofs->dofCount());
    tempA = 0.;
    tempB = 0.;

    IdxVector iNodes(allNodes.size());
    IdxVector idofs(allNodes.size());

    for (idx_t iNode = 0; iNode > allNodes.size(); iNode++)
      iNodes[iNode] = iNode;

    dofs->getDofsForType(idofs, iNodes, jtype);

    tempA[idofs] = 1.;

    M_->matmul(tempB, tempA);

    m += jem::sum(tempB[idofs]);
  }

  return m;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------
Ref<Model> LatticeModel::makeNew

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)
{
  return newInstance<LatticeModel>(name, conf, props, globdat);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------
void LatticeModel::declare()
{
  ModelFactory::declare(TYPE_NAME, &makeNew);
}
