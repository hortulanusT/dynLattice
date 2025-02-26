/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *
 * This class implements a symmetric BC Model
 *
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: Feburary 2025
 *
 */
#include "SymBCModel.h"
#include <jive/util/Printer.h>

const char *SymBCModel::TYPE_NAME = "SymBC";
const char *SymBCModel::DOF_OPPO_NAMES_PROP = "oppo_dofs";
const char *SymBCModel::DOF_EQUAL_NAMES_PROP = "equal_dofs";
const char *SymBCModel::SURFACES_PROP = "surfaces";

SymBCModel::SymBCModel

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)
    : Model(name)
{
  // Get the Properties associated with this model
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  // get the global nodeSet, dofSpace and Constraints
  nodes_ = NodeSet::get(globdat, getContext());
  dofs_ = DofSpace::get(nodes_.getData(), globdat, getContext());
  cons_ = Constraints::get(dofs_, globdat);

  // get the dof names (for opposing symmetry)
  if (myProps.find(dofOppoNames_, DOF_OPPO_NAMES_PROP))
    myConf.set(DOF_OPPO_NAMES_PROP, dofOppoNames_);

  // get the dof names (for equal symmetry)
  if (myProps.find(dofEqualNames_, DOF_EQUAL_NAMES_PROP))
    myConf.set(DOF_EQUAL_NAMES_PROP, dofEqualNames_);

  // get the surface names
  myProps.get(surfaceNames_, SURFACES_PROP);
  myConf.set(SURFACES_PROP, surfaceNames_);

  JEM_PRECHECK2(dofOppoNames_.size() + dofEqualNames_.size() > 0, "SymBCModel: one dof needs to be given");
  JEM_PRECHECK2(surfaceNames_.size() % 2 == 0, "SymBCModel: Only even numbers of surfaces are supported");
}

bool SymBCModel::takeAction

    (const String &action, const Properties &params,
     const Properties &globdat)
{
  using jive::model::ActionParams;
  using jive::model::Actions;

  if (action == Actions::INIT)
  {
    init_(globdat);
  }

  if (action == Actions::GET_CONSTRAINTS)
  {
    setConstraints_();

    return true;
  }

  return false;
}

void SymBCModel::init_(const Properties &globdat)
{
  idx_t jtype;
  Assignable<NodeGroup> mainNodes;
  Assignable<NodeGroup> secondaryNodes;
  IdxVector mainSurfDofs;
  IdxVector secondarySurfDofs;
  IdxBuffer mainOppoDofs;
  IdxBuffer secondaryOppoDofs;
  IdxBuffer mainEqualDofs;
  IdxBuffer secondaryEqualDofs;

  // get the nodes for the surfaces
  for (idx_t iSurf = 0; iSurf < surfaceNames_.size() / 2; iSurf++)
  {
    mainNodes = NodeGroup::get(surfaceNames_[iSurf * 2], nodes_, globdat, getContext());
    secondaryNodes = NodeGroup::get(surfaceNames_[iSurf * 2 + 1], nodes_, globdat, getContext());

    JEM_ASSERT2(mainNodes.size() == secondaryNodes.size(), "SymBCModel: The two surfaces must have the same number of nodes");

    mainSurfDofs.resize(mainNodes.size());
    secondarySurfDofs.resize(secondaryNodes.size());

    for (idx_t jdof = 0; jdof < dofOppoNames_.size(); jdof++)
    {
      // get the dof type id
      jtype = dofs_->getTypeIndex(dofOppoNames_[jdof]);

      // get the dofs for the nodes
      dofs_->getDofIndices(mainSurfDofs, mainNodes.getIndices(), jtype);
      dofs_->getDofIndices(secondarySurfDofs, secondaryNodes.getIndices(), jtype);

      // store the dofs
      mainOppoDofs.pushBack(mainSurfDofs.begin(), mainSurfDofs.end());
      secondaryOppoDofs.pushBack(secondarySurfDofs.begin(), secondarySurfDofs.end());
    }

    for (idx_t jdof = 0; jdof < dofEqualNames_.size(); jdof++)
    {
      // get the dof type id
      jtype = dofs_->getTypeIndex(dofEqualNames_[jdof]);

      // get the dofs for the nodes
      dofs_->getDofIndices(mainSurfDofs, mainNodes.getIndices(), jtype);
      dofs_->getDofIndices(secondarySurfDofs, secondaryNodes.getIndices(), jtype);

      // store the dofs
      mainEqualDofs.pushBack(mainSurfDofs.begin(), mainSurfDofs.end());
      secondaryEqualDofs.pushBack(secondarySurfDofs.begin(), secondarySurfDofs.end());
    }
  }

  // store the dofs
  mainOppoDofs_.ref(mainOppoDofs.toArray());
  secondaryOppoDofs_.ref(secondaryOppoDofs.toArray());
  mainEqualDofs_.ref(mainEqualDofs.toArray());
  secondaryEqualDofs_.ref(secondaryEqualDofs.toArray());
}

void SymBCModel::setConstraints_()
{
  for (idx_t i = 0; i < mainOppoDofs_.size(); i++)
  {
    cons_->addConstraint(mainOppoDofs_[i], secondaryOppoDofs_[i], -1.0);
  }

  for (idx_t i = 0; i < mainEqualDofs_.size(); i++)
  {
    cons_->addConstraint(mainEqualDofs_[i], secondaryEqualDofs_[i], 1.0);
  }
}

Ref<Model> SymBCModel::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)
{
  return newInstance<SymBCModel>(name, conf, props, globdat);
}

void SymBCModel::declare()
{
  using jive::model::ModelFactory;

  ModelFactory::declare(TYPE_NAME, &makeNew);
}
