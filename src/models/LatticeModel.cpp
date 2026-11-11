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
const char *LatticeModel::TRANS_DOF_NAMES = "dofNamesTrans";
const char *LatticeModel::ROT_DOF_NAMES = "dofNamesRot";
const char *LatticeModel::MOMENTUM[3] = {"momentum_x", "momentum_y", "momentum_z"};
const char *LatticeModel::ANGULAR_MOMENTUM[3] = {"angularMomentum_x", "angularMomentum_y", "angularMomentum_z"};

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

  // dof names of the rods, for the momenta
  childConf.find(transDofs_, TRANS_DOF_NAMES);
  childConf.find(rotDofs_, ROT_DOF_NAMES);

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
    for (idx_t i = 0; i < 3; i++)
    {
      vars.set(MOMENTUM[i], 0.);
      vars.set(ANGULAR_MOMENTUM[i], 0.);
    }
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
        getKineticEnergy(*table, weights, globdat);
      if (name == "mass")
        getMass(*table, weights, globdat);
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
    Vector velo;

    vars.find(E_kin, "kineticEnergy");
    vars.find(m, "mass");

    // M v once, for the kinetic energy and the momenta
    if (StateVector::find(velo, jive::model::STATE1, DofSpace::get(globdat, getContext()), globdat) && M_)
    {
      Vector massVelo(velo.size());
      M_->matmul(massVelo, velo);
      E_kin += getKineticEnergy_(velo, massVelo, globdat);

      if (transDofs_.size() == 3 && rotDofs_.size() == 3)
      {
        Vector p(3);
        Vector l(3);
        double sum = 0.;
        getMomenta_(p, l, massVelo, globdat);
        for (idx_t i = 0; i < 3; i++)
        {
          sum = 0.;
          vars.find(sum, MOMENTUM[i]);
          vars.set(MOMENTUM[i], sum + p[i]);
          sum = 0.;
          vars.find(sum, ANGULAR_MOMENTUM[i]);
          vars.set(ANGULAR_MOMENTUM[i], sum + l[i]);
        }
      }
    }
    m += getMass(globdat);

    vars.set("kineticEnergy", E_kin);
    vars.set("mass", m);
  }

  return actionTaken;
}

void LatticeModel::getKineticEnergy(XTable &energy_table, const Vector &table_weights, const Properties &globdat) const
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

double LatticeModel::getKineticEnergy(const Properties &globdat) const
{
  Vector velo, temp;
  double E_kin = 0.0;

  if (StateVector::find(velo, jive::model::STATE1, DofSpace::get(globdat, getContext()), globdat) && M_)
  {
    temp.resize(velo.size());
    M_->matmul(temp, velo);
    E_kin = getKineticEnergy_(velo, temp, globdat);
  }

  return E_kin;
}

double LatticeModel::getKineticEnergy_(const Vector &velo, const Vector &massVelo, const Properties &globdat) const
{
  Assignable<jive::fem::NodeSet> allNodes = jive::fem::NodeSet::get(globdat, getContext());
  Ref<jive::util::DofSpace> dofs = jive::util::DofSpace::get(globdat, getContext());
  IdxVector jtypes(dofs->typeCount());
  IdxVector idofs(dofs->typeCount());
  double E_kin = 0.0;

  for (idx_t idof = 0; idof < dofs->typeCount(); idof++)
  {
    jtypes[idof] = dofs->getTypeIndex(dofs->getTypeName(idof));
  }

  for (idx_t iNode = 0; iNode < allNodes.size(); iNode++)
  {
    dofs->getDofsForItem(idofs, jtypes, iNode);
    E_kin += 0.5 * dotProduct(velo[idofs], massVelo[idofs]);
  }

  return E_kin;
}

void LatticeModel::getMomenta_(const Vector &p, const Vector &l, const Vector &massVelo,
                               const Properties &globdat) const
{
  Assignable<jive::fem::NodeSet> allNodes = jive::fem::NodeSet::get(globdat, getContext());
  Ref<jive::util::DofSpace> dofs = jive::util::DofSpace::get(globdat, getContext());
  const idx_t rank = allNodes.rank();
  Vector disp;
  Vector coords(rank);
  Vector x(3);
  Vector mv(3);
  IdxVector itrans(3);
  IdxVector irot(3);

  StateVector::get(disp, dofs, globdat);

  for (idx_t i = 0; i < 3; i++)
  {
    itrans[i] = dofs->getTypeIndex(transDofs_[i]);
    irot[i] = dofs->getTypeIndex(rotDofs_[i]);
  }

  // p = sum (M v)_trans, l = sum x x (M v)_trans + (M v)_rot, x = X + u: exact for the assembled mass, as the
  // consistent translational mass couples the nodes with the shape functions that interpolate x and v
  p = 0.;
  l = 0.;
  for (idx_t iNode = 0; iNode < allNodes.size(); iNode++)
  {
    if (dofs->findDofIndex(iNode, itrans[0]) < 0)
      continue;

    allNodes.getNodeCoords(coords, iNode);
    x = 0.;
    x[jem::SliceTo(rank)] = coords;
    for (idx_t i = 0; i < 3; i++)
    {
      const idx_t idof = dofs->getDofIndex(iNode, itrans[i]);
      x[i] += disp[idof];
      mv[i] = massVelo[idof];
    }

    p += mv;
    l[0] += x[1] * mv[2] - x[2] * mv[1];
    l[1] += x[2] * mv[0] - x[0] * mv[2];
    l[2] += x[0] * mv[1] - x[1] * mv[0];
    for (idx_t i = 0; i < 3; i++)
      l[i] += massVelo[dofs->getDofIndex(iNode, irot[i])];
  }
}

void LatticeModel::getMass(XTable &mass_table, const Vector &table_weights, const Properties &globdat) const
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

double LatticeModel::getMass(const Properties &globdat) const
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
