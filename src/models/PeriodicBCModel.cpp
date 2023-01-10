/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *
 * This class implements a periodic BC Model
 *
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: September 21
 *
 * ONLY WORKS FOR RECTANGULAR UNIT CELLS
 *
 */
#include "PeriodicBCModel.h"
#include <jive/util/Printer.h>

const char *periodicBCModel::TYPE_NAME = "PeriodicBC";
const char *periodicBCModel::MODE_PROP = "mode";
const char *periodicBCModel::DOF_NAMES_PROP = "dofs";
const char *periodicBCModel::ROT_NAMES_PROP = "rotDofs";
const char *periodicBCModel::FIXEDGRAD_PARAM = "fixedGrad";

periodicBCModel::periodicBCModel

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

  // get the dof names
  myProps.get(dofNames_, DOF_NAMES_PROP);
  myConf.set(DOF_NAMES_PROP, dofNames_);

  pbcRank_ = dofNames_.size();

  // get the rot names
  myProps.find(rotNames_, ROT_NAMES_PROP);
  myConf.set(ROT_NAMES_PROP, rotNames_);

  // get the mode name
  String mode = "DISP";
  myProps.find(mode, MODE_PROP);
  myConf.set(MODE_PROP, mode.toLower());
  if (mode.toUpper() == "DISP")
  {
    gradName_ = "H";
    mode_ = DISP;
  }
  else if (mode.toUpper() == "UPD")
  {
    gradName_ = "H";
    mode_ = UPD;
  }
  else if (mode.toUpper() == "LOAD")
  {
    gradName_ = "P";
    mode_ = LOAD;
  }
  else
    throw jem::IllegalInputException("Unknown mode");

  // get the Gradient
  // H_ij = du_i/dX_j
  grad_.resize(pbcRank_, pbcRank_);
  grad_ = 0;
  Ref<Object> dummy;

  for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
    for (idx_t iEdge = 0; iEdge < pbcRank_; iEdge++)
    {
      if (myProps.find(dummy,
                       gradName_ + String(iDof + 1) + String(iEdge + 1)))
        if (!(dummy->toString().toUpper().equals("NAN")))
          grad_(iDof, iEdge) = jem::Float::dynamicCast(*dummy)->toFloat();
        else
          grad_(iDof, iEdge) = NAN;
      else
        grad_(iDof, iEdge) = 0.;
    }

  WARN_ASSERT2(jem::sum(grad_) != 0, "no displacement gradient given!");

  for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
    for (idx_t iEdge = 0; iEdge < pbcRank_; iEdge++)
      myConf.set(gradName_ + String(iDof + 1) + String(iEdge + 1),
                 grad_(iDof, iEdge));
}

bool periodicBCModel::takeAction

    (const String &action, const Properties &params,
     const Properties &globdat)
{
  using jive::model::ActionParams;
  using jive::model::Actions;

  if (action == Actions::INIT)
  {
    init_(globdat);
  }

  if (action == Actions::GET_CONSTRAINTS &&
      (mode_ == DISP || mode_ == UPD))
  {
    double scale;
    Vector currVec;
    Matrix currentGrad(grad_.shape());

    scale = NAN;
    currentGrad = NAN;

    // get the scale factor
    if (!globdat.find(currVec, FIXEDGRAD_PARAM))
      params.get(scale, ActionParams::SCALE_FACTOR);
    else
      vec2mat(currentGrad, currVec);

    setConstraints_(globdat, currentGrad, scale);

    return true;
  }

  if (action == Actions::GET_EXT_VECTOR && mode_ == LOAD)
  {
    double scale;
    Vector f;

    // get the scale factor & external force vector
    params.get(f, ActionParams::EXT_VECTOR);

    if (params.find(scale, ActionParams::SCALE_FACTOR))
      getExtVec_(f, globdat, scale);
    else
      getExtVec_(f, globdat);

    return true;
  }

  return false;
}

