/*
 *  Copyright (C) 2015 TU Delft. All rights reserved.
 *
 *  Frans van der Meer, January 2015
 *
 *  Module to generate default NodeGroups for periodic boundary conditions
 *  It basically runs the GroupInputModule with some predefined input
 *
 */

#include <jem/base/Error.h>
#include <jem/base/System.h>
#include <jem/base/array/operators.h>
#include <jem/base/array/select.h>
#include <jem/base/array/utilities.h>
#include <jem/numeric/algebra/utilities.h>
#include <jem/util/ArrayBuffer.h>
#include <jem/util/Properties.h>
#include <jive/app/ModuleFactory.h>
#include <jive/fem/NodeGroup.h>

#include "PBCGroupInputModule.h"
#include "utils/testing.h"

using jem::Error;
using jem::io::endl;
using jem::numeric::norm2;
using jem::util::ArrayBuffer;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;

//=======================================================================
//   class PBCGroupInputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *PBCGroupInputModule::TYPE_NAME = "PBCGroupInput";
const char *PBCGroupInputModule::EDGES[6] = {"xmin", "xmax", "ymin",
                                             "ymax", "zmin", "zmax"};
const char *PBCGroupInputModule::CORNERS[4] = {"corner0", "cornerx",
                                               "cornery", "cornerz"};
const char *PBCGroupInputModule::DUPEDNODES_PROP = "duplicatedNodes";
const char *PBCGroupInputModule::NGROUPS_PROP = "groupSettings";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

PBCGroupInputModule::PBCGroupInputModule

    (const String &name)
    :

      Super(name)

{
  rank_ = -1;
  dupedNodeGroup_ = "";
  groupSettings_ = Properties();
  edges_ = true;
  corners_ = true;
}

PBCGroupInputModule::~PBCGroupInputModule()
{
}

//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------

Module::Status PBCGroupInputModule::init

    (const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  // check what groups to get
  props.findProps(myName_).find(corners_, "corners");
  props.findProps(myName_).find(edges_, "edges");

  // set some variables

  NodeSet nodes = NodeSet::find(globdat);
  rank_ = nodes.rank();

  Matrix coords(rank_, nodes.size());

  nodes.getCoords(coords);

  double dx = max(coords(0, ALL)) - min(coords(0, ALL));
  double dz = max(coords(1, ALL)) - min(coords(1, ALL));
  double dy = max(coords(2, ALL)) - min(coords(2, ALL));

  small_ = max(dx, dy, dz) / 1.e6;

  // get duplicated nodes, if they exist

  props.findProps(myName_).find(dupedNodeGroup_, DUPEDNODES_PROP);

  // get the restriction Group
  props.findProps(myName_).find(groupSettings_, NGROUPS_PROP);

  // make default Properties object for Super

  Properties myProps = props.makeProps(myName_);

  prepareProps_(myProps);

  // Touch the pre settings to get rid of warinings
  Ref<Object> dummyObj;

  for (idx_t i = 0; i < groupSettings_.size(); i++)
    groupSettings_.find(dummyObj, groupSettings_.listProps()[i]);

  // make NodeGroups

  Super::init(conf, props, globdat);

  // sort NodeGroups such that ordering of opposite faces is matching

  for (idx_t i = 0; i < rank_; ++i)
  {
    NodeGroup edge0 = NodeGroup::find(EDGES[i * 2], nodes, globdat);
    NodeGroup edge1 = NodeGroup::find(EDGES[i * 2 + 1], nodes, globdat);

    IdxVector inodes0 = edge0.getIndices();
    IdxVector inodes1 = edge1.getIndices();

    if (inodes0.size() != inodes1.size())
    {
      jem::System::warn()
          << EDGES[i * 2] << " and " << EDGES[i * 2 + 1]
          << " opposite edges do not have the same number of nodes\n";
      continue;
    }

    sortBoundaryNodes_(inodes1, inodes0, nodes, globdat, i);

    NodeGroup updated = newNodeGroup(inodes1, nodes);

    updated.store(EDGES[i * 2 + 1], globdat);

    System::info(myName_) << " ...Sorted NodeGroup `" << EDGES[i * 2 + 1]
                          << "' wrt `" << EDGES[i * 2] << "'\n";
  }

  Properties myVars = Globdat::getVariables(globdat);
  Properties extentVars;

  extentVars = myVars.makeProps("all").makeProps("extent");

  // report back the extent
  for (idx_t iDof = 0; iDof < 3; iDof++)
  {
    IdxVector n_min =
        NodeGroup::get(EDGES[iDof * 2], nodes, globdat, getContext())
            .getIndices();
    IdxVector n_max =
        NodeGroup::get(EDGES[iDof * 2 + 1], nodes, globdat, getContext())
            .getIndices();

    double c_min = min(coords(iDof, n_min));
    double c_max = max(coords(iDof, n_max));

    extentVars.set(String::format("d%c", EDGES[iDof * 2][0]),
                   c_max - c_min);
  }

  return DONE;
}

