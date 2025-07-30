/**
 * @file FollowerLoadModel.cpp
 * @author Til Gärtner
 * @brief Implementation of follower load model for direction-dependent loading
 */

#include "FollowerLoadModel.h"
#include <jem/base/ClassTemplate.h>

//=======================================================================
//   class FollowerLoadModel
//=======================================================================

JEM_DEFINE_CLASS(FollowerLoadModel);

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *FollowerLoadModel::TYPE_NAME = "FollowerLoad";

const char *FollowerLoadModel::NODES_PROP = "nodeGroup";
const char *FollowerLoadModel::START_PROP = "start_dir";
const char *FollowerLoadModel::DOF_PROP = "dofs";
const char *FollowerLoadModel::ROT_DOF_PROP = "rot_dofs";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------
FollowerLoadModel::FollowerLoadModel

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat) : Model(name)
{
  // Get the Properties associated with this model
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  // get the global nodes
  nodes_ = NodeSet::find(globdat);

  // get all the properties
  myProps.get(nameGroup_, NODES_PROP);
  myConf.set(NODES_PROP, nameGroup_);

  myProps.get(orgDir_, START_PROP);
  myConf.set(START_PROP, orgDir_);

  myProps.get(forceDOFs_, DOF_PROP);
  myConf.set(DOF_PROP, forceDOFs_);

  myProps.get(rotDOFs_, ROT_DOF_PROP);
  myConf.set(ROT_DOF_PROP, rotDOFs_);
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------
bool FollowerLoadModel::takeAction

    (const String &action,
     const Properties &params,
     const Properties &globdat)
{
  if (action == Actions::INIT)
  {
    nodes_ = NodeSet::find(globdat);
    group_ = NodeGroup::get(nameGroup_, nodes_, globdat, getContext());
    dofs_ = DofSpace::get(nodes_.getData(), globdat, getContext());

    iForceDOFs_.resize(forceDOFs_.size());
    iRotDOFs_.resize(rotDOFs_.size());
    rotMats_.resize(forceDOFs_.size(), forceDOFs_.size(), group_.size());

    for (idx_t i = 0; i < iForceDOFs_.size(); i++)
      iForceDOFs_[i] = dofs_->getTypeIndex(forceDOFs_[i]);
    for (idx_t i = 0; i < iRotDOFs_.size(); i++)
      iRotDOFs_[i] = dofs_->getTypeIndex(rotDOFs_[i]);
    for (idx_t i = 0; i < group_.size(); i++)
      rotMats_[i] = eye();

    return true;
  }

  if (action == Actions::ADVANCE)
  {
    Vector d;

    // Get the last loadstep displacements
    StateVector::get(d, dofs_, globdat);

    advance_rots_(d);

    return true;
  }

  if (action == Actions::GET_EXT_VECTOR)
  {
    Vector f;
    double scale;

    // get the external vector
    params.get(f, ActionParams::EXT_VECTOR);
    // get the scale factor
    params.get(scale, ActionParams::SCALE_FACTOR);

    System::info() << " ...Scale Factor for " << myName_ << ": " << scale << "\n";

    get_ext_vec_(f, scale, globdat);

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------
//   get_ext_vec_
//-----------------------------------------------------------------------
void FollowerLoadModel::get_ext_vec_(const Vector &fext,
                                     const double scale,
                                     const Properties &globdat) const
{
  IdxVector inodes(group_.size());
  IdxVector if_dofs(iForceDOFs_.size());

  Matrix Lambda(iForceDOFs_.size(), iForceDOFs_.size());

  // get the node group indices
  inodes = group_.getIndices();

  for (idx_t inode = 0; inode < inodes.size(); inode++)
  {
    dofs_->getDofIndices(if_dofs, inodes[inode], iForceDOFs_);

    fext[if_dofs] += scale * matmul(rotMats_[inode], orgDir_);
  }
}

//-----------------------------------------------------------------------
//   advance_rots
//-----------------------------------------------------------------------
void FollowerLoadModel::advance_rots_(const Vector &d) const
{
  IdxVector inodes(group_.size());
  IdxVector ir_dofs(iRotDOFs_.size());

  Matrix Lambda(iForceDOFs_.size(), iForceDOFs_.size());

  // get the node group indices
  inodes = group_.getIndices();

  // update the rotations
  for (idx_t inode = 0; inode < inodes.size(); inode++)
  {
    dofs_->getDofIndices(ir_dofs, inodes[inode], iRotDOFs_);

    // TEST_CONTEXT(d[ir_dofs])
    expVec(rotMats_[inode], (Vector)d[ir_dofs]);
  }
  // TEST_CONTEXT(rotMats_)
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------
Ref<Model> FollowerLoadModel::makeNew

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)
{
  return newInstance<FollowerLoadModel>(name, conf, props, globdat);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------
void FollowerLoadModel::declare()
{
  using jive::model::ModelFactory;

  ModelFactory::declare(TYPE_NAME, &makeNew);
}