void periodicBCModel::init_(const Properties &globdat)
{
  IdxVector rdofs(rotNames_.size());
  for (idx_t iDof = 0; iDof < rdofs.size(); iDof++)
    rdofs[iDof] = dofs_->getTypeIndex(rotNames_[iDof]);

  jdofs_.resize(dofNames_.size());
  for (idx_t iDof = 0; iDof < jdofs_.size(); iDof++)
    jdofs_[iDof] = dofs_->getTypeIndex(dofNames_[iDof]);

  masterEdgeDofs_.resize(pbcRank_, pbcRank_);
  slaveEdgeDofs_.resize(pbcRank_, pbcRank_);
  cornerDofs_.resize(pbcRank_, pbcRank_);

  IdxVector masterRots;
  IdxVector slaveRots;
  idx_t corner0Dof;
  idx_t cornerRot;

  Assignable<NodeGroup> masterEdge;
  Assignable<NodeGroup> slaveEdge;
  Assignable<NodeGroup> corner;

  // fix 0 corner
  corner = NodeGroup::get(PBCGroupInputModule::CORNERS[0], nodes_,
                          globdat, getContext());
  for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
  {
    corner0Dof = dofs_->getDofIndex(corner.getIndex(0), jdofs_[iDof]);
    cons_->addConstraint(corner0Dof);
  }
  for (idx_t iDof = 0; iDof < rotNames_.size(); iDof++)
  {
    cornerRot = dofs_->getDofIndex(corner.getIndex(0), rdofs[iDof]);
    cons_->addConstraint(cornerRot);
  }

  // moving corners
  for (idx_t iDir = 0; iDir < pbcRank_; iDir++)
  {
    idx_t iEdge = dofs_->getTypeIndex(dofNames_[iDir]);

    masterEdge = NodeGroup::get(PBCGroupInputModule::EDGES[2 * iEdge],
                                nodes_, globdat, getContext());
    slaveEdge = NodeGroup::get(PBCGroupInputModule::EDGES[2 * iEdge + 1],
                               nodes_, globdat, getContext());
    corner = NodeGroup::get(PBCGroupInputModule::CORNERS[iEdge + 1],
                            nodes_, globdat, getContext());

    // save the translational DOFs for the
    for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
    {
      masterEdgeDofs_(iDof, iDir).resize(masterEdge.size());
      slaveEdgeDofs_(iDof, iDir).resize(slaveEdge.size());
      dofs_->getDofIndices(masterEdgeDofs_(iDof, iDir),
                           masterEdge.getIndices(), jdofs_[iDof]);
      dofs_->getDofIndices(slaveEdgeDofs_(iDof, iDir),
                           slaveEdge.getIndices(), jdofs_[iDof]);
      cornerDofs_(iDof, iDir) =
          dofs_->getDofIndex(corner.getIndex(0), jdofs_[iDof]);
    }

    // lock the rotational DOFs for the edges
    for (idx_t iDof = 0; iDof < rotNames_.size(); iDof++)
    {
      masterRots.resize(masterEdge.size());
      slaveRots.resize(slaveEdge.size());

      dofs_->getDofIndices(masterRots, masterEdge.getIndices(),
                           rdofs[iDof]);
      dofs_->getDofIndices(slaveRots, slaveEdge.getIndices(),
                           rdofs[iDof]);
      cornerRot = dofs_->getDofIndex(corner.getIndex(0), rdofs[iDof]);

      for (idx_t iN = 0; iN < masterRots.size(); iN++)
        cons_->addConstraint(slaveRots[iN], masterRots[iN], 1.0);
      cons_->addConstraint(cornerRot);
    }
  }

  // TEST_PRINTER((*cons_))
}

