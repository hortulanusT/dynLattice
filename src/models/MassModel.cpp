#include "models/MassModel.h"
#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(MassModel);

//=======================================================================
//   class MassModel
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------
const char *MassModel::TYPE_NAME = "Mass";
const char *MassModel::TOTAL_MASS_PROP = "totalMass";
const char *MassModel::NODE_GROUPS_PROP = "nodeGroups";
const char *MassModel::DOFS_PROP = "dofs";

//-----------------------------------------------------------------------
//   constructors & destructor
//-----------------------------------------------------------------------
MassModel::MassModel

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat) :

                                  Super(name)

{
  // Get the Properties associated with this model
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  // get the total mass
  myProps.get(totalMass_, TOTAL_MASS_PROP);
  myConf.set(TOTAL_MASS_PROP, totalMass_);

  // get the node group names
  myProps.get(nodeGroupNames_, NODE_GROUPS_PROP);
  myConf.set(NODE_GROUPS_PROP, nodeGroupNames_);

  JEM_PRECHECK2(nodeGroupNames_.size() == totalMass_.size(), "NodeGroups and totalMass need to have the same length! ");

  // get the dofs
  myProps.get(dofNames_, DOFS_PROP);
  myConf.set(DOFS_PROP, dofNames_);
}

MassModel::~MassModel() {}

//-----------------------------------------------------------------------
//  takeAction
//-----------------------------------------------------------------------
bool MassModel::takeAction

    (const String &action,
     const Properties &params,
     const Properties &globdat)
{
  if (action == Actions::INIT)
  {
    // get the node groups
    nodeGroups_.resize(nodeGroupNames_.size());
    NodeSet allNodes = NodeSet::get(globdat, getContext());

    for (idx_t i = 0; i < nodeGroupNames_.size(); i++)
    {
      nodeGroups_[i] = NodeGroup::get(nodeGroupNames_[i], allNodes, globdat, getContext());
    }

    // get the dofs
    dofs_ = DofSpace::get(globdat, getContext());
    dofTypes_.resize(dofNames_.size());

    for (idx_t i = 0; i < dofNames_.size(); i++)
    {
      dofTypes_[i] = dofs_->getTypeIndex(dofNames_[i]);
    }

    return true;
  }

  if (action == Actions::GET_MATRIX2)
  {
    Ref<MatrixBuilder> mbld;
    params.get(mbld, ActionParams::MATRIX2);

    // iterate through the masses and add them to the matrix
    for (idx_t i = 0; i < totalMass_.size(); i++)
    {
      IdxVector inodes = nodeGroups_[i].getIndices();
      IdxVector idofs(inodes.size() * dofTypes_.size());
      dofs_->getDofIndices(idofs, inodes, dofTypes_);

      double massToAdd = totalMass_[i] / inodes.size();

      for (idx_t idof : idofs)
      {
        mbld->addValue(idof, idof, massToAdd);
      }
    }
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------
Ref<Model> MassModel::makeNew

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
void MassModel::declare()
{
  using jive::model::ModelFactory;

  ModelFactory::declare(TYPE_NAME, &makeNew);
  ModelFactory::declare(CLASS_NAME, &makeNew);
}