//-----------------------------------------------------------------------
//   prepareProps_
//-----------------------------------------------------------------------

void PBCGroupInputModule::prepareProps_

    (const Properties &myProps) const

{
  myProps.set("eps", small_);

  StringVector nGroupNames(3 * rank_ + 1);

  idx_t j = 0;
  if (corners_)
    nGroupNames[j++] = CORNERS[0];
  for (idx_t i = 0; i < rank_; ++i)
  {
    if (corners_)
      nGroupNames[j++] = CORNERS[i + 1];
    if (edges_)
      nGroupNames[j++] = EDGES[i * 2];
    if (edges_)
      nGroupNames[j++] = EDGES[i * 2 + 1];
  }

  nGroupNames.resize(j);

  // read names of Groups that are to be created

  StringVector TYPES = {".xtype", ".ytype", ".ztype"};
  StringVector VALS = {".xval", ".yval", ".zval"};
  String MIN = "min";
  String MAX = "max";
  Properties groupProps;
  StringVector existingGroups;
  double dummy;

  myProps.find(existingGroups, NODE_GROUPS);
  StringVector newGroups(existingGroups.size() + nGroupNames.size());
  newGroups[jem::SliceTo(existingGroups.size())] = existingGroups;
  newGroups[jem::SliceFrom(existingGroups.size())] = nGroupNames;
  myProps.set(NODE_GROUPS, newGroups);
  for (idx_t i = 0; i < j; i++)
  {
    groupProps = myProps.makeProps(nGroupNames[i]);
    groupProps.mergeWith(groupSettings_);
  }

  for (idx_t k = 0; k < rank_; k++)
  {
    if (!myProps.find(dummy, String(CORNERS[0]) + VALS[k]))
      myProps.set(String(CORNERS[0]) + TYPES[k], MIN);
  }

  for (idx_t i = 0; i < rank_; ++i)
  {
    if (corners_)
      for (idx_t k = 0; k < rank_; k++)
      {
        if (!myProps.find(dummy, String(CORNERS[i + 1]) + VALS[k]))
          myProps.set(String(CORNERS[i + 1]) + TYPES[k],
                      k == i ? MAX : MIN);
      }
    if (edges_)
    {
      if (!myProps.find(dummy, String(EDGES[i * 2]) + VALS[i]))
        myProps.set(String(EDGES[i * 2]) + TYPES[i], MIN);
      if (!myProps.find(dummy, String(EDGES[i * 2 + 1]) + VALS[i]))
        myProps.set(String(EDGES[i * 2 + 1]) + TYPES[i], MAX);
    }
  }
}

//-----------------------------------------------------------------------
//   sortBoundaryNodes_
//----------------------------------------------------------------------

void PBCGroupInputModule::sortBoundaryNodes_

    (const IdxVector &islaves, const IdxVector &imasters,
     const NodeSet &nodes, const Properties &globdat,
     const idx_t ix) const

{

  // make sure that ordering of islaves matches orderning of imasters

  JEM_ASSERT(islaves.size() == imasters.size());
  const idx_t nn = islaves.size();

  Vector mcoords(rank_);
  Vector scoords(rank_);
  IdxVector sorted(nn);

  sorted = -1;

  // collect relevant coordinates
  // e.g. for XMIN and XMAX plane, compare y and z coordinate

  ArrayBuffer<idx_t> ibuf;
  for (idx_t jx = 0; jx < rank_; ++jx)
  {
    if (jx != ix)
      ibuf.pushBack(jx);
  }
  IdxVector irelevant(ibuf.toArray());

  for (idx_t in = 0; in < nn; ++in)
  {
    nodes.getNodeCoords(mcoords, imasters[in]);

    for (idx_t jn = 0; jn < nn; ++jn)
    {
      nodes.getNodeCoords(scoords, islaves[jn]);

      double dist = norm2(scoords[irelevant] - mcoords[irelevant]);

      if (dist < small_)
      {
        if (dupedNodeGroup_ == "")
        {
          sorted[in] = islaves[jn];
          break;
        }
        else
        {
          NodeGroup newNodes(
              NodeGroup::get(dupedNodeGroup_, nodes, globdat, ""));

          bool neither = !newNodes.contains(imasters[in]) &&
                         !newNodes.contains(islaves[jn]);

          bool both = newNodes.contains(imasters[in]) &&
                      newNodes.contains(islaves[jn]);

          if (neither || both)
          {
            sorted[in] = islaves[jn];
            break;
          }
        }
      }

      if (jn == nn - 1)
      {
        throw Error(JEM_FUNC, "No match found for node " + String(in));
      }
    }
  }
  JEM_ASSERT(testall(sorted >= 0));
  islaves = sorted;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> PBCGroupInputModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<Self>(name);
}
//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void PBCGroupInputModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(PBCGroupInputModule::TYPE_NAME,
                         &PBCGroupInputModule::makeNew);
}