void periodicBCModel::setConstraints_(const Properties &globdat,
                                      const Matrix &currentGrad,
                                      const double scale)
{
  Matrix applyGrad(grad_.shape());

  applyGrad = std::isnan(scale) ? currentGrad : Matrix(scale * grad_);

  System::debug(myName_) << " ...Applying strain matrix \n"
                         << applyGrad << "\n";
  // TEST_PRINTER((*cons_))
  Matrix corner_deform(pbcRank_, pbcRank_);
  double size;

  for (idx_t i = 0; i < pbcRank_; i++)
  {
    Globdat::getVariables("all.extent", globdat).get(size, dofNames_[i]);
    corner_deform[i] = size * applyGrad[i];
  }
  // TEST_CONTEXT(corner_deform)

  // set the positive dofs
  for (idx_t iCorner = 0; iCorner < pbcRank_; iCorner++)
    for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
      if (std::isnan(corner_deform(iDof, iCorner)))
        cons_->eraseConstraint(cornerDofs_(iDof, iCorner));
      else
        cons_->addConstraint(cornerDofs_(iDof, iCorner),
                             corner_deform(iDof, iCorner));

  // iterate over the far field edges (right, top, behind)
  for (idx_t iEdge = 0; iEdge < pbcRank_; iEdge++)
  {
    // iterate over the deformation degrees
    for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
    {
      // iterate over the nodes
      for (idx_t iNode = 0; iNode < slaveEdgeDofs_(iDof, iEdge).size();
           iNode++)
        cons_->addConstraint(slaveEdgeDofs_(iDof, iEdge)[iNode],
                             {masterEdgeDofs_(iDof, iEdge)[iNode],
                              cornerDofs_(iDof, iEdge)},
                             {1., 1.});
    }
  }

  // TEST_CONTEXT(cons_->isSlaveDof(slaveEdgeDofs_(0, 0)[0]))
  // TEST_PRINTER((*cons_))
}

void periodicBCModel::getExtVec_(const Vector &f,
                                 const Properties &globdat,
                                 const double scale)
{
  NOT_IMPLEMENTED

  double extent = 1.;
  double area = 1.;
  idx_t nNodes = 0.;

  for (idx_t iDof = 0; iDof < pbcRank_; iDof++)
    for (idx_t iDir = 0; iDir < pbcRank_; iDir++)
    {
      idx_t iEdge = dofs_->getTypeIndex(dofNames_[iDir]);
      // TEST_CONTEXT(f[slaveEdgeDofs_(iDof, iDir)])
      if (std::isnan(grad_(iDof, iDir)))
        continue; // if the dispGrad for this is not configured, skip
                  // it

      area = 1.;
      for (idx_t iDim = 0; iDim < pbcRank_; iDim++)
        if (iDim != iDir)
        {
          try
          {
            Globdat::getVariables("all.extent", globdat)
                .get(extent, dofNames_[iDim]);
          }
          catch (const jem::util::PropertyException &e)
          {
            Globdat::getVariables("SIZE", globdat)
                .get(extent,
                     jem::String(dofNames_[iDim].back()).toUpper());
          }

          area *= extent;
        }
      nNodes = masterEdgeDofs_(iDof, iDir).size();
      // TEST_CONTEXT(area)

      System::info(myName_) << " ...Applying stress in direction of "
                            << dofNames_[iDof] << "\n";
      System::info(myName_)
          << "      of magnitude " << scale * grad_(iDof, iDir) << "\n";
      System::info(myName_)
          << "      at " << PBCGroupInputModule::EDGES[2 * iEdge + 1]
          << " \n";

      for (idx_t iNode = 0; iNode < masterEdgeDofs_(iDof, iDir).size();
           iNode++)
      {
        // set the unit load at the slave nodes
        f[slaveEdgeDofs_(iDof, iDir)[iNode]] +=
            scale * grad_(iDof, iDir) * area / nNodes;
      }

      // TEST_CONTEXT(f[slaveEdgeDofs_(iDof, iDir)])
    }
}

Ref<Model> periodicBCModel::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)
{
  return newInstance<periodicBCModel>(name, conf, props, globdat);
}

void periodicBCModel::declare()
{
  using jive::model::ModelFactory;

  ModelFactory::declare(TYPE_NAME, &makeNew);
}