/**
 * @file GroupOutputModule.cpp
 * @author Til Gärtner
 * @brief Implementation of group-based output data extraction module
 *
 * This module extracts displacement, velocity, acceleration, load, and response
 * data from specified node and element groups and stores them in the global
 * database for post-processing and analysis. For node groups, it computes sums
 * for loads/responses and averages for displacements. Element group processing
 * extracts data from all associated nodes.
 */

#include "GroupOutputModule.h"
#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(GroupOutputModule);

const char *GroupOutputModule::TYPE_NAME = "GroupOutput";

GroupOutputModule::GroupOutputModule(const String &name) : Module(name)
{
  // Default configuration: extract composite values for all elements
  elemGroups_.resize(1);
  elemGroups_[0] = "all";
}

GroupOutputModule::~GroupOutputModule()
{
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

  // Translate DOF names to indices
  elemDofs_.resize(elemDofNames_.size());
  nodeDofs_.resize(nodeDofNames_.size());
  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());

  for (idx_t idof = 0; idof < elemDofNames_.size(); idof++)
    elemDofs_[idof] = dofs->getTypeIndex(elemDofNames_[idof]);
  for (idx_t idof = 0; idof < nodeDofNames_.size(); idof++)
    nodeDofs_[idof] = dofs->getTypeIndex(nodeDofNames_[idof]);

  // Perform initial data extraction if any DOFs are configured
  return elemDofs_.size() + nodeDofs_.size() > 0 ? run(globdat)
                                                 : Status::DONE;
}

Module::Status GroupOutputModule::run(const Properties &globdat)
{
  // Initialize data structures and retrieve global state
  Properties myVars = Globdat::getVariables(globdat);
  Properties currentVars, loadVars, respVars, dispVars, veloVars,
      acceVars;
  NodeSet nodes = NodeSet::get(globdat, getContext());
  ElementSet elems = ElementSet::get(globdat, getContext());

  IdxVector nodeIndices;
  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());
  IdxVector dofIndices;

  // Retrieve all state vectors
  Vector allLoad(dofs->dofCount());
  Vector allResp(dofs->dofCount());
  Vector allDisp(dofs->dofCount());
  Vector allVelo(dofs->dofCount());
  Vector allAcce(dofs->dofCount());

  StateVector::get(allDisp, dofs, globdat);
  bool doVelo =
      StateVector::find(allVelo, jive::model::STATE1, dofs, globdat);
  bool doAcce =
      StateVector::find(allAcce, jive::model::STATE2, dofs, globdat);

  // Retrieve load and response vectors from model
  Ref<Model> model = Model::get(globdat, getContext());
  Properties params("actionParams");
  globdat.set(PropNames::LOAD_CASE, "output");
  allLoad = 0.;
  if (!globdat.find(allLoad, ActionParams::EXT_VECTOR))
  {
    params.set(ActionParams::EXT_VECTOR, allLoad);
    model->takeAction(Actions::GET_EXT_VECTOR, params, globdat);
    params.erase(ActionParams::EXT_VECTOR);
  }

  allResp = 0.;
  if (!globdat.find(allResp, ActionParams::INT_VECTOR))
  {
    params.set(ActionParams::INT_VECTOR, allResp);
    model->takeAction(Actions::GET_INT_VECTOR, params, globdat);
    params.erase(ActionParams::INT_VECTOR);
  }
  globdat.erase(PropNames::LOAD_CASE);

  Vector load, disp, velo, acce, resp;

  // Process node groups: compute sums for loads/responses, averages for displacements
  for (idx_t iNodeGroup = 0; iNodeGroup < nodeGroups_.size();
       iNodeGroup++)
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
    NodeGroup nodeGroup = NodeGroup::get(nodeGroups_[iNodeGroup], nodes,
                                         globdat, getContext());
    nodeIndices.resize(nodeGroup.size());
    dofIndices.resize(nodeGroup.size());
    load.resize(nodeGroup.size());
    resp.resize(nodeGroup.size());
    disp.resize(nodeGroup.size());
    velo.resize(nodeGroup.size());
    acce.resize(nodeGroup.size());
    nodeIndices = nodeGroup.getIndices();

    // Extract and aggregate DOF data for current node group
    for (idx_t iDof = 0; iDof < nodeDofs_.size(); iDof++)
    {
      // Get DOF indices for all nodes in the group
      dofs->getDofIndices(dofIndices, nodeIndices, nodeDofs_[iDof]);

      // Extract state variables for these DOFs
      disp = allDisp[dofIndices];
      if (doVelo)
        velo = allVelo[dofIndices];
      if (doAcce)
        acce = allAcce[dofIndices];

      load = allLoad[dofIndices];
      resp = allResp[dofIndices];

      // Store aggregated results: sum for forces, average for displacements
      loadVars.set(nodeDofNames_[iDof], sum(load));
      respVars.set(nodeDofNames_[iDof], sum(resp));
      dispVars.set(nodeDofNames_[iDof], sum(disp) / disp.size());
      if (doVelo)
        veloVars.set(nodeDofNames_[iDof], sum(velo) / velo.size());
      if (doAcce)
        acceVars.set(nodeDofNames_[iDof], sum(acce) / acce.size());
    }
  }

  // Process element groups: extract data from all nodes in the element group
  for (idx_t iElemGroup = 0; iElemGroup < elemGroups_.size();
       iElemGroup++)
  {
    // Get all nodes associated with this element group
    currentVars = myVars.makeProps(elemGroups_[iElemGroup]);
    ElementGroup elemGroup = ElementGroup::get(
        elemGroups_[iElemGroup], elems, globdat, getContext());
    nodeIndices.resize(elemGroup.getNodeIndices().size());
    dofIndices.resize(nodeIndices.size());
    load.resize(nodeIndices.size());
    disp.resize(nodeIndices.size());
    nodeIndices = elemGroup.getNodeIndices();

    // Extract data for each specified DOF dimension
    for (idx_t iDof = 0; iDof < elemDofs_.size(); iDof++)
    {
      // Get DOF indices for all nodes in the element group
      dofs->getDofIndices(dofIndices, nodeIndices, elemDofs_[iDof]);

      // Extract state variables for these DOFs
      disp = allDisp[dofIndices];
      load = allLoad[dofIndices];

      // Note: Element group processing currently extracts data but doesn't store it
      // TODO: Add storage of element group aggregated data if needed
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
