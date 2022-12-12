/**
 * @file PBCGroupOutputModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Wrapper Class for some default PBC Outputs
 * @version 0.1
 * @date 2021-12-06
 *
 * @copyright Copyright (C) 2021 TU Delft. All rights reserved.
 *
 */

#include "PBCGroupOutputModule.h"

const char *PBCGroupOutputModule::TYPE_NAME = "PBCGroupOutput";
const char *PBCGroupOutputModule::CHILD_NAME = "sampling";

PBCGroupOutputModule::PBCGroupOutputModule(const String &name)
    : Super(name)
{
  child_ = newInstance<SampleModule>(myName_ + "." + CHILD_NAME);
}

Module::Status PBCGroupOutputModule::init(const Properties &conf,
                                          const Properties &props,
                                          const Properties &globdat)
{

  // fill the properties with the default node and element Groups
  Properties myProps = props.getProps(myName_);

  StringVector groups;
  myProps.find(groups, "nodeGroups");
  groups.reshape(groups.size() + 6);
  for (idx_t i = 1; i <= 6; i++)
    groups[groups.size() - i] = PBCGroupInputModule::EDGES[i - 1];
  myProps.set("nodeGroups", groups);

  StringVector elements;
  myProps.find(elements, "elementGroups");
  elements.reshape(elements.size() + 1);
  elements[elements.size() - 1] = "all";
  myProps.set("elemGroups", elements);

  JEM_PRECHECK2(Super::init(conf, props, globdat) == Status::OK,
                "Error setting up the GroupOutputModule!");

  // configure the output module
  // LATER get multiple Children for different kinds of outputs
  Properties childProps = props.makeProps(myName_ + "." + CHILD_NAME);
  bool append = false;
  if (!childProps.find(append, PropNames::APPEND) || !append)
  {
    String header = "";
    childProps.find(header, PropNames::HEADER);
    childProps.set(PropNames::HEADER, getHeader_(header));
  }
  StringVector data_sets;
  childProps.find(data_sets, PropNames::DATA_SETS);
  childProps.set(PropNames::DATA_SETS, getDataSets_(data_sets));

  childProps.set(PropNames::SEPARATOR, ",");

  child_->configure(props, globdat);
  child_->getConfig(conf, globdat);
  JEM_PRECHECK2(child_->init(conf, props, globdat) == Status::OK,
                "Error setting up the SampleModule!");

  return Status::OK;
}

Module::Status PBCGroupOutputModule::run(const Properties &globdat)
{
  Super::run(globdat);

  child_->run(globdat);

  return Status::OK;
}

void PBCGroupOutputModule::shutdown(const Properties &globdat)
{
  Super::shutdown(globdat);
  child_->shutdown(globdat);
}

String PBCGroupOutputModule::getHeader_(String head) const
{
  const idx_t dim = elemDofs_.size();

  // step
  if (head.size() < 1)
    head = "step,";

  if (head.back() != ',')
    head = head + ",";

  // displacement gradient
  for (idx_t i = 1; i <= dim; i++)
    for (idx_t j = 1; j <= dim; j++)
      head = head + String::format("H%d%d,", i, j);

  // 1st PK Tensor
  for (idx_t i = 1; i <= dim; i++)
    for (idx_t j = 1; j <= dim; j++)
      head = head + String::format("P%d%d,", i, j);

  return head[SliceTo(head.size() - 1)];
}

StringVector PBCGroupOutputModule::getDataSets_(
    StringVector existingDataSets) const
{
  const idx_t dim = elemDofs_.size();

  ArrayBuffer<String> dataSets;

  // step
  if (existingDataSets.size() > 0)
    dataSets.pushBack(existingDataSets.begin(), existingDataSets.end());
  else
    dataSets.pushBack("i");

  // displacement gradient
  for (String dispGrad : getDataSets(dim, true, false))
    dataSets.pushBack(dispGrad);

  // 1st PK Tensor
  for (String stressPK : getDataSets(dim, false, true))
    dataSets.pushBack(stressPK);

  return dataSets.toArray();
}

StringVector PBCGroupOutputModule::getDataSets(
    const idx_t dim, const bool strains, const bool stresses,
    const StringVector &dofNames)
{
  JEM_ASSERT2(dofNames.size() >= dim, "Not enough dofnames given!");
  ArrayBuffer<String> dataSets;

  if (strains)
  {
    // displacement gradient
    for (idx_t i = 0; i < dim; i++)
      for (idx_t j = 0; j < dim; j++)
      {
        dataSets.pushBack(String::format(
            "(%cmax.disp.%s - %cmin.disp.%s) / all.extent.%s",
            dofNames[j].back(), dofNames[i], dofNames[j].back(),
            dofNames[i], dofNames[j]));
      }
  }

  if (stresses)
  {
    // Prepare areas
    StringVector areas(dim);
    if (dim == 3)
    {
      areas[0] = String::format("( all.extent.%s * all.extent.%s )",
                                dofNames[1], dofNames[2]);
      areas[1] = String::format("( all.extent.%s * all.extent.%s )",
                                dofNames[0], dofNames[2]);
      areas[2] = String::format("( all.extent.%s * all.extent.%s )",
                                dofNames[1], dofNames[0]);
    }
    else if (dim == 2)
    {
      areas[0] = "all.extent." + dofNames[1];
      areas[1] = "all.extent." + dofNames[0];
    }
    else if (dim == 1)
    {
      areas[0] = "1";
    }
    else
      throw jem::Exception(
          JEM_FUNC, String::format("unkown dimension number %d", dim));

    // 1st PK Tensor
    for (idx_t i = 0; i < dim; i++)
      for (idx_t j = 0; j < dim; j++)
        dataSets.pushBack(String::format("%cmax.resp.%s / %s",
                                         dofNames[j].back(), dofNames[i],
                                         areas[j]));
  }

  return dataSets.toArray();
}

Ref<Module> PBCGroupOutputModule::makeNew(const String &name,
                                          const Properties &conf,
                                          const Properties &props,
                                          const Properties &globdat)
{
  return newInstance<PBCGroupOutputModule>(name);
}

void PBCGroupOutputModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &makeNew);
}