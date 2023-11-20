/**
 * @file LatticeModel.cpp
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief LatticeModel for collection of rods
 * @version 0.1
 * @date 2022-04-26
 *
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 *
 */

#include "models/LatticeModel.h"

//=======================================================================
//   class LatticeModel
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------
const char *LatticeModel::TYPE_NAME = "Lattice";
const char *LatticeModel::CHILD_PROPS = "child";
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
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------
bool LatticeModel::takeAction

    (const String &action,
     const Properties &params,
     const Properties &globdat)
{
  bool success = true;
  for (Ref<Model> child : children_)
    success = child->takeAction(action, params, globdat) && success;
  return success;
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
