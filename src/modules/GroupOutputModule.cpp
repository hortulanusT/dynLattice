#include "GroupOutputModule.h"

const char *GroupOutputModule::TYPE_NAME = "GroupOutput";

GroupOutputModule::GroupOutputModule(const String &name) : Module(name)
{
  // per default extract composite values for all elements
  elemGroups_.resize(1);
  elemGroups_[0] = "all";
}

Module::Status GroupOutputModule::init(const Properties &conf,
                                       const Properties &props,
                                       const Properties &globdat)
{
  // retrieve and create subpropertysets
  Properties myProps = props.getProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  // get the node groups
  myProps.find(nodeGroups_, "nodeGroups");
  myConf.set("nodeGroups", nodeGroups_);

  // get the element groups
  myProps.find(elemGroups_, "elementGroups");
  myConf.set("elementGroups", elemGroups_);

  // get the dofs for the node groups
  myProps.find(nodeDofNames_, "dofs");
  myConf.set("dofs", nodeDofNames_);

  // get the dofs for the element group
  if (!myProps.find(elemDofNames_, "dimensions"))
    elemDofNames_.ref(nodeDofNames_);
  myConf.set("dimensions", elemDofNames_);

  // translate the DOF Names to IDs
  elemDofs_.resize(elemDofNames_.size());
  nodeDofs_.resize(nodeDofNames_.size());
  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());

  for (idx_t idof = 0; idof < elemDofNames_.size(); idof++)
    elemDofs_[idof] = dofs->getTypeIndex(elemDofNames_[idof]);
  for (idx_t idof = 0; idof < nodeDofNames_.size(); idof++)
    nodeDofs_[idof] = dofs->getTypeIndex(nodeDofNames_[idof]);

  return elemDofs_.size() + nodeDofs_.size() > 0 ? run(globdat) : Status::DONE;
}

Module::Status GroupOutputModule::run(const Properties &globdat)
{
  Properties myVars = Globdat::getVariables(globdat);
  Properties currentVars, loadVars, respVars, dispVars, veloVars, acceVars, extentVars;
  NodeSet nodes = NodeSet::get(globdat, getContext());
  ElementSet elems = ElementSet::get(globdat, getContext());

  IdxVector nodeIndices;
  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());
  IdxVector dofIndices;

  Vector allLoad(dofs->dofCount());
  Vector allResp(dofs->dofCount());
  Vector allDisp(dofs->dofCount());
  Vector allVelo(dofs->dofCount());
  Vector allAcce(dofs->dofCount());
  Matrix coords(nodes.size(), nodes.rank());

  StateVector::get(allDisp, dofs, globdat);
  bool doVelo = StateVector::find(allVelo, jive::model::STATE1, dofs, globdat);
  bool doAcce = StateVector::find(allAcce, jive::model::STATE2, dofs, globdat);

  Ref<Model> model = Model::get(globdat, getContext());
  Properties params("actionParams");
  allLoad = 0.;
  params.set(ActionParams::EXT_VECTOR, allLoad);
  model->takeAction(Actions::GET_EXT_VECTOR, params, globdat);
  params.erase(ActionParams::EXT_VECTOR);

  allResp = 0.;
  params.set(ActionParams::INT_VECTOR, allResp);
  model->takeAction(Actions::GET_INT_VECTOR, params, globdat);
  params.erase(ActionParams::INT_VECTOR);

  Vector load, disp, velo, acce, resp;

  // iterate through the node groups
  for (idx_t iNodeGroup = 0; iNodeGroup < nodeGroups_.size(); iNodeGroup++)
  {
    // get the nodes associated with this node group
    currentVars = myVars.makeProps(nodeGroups_[iNodeGroup]);
    loadVars = currentVars.makeProps("load");
    respVars = currentVars.makeProps("resp");
    dispVars = currentVars.makeProps("disp");
    if (doVelo)
      veloVars = currentVars.makeProps("velo");
    if (doAcce)
      acceVars = currentVars.makeProps("acce");
    NodeGroup nodeGroup = NodeGroup::get(nodeGroups_[iNodeGroup], nodes, globdat, getContext());
    nodeIndices.resize(nodeGroup.size());
    dofIndices.resize(nodeGroup.size());
    load.resize(nodeGroup.size());
    resp.resize(nodeGroup.size());
    disp.resize(nodeGroup.size());
    velo.resize(nodeGroup.size());
    acce.resize(nodeGroup.size());
    nodeIndices = nodeGroup.getIndices();

    // iterate through all the dofs to report
    for (idx_t iDof = 0; iDof < nodeDofs_.size(); iDof++)
    {
      // get the indices of the dofs at those nodes
      dofs->getDofIndices(dofIndices, nodeIndices, nodeDofs_[iDof]);

      // get the displacements and loads for those dofs
      disp = allDisp[dofIndices];
      if (doVelo)
        velo = allVelo[dofIndices];
      if (doAcce)
        acce = allAcce[dofIndices];

      load = allLoad[dofIndices];
      resp = allResp[dofIndices];

      // report it back (sum for load/resp and avg for disp)
      loadVars.set(nodeDofNames_[iDof], sum(load));
      respVars.set(nodeDofNames_[iDof], sum(resp));
      dispVars.set(nodeDofNames_[iDof], sum(disp) / disp.size());
      if (doVelo)
        veloVars.set(nodeDofNames_[iDof], sum(velo) / velo.size());
      if (doAcce)
        acceVars.set(nodeDofNames_[iDof], sum(acce) / acce.size());
    }
  }

  // iterate through the element groups
  for (idx_t iElemGroup = 0; iElemGroup < elemGroups_.size(); iElemGroup++)
  {
    // get the nodes associated with this element group
    currentVars = myVars.makeProps(elemGroups_[iElemGroup]);
    extentVars = currentVars.makeProps("extent"); // TODO move to PBC module and use the group positions!
    ElementGroup elemGroup = ElementGroup::get(elemGroups_[iElemGroup], elems, globdat, getContext());
    nodeIndices.resize(elemGroup.getNodeIndices().size());
    dofIndices.resize(nodeIndices.size());
    load.resize(nodeIndices.size());
    disp.resize(nodeIndices.size());
    nodeIndices = elemGroup.getNodeIndices();

    nodes.getCoords(coords.transpose());

    // iterate through the dimensions
    for (idx_t iDof = 0; iDof < elemDofs_.size(); iDof++)
    {
      // get the indices of the dofs at those nodes
      dofs->getDofIndices(dofIndices, nodeIndices, elemDofs_[iDof]);

      // get the displacements and loads for those dofs
      disp = allDisp[dofIndices];
      load = allLoad[dofIndices];

      // report back the extent
      extentVars.set(elemDofNames_[iDof], max(coords[iDof]) - min(coords[iDof]));
    }
  }
  return Status::OK;
}

Ref<Module> GroupOutputModule::makeNew(const String &name,
                                       const Properties &conf,
                                       const Properties &props,
                                       const Properties &globdat)
{
  return newInstance<GroupOutputModule>(name);
}

void GroupOutputModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &makeNew);
}