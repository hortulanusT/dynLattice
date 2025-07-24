/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *
 * This class implements a special Cosserat Rod model
 * (also called Simo Reissner Rod or nonlinear
 * Timoshenko Rod)
 *
 * Author: T. Gaertner
 * Date: July 21
 *
 */

#include "specialCosseratRodModel.h"

#include <math.h>

//=======================================================================
//    class specialCosseratRodModel -- implementation
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *specialCosseratRodModel::TYPE_NAME = "specialCosseratRod";
const char *specialCosseratRodModel::TRANS_DOF_DEFAULT = "trans_";
const char *specialCosseratRodModel::ROT_DOF_DEFAULT = "rot_";
const char *specialCosseratRodModel::TRANS_DOF_NAMES = "dofNamesTrans";
const char *specialCosseratRodModel::ROT_DOF_NAMES = "dofNamesRot";
const char *specialCosseratRodModel::SYMMETRIC_ONLY =
    "symmetric_tanget_stiffness";
const char *specialCosseratRodModel::MATERIAL_Y_DIR = "material_ey";
const char *specialCosseratRodModel::GIVEN_NODES = "given_dir_nodes";
const char *specialCosseratRodModel::GIVEN_DIRS = "given_dir_dirs";
const char *specialCosseratRodModel::LUMPED_MASS = "lumpedMass";
const char *specialCosseratRodModel::HINGES = "hinges";
const idx_t specialCosseratRodModel::TRANS_DOF_COUNT = 3;
const idx_t specialCosseratRodModel::ROT_DOF_COUNT = 3;
const Slice specialCosseratRodModel::TRANS_PART =
    jem::SliceFromTo(0, TRANS_DOF_COUNT);
const Slice specialCosseratRodModel::ROT_PART =
    jem::SliceFromTo(TRANS_DOF_COUNT, TRANS_DOF_COUNT + ROT_DOF_COUNT);

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------

specialCosseratRodModel::specialCosseratRodModel

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)
    :

      Model(name)

{
  // Get the Properties associated with this model
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  // Get the element name from the global database
  String elementsName = jem::util::StringUtils::split(myName_, '.').back();

  // ARRANGE DOF NAMES FIRST
  // set the default
  Array<String> trans_dofs(TRANS_DOF_COUNT);
  Array<String> rot_dofs(ROT_DOF_COUNT);

  for (idx_t i = 0; i < TRANS_DOF_COUNT; i++)
    trans_dofs[i] = TRANS_DOF_DEFAULT + String(i);
  for (idx_t i = 0; i < ROT_DOF_COUNT; i++)
    rot_dofs[i] = ROT_DOF_DEFAULT + String(i);

  // get the names
  myProps.find(trans_dofs, TRANS_DOF_NAMES);
  myProps.find(rot_dofs, ROT_DOF_NAMES);

  myConf.set(TRANS_DOF_NAMES, trans_dofs);
  myConf.set(ROT_DOF_NAMES, rot_dofs);

  // create hinges in the first step (if neccessary)
  if (myProps.contains(HINGES))
  {
    Properties hingeProps = myProps.getProps(HINGES);
    hingeProps.set("material", material_);
    hingeProps.set("elements", jive::util::joinNames(elementsName, HINGES));
    hinges_ = jive::model::ModelFactory::newInstance(HINGES, myConf, myProps, globdat);
  }
  else
    hinges_ = nullptr;

  // Get the elements and nodes from the global database
  allElems_ = ElementSet::get(globdat, getContext()); // all the elements
  rodElems_ = ElementGroup::get(myName_, allElems_, globdat,
                                getContext()); // only the desired group
  allNodes_ = allElems_.getNodes();            // all the inodes

  // store the inverse relation from the global node ids to the local
  // indizes on this rod
  rodNodes_.resize(allNodes_.size());
  rodNodes_ = -1;
  rodNodes_[rodElems_.getNodeIndices()] =
      jem::iarray(rodElems_.getNodeIndices().size());

  // Initialize the internal shape.
  myProps.makeProps("stiffShape").set("numPoints", allElems_.getElemNodeCount(rodElems_.getIndex(0)));
  shapeK_ = newInstance<Line3D>("stiffShape", myConf, myProps);
  myProps.makeProps("massShape").set("numPoints", allElems_.getElemNodeCount(rodElems_.getIndex(0)));
  myProps.makeProps("massShape").set("intScheme", "Gauss" + String(allElems_.getElemNodeCount(rodElems_.getIndex(0))));
  shapeM_ = newInstance<Line3D>("massShape", myConf, myProps);

  // Check whether the mesh is valid.
  rodElems_.checkElements(getContext(), shapeK_->nodeCount());
  rodElems_.checkElements(getContext(), shapeM_->nodeCount());

  // Define the DOFs.
  Ref<XDofSpace> dofs = XDofSpace::get(allNodes_.getData(), globdat);
  trans_types_.resize(TRANS_DOF_COUNT);
  rot_types_.resize(ROT_DOF_COUNT);
  jtypes_.resize(TRANS_DOF_COUNT + ROT_DOF_COUNT);

  // create the DOFs
  for (idx_t i = 0; i < TRANS_DOF_COUNT; i++)
    trans_types_[i] = dofs->addType(trans_dofs[i]);
  for (idx_t i = 0; i < ROT_DOF_COUNT; i++)
    rot_types_[i] = dofs->addType(rot_dofs[i]);

  jtypes_[TRANS_PART] = trans_types_;
  jtypes_[ROT_PART] = rot_types_;

  // Assign the DOFs to the inodes
  for (idx_t inode = 0; inode < allNodes_.size(); inode++)
  {
    for (idx_t idof = 0; idof < trans_types_.size(); idof++)
      dofs->addDof(inode, trans_types_[idof]);
    for (idx_t idof = 0; idof < rot_types_.size(); idof++)
      dofs->addDof(inode, rot_types_[idof]);
  }

  // get the nonmutable DOF-Space into the class member
  dofs_ = dofs;

  // get the material
  props.set(joinNames(myName_, "material.ipCount"), shapeK_->ipointCount());
  props.set(joinNames(myName_, "material.elemCount"), rodElems_.size());
  material_ = MaterialFactory::newInstance(joinNames(myName_, "material"), conf, props, globdat);

  // get the incremental property
  symmetric_only_ = false;
  myProps.find(symmetric_only_, SYMMETRIC_ONLY);
  myConf.set(SYMMETRIC_ONLY, symmetric_only_);

  // Get the material parameters.
  if (myProps.find(material_ey_, MATERIAL_Y_DIR))
  {
    JEM_ASSERT(material_ey_.size() == allNodes_.rank());
    JEM_ASSERT(norm2(material_ey_) == 1.);
    myConf.set(MATERIAL_Y_DIR, material_ey_);
  }

  // get given node dirs
  if (myProps.find(givenNodes_, GIVEN_NODES))
  {
    Vector givenDirs(givenNodes_.size() * TRANS_DOF_COUNT);
    givenDirs_.resize(TRANS_DOF_COUNT, givenNodes_.size());

    myProps.get(givenDirs, GIVEN_DIRS);
    myConf.set(GIVEN_NODES, givenNodes_);
    myConf.set(GIVEN_DIRS, givenDirs);

    vec2mat(givenDirs_.transpose(), givenDirs);
  }
  else if (Globdat::hasVariable(joinNames("tangents", elementsName),
                                globdat))
  {
    Properties tangentVars = Globdat::getVariables(
        joinNames("tangents", elementsName), globdat);
    tangentVars.get(givenNodes_, GIVEN_NODES);

    Vector givenDirs(givenNodes_.size() * TRANS_DOF_COUNT);
    givenDirs_.resize(TRANS_DOF_COUNT, givenNodes_.size());

    tangentVars.get(givenDirs, GIVEN_DIRS);
    myConf.set(GIVEN_NODES, givenNodes_);
    myConf.set(GIVEN_DIRS, givenDirs);

    vec2mat(givenDirs_.transpose(), givenDirs);
  }
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------

// Performs an action requested by a module or a parent model.

bool specialCosseratRodModel::takeAction

    (const String &action, const Properties &params,
     const Properties &globdat)

{
  using jive::model::ActionParams;
  using jive::model::Actions;
  using jive::model::StateVector;

  // REPORT(action)
  // TEST_CONTEXT(myName_)
  // TEST_CONTEXT(params)

  if (action == Actions::INIT)
  {
    init_rot_();
    init_strain_();
    // TEST_CONTEXT(LambdaN_)
    // TEST_CONTEXT(mat_strain0_)
    // if (hinges_)
    //   hinges_->takeAction(action, params, globdat);
    return true;
  }

  if (action == Actions::GET_TABLE)
  {
    Ref<XTable> table;
    Vector weights;
    String name;
    // Get the action-specific parameters.
    params.get(table, ActionParams::TABLE);
    params.get(weights, ActionParams::TABLE_WEIGHTS);
    params.get(name, ActionParams::TABLE_NAME);

    // Check whether the requested table is supported by
    // this model.
    if (table->getRowItems() == allElems_.getData())
    {
      Vector disp;
      StateVector::get(disp, dofs_, globdat);

      if (name == "strain")
        get_strain_table_(*table, weights, disp);
      else if (name == "stress")
        get_stress_table_(*table, weights, disp);
      else if (name == "mat_strain")
        get_strain_table_(*table, weights, disp, true);
      else if (name == "mat_stress")
        get_stress_table_(*table, weights, disp, true);
      else
        get_mat_table_(*table, weights, name);

      return true;
    }

    if (table->getRowItems() == allNodes_.getData())
    {
      Vector disp;
      StateVector::get(disp, dofs_, globdat);

      if (name == "potentialEnergy")
        calc_pot_Energy_(*table, weights, disp);
      if (name == "dissipatedEnergy")
        calc_diss_Energy_(*table, weights, disp);
    }
  }

  if (action == Actions::GET_MATRIX0)
  {
    Ref<MatrixBuilder> mbld;
    Vector fint;
    Vector disp;
    String loadCase = "";

    // Get the action-specific parameters.
    params.get(mbld, ActionParams::MATRIX0);
    params.get(fint, ActionParams::INT_VECTOR);
    // TEST_CONTEXT ( fint )

    // get the load case
    globdat.find(loadCase, jive::app::PropNames::LOAD_CASE);

    // Get the current displacements.
    StateVector::get(disp, dofs_, globdat);
    // TEST_CONTEXT( disp )

    // Assemble the global stiffness matrix together with
    // the internal vector.
    assemble_(*mbld, fint, disp, loadCase);

    // // DEBUGGING
    // IdxVector dofList(fint.size());
    // Matrix K(fint.size(), fint.size());
    // Matrix D(dofs_->typeCount(), allNodes_.size());
    // Matrix F(dofs_->typeCount(), allNodes_.size());
    // for (idx_t i = 0; i < dofList.size(); i++)
    //   dofList[i] = i;
    // mbld->getBlock(K, dofList, dofList);
    // vec2mat(D.transpose(), disp);
    // vec2mat(F.transpose(), fint);
    // REPORT(action)
    // TEST_CONTEXT(D)
    // TEST_CONTEXT(K)
    // TEST_CONTEXT(F)

    // if (hinges_)
    //   hinges_->takeAction(action, params, globdat);
    return true;
  }

  if (action == Actions::GET_MATRIX2)
  {
    Ref<MatrixBuilder> mbld;
    Vector disp;

    params.get(mbld, ActionParams::MATRIX2);
    StateVector::get(disp, dofs_, globdat);

    assembleM_(*mbld, disp);

    // // DEBUGGING
    // IdxVector dofList(dofs_->dofCount());
    // Matrix M(dofList.size(), dofList.size());
    // for (idx_t i = 0; i < dofList.size(); i++)
    //   dofList[i] = i;
    // mbld->getBlock(M, dofList, dofList);
    // REPORT(action)
    // TEST_CONTEXT(mbld->toString())
    // TEST_CONTEXT(M)

    return true;
  }

  if (action == Actions::GET_INT_VECTOR)
  {
    Vector fint;
    Vector disp;
    Vector velo;
    Ref<AbstractMatrix> mass;
    String loadCase = "";

    // Get the action-specific parameters.
    params.get(fint, ActionParams::INT_VECTOR);

    // get the load case
    globdat.find(loadCase, jive::app::PropNames::LOAD_CASE);

    // Get the current displacements.
    StateVector::get(disp, dofs_, globdat);

    // Assemble the global stiffness matrix together with
    // the internal vector.
    assemble_(fint, disp, loadCase);

    if (params.find(mass, ActionParams::MATRIX2))
    {
      StateVector::get(velo, jive::model::STATE1, dofs_, globdat);
      assembleGyro_(fint, velo, mass);
    }

    // //DEBUGGING
    // Matrix      F ( dofs_->typeCount(), nodes_.size() );
    // vec2mat( F.transpose(), fint );
    // TEST_CONTEXT ( F )

    // if (hinges_)
    //   hinges_->takeAction(action, params, globdat);
    return true;
  }

  if (action == Actions::COMMIT)
  {
    material_->apply_deform();

    Properties vars = Globdat::getVariables(globdat);
    Vector disp;
    Vector velo;
    double E_pot = 0.;
    double E_diss = 0.;

    vars.find(E_pot, "potentialEnergy");
    vars.find(E_diss, "dissipatedEnergy");

    StateVector::get(disp, dofs_, globdat);
    E_diss += calc_diss_Energy_(disp);
    E_pot += calc_pot_Energy_(disp);

    vars.set("potentialEnergy", E_pot);
    vars.set("dissipatedEnergy", E_diss);

    return true;
  }

  if (action == Actions::CANCEL)
  {
    material_->reject_deform();
    return true;
  }

  // if (hinges_)
  //   return hinges_->takeAction(action, params, globdat);
  // else
  return false;
}
//-----------------------------------------------------------------------
//   get_mat_table_ (plastic version)
//-----------------------------------------------------------------------
void specialCosseratRodModel::get_mat_table_

    (XTable &mat_table, const Vector &weights, const String &name)
{
  IdxVector icols(dofs_->typeCount());
  String dofName;

  // add all the dofs to the Table
  for (idx_t idof = 0; idof < dofs_->typeCount(); idof++)
  {
    dofName = dofs_->getTypeName(idof);
    if (idof < TRANS_DOF_COUNT)
      icols[idof] = mat_table.addColumn(
          "gamma_" + dofName[SliceFrom(dofName.size() - 1)]);
    else
      icols[idof] = mat_table.addColumn(
          "kappa_" + dofName[SliceFrom(dofName.size() - 1)]);
  }

  material_->getTable(name, mat_table, rodElems_.getIndices(), weights);
}
//-----------------------------------------------------------------------
//   get_strain_table_
//-----------------------------------------------------------------------
void specialCosseratRodModel::get_strain_table_

    (XTable &strain_table, const Vector &weights, const Vector &disp,
     const bool mat_vals)
{
  const idx_t elemCount = rodElems_.size();
  const idx_t nodeCount = shapeK_->nodeCount();
  const idx_t ipCount = shapeK_->ipointCount();
  String dofName = "";

  IdxVector icols(dofs_->typeCount());
  IdxVector inodes(nodeCount);

  Vector ipWeights(ipCount);
  Matrix Lambda_r(TRANS_DOF_COUNT, TRANS_DOF_COUNT);
  Matrix nodePhi_0(TRANS_DOF_COUNT, nodeCount);
  Matrix nodeU(TRANS_DOF_COUNT, nodeCount);
  Cubix nodeLambda(TRANS_DOF_COUNT, TRANS_DOF_COUNT, nodeCount);

  Matrix strain(TRANS_DOF_COUNT + ROT_DOF_COUNT, ipCount);

  // add all the dofs to the Table
  for (idx_t idof = 0; idof < dofs_->typeCount(); idof++)
  {
    dofName = dofs_->getTypeName(idof);
    if (idof < TRANS_DOF_COUNT)
      icols[idof] = strain_table.addColumn(
          "gamma_" + dofName[SliceFrom(dofName.size() - 1)]);
    else
      icols[idof] = strain_table.addColumn(
          "kappa_" + dofName[SliceFrom(dofName.size() - 1)]);
  }

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    idx_t ielem = rodElems_.getIndices()[ie];
    allElems_.getElemNodes(inodes, ielem);
    get_disps_(nodePhi_0, nodeU, nodeLambda, disp, inodes);

    get_strains_(strain, ipWeights, nodePhi_0, nodeU, nodeLambda, ie,
                 !mat_vals);

    for (idx_t ip = 0; ip < ipCount; ip++)
    {
      strain_table.addRowValues(ielem, icols, strain(ALL, ip));
      weights[ielem] += 1.;
    }
  }
}

//-----------------------------------------------------------------------
//   get_stress_table_
//-----------------------------------------------------------------------
void specialCosseratRodModel::get_stress_table_

    (XTable &stress_table, const Vector &weights, const Vector &disp,
     const bool mat_vals)
{
  const idx_t elemCount = rodElems_.size();
  const idx_t nodeCount = shapeK_->nodeCount();
  const idx_t ipCount = shapeK_->ipointCount();
  String dofName = "";

  IdxVector icols(dofs_->typeCount());
  IdxVector inodes(nodeCount);

  Vector ipWeights(ipCount);
  Matrix Lambda_r(TRANS_DOF_COUNT, TRANS_DOF_COUNT);
  Matrix nodePhi_0(TRANS_DOF_COUNT, nodeCount);
  Matrix nodeU(TRANS_DOF_COUNT, nodeCount);
  Cubix nodeLambda(TRANS_DOF_COUNT, TRANS_DOF_COUNT, nodeCount);

  Matrix stress(TRANS_DOF_COUNT + ROT_DOF_COUNT, ipCount);

  // add all the dofs to the Table
  for (idx_t idof = 0; idof < dofs_->typeCount(); idof++)
  {
    dofName = dofs_->getTypeName(idof);
    if (idof < TRANS_DOF_COUNT)
      icols[idof] = stress_table.addColumn(
          "n_" + dofName[SliceFrom(dofName.size() - 1)]);
    else
      icols[idof] = stress_table.addColumn(
          "m_" + dofName[SliceFrom(dofName.size() - 1)]);
  }

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    idx_t ielem = rodElems_.getIndex(ie);
    allElems_.getElemNodes(inodes, ielem);
    get_disps_(nodePhi_0, nodeU, nodeLambda, disp, inodes);

    get_stresses_(stress, ipWeights, nodePhi_0, nodeU, nodeLambda, ie,
                  !mat_vals);

    for (idx_t ip = 0; ip < ipCount; ip++)
    {
      stress_table.addRowValues(ielem, icols, stress(ALL, ip));
      weights[ielem] += 1.;
    }
  }
}

//-----------------------------------------------------------------------
//  init_strain_
//-----------------------------------------------------------------------
void specialCosseratRodModel::init_strain_()
{
  const idx_t rank = shapeK_->globalRank();
  const idx_t dofCount = dofs_->typeCount();
  const idx_t ipCount = shapeK_->ipointCount();
  const idx_t elemCount = rodElems_.size();
  const idx_t nodeCount = shapeK_->nodeCount();

  // PER ELEMENT VALUES
  Vector weights(ipCount);
  IdxVector ins(nodeCount);
  IdxVector inodes(nodeCount);
  Matrix coords(rank, nodeCount);
  // STRAINS
  Matrix strains(dofCount, ipCount);
  Matrix null_mat(rank, nodeCount);
  null_mat = 0.;

  mat_strain0_.resize(dofCount, ipCount, elemCount);
  mat_strain0_ = 0.;

  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    idx_t ielem = rodElems_.getIndex(ie);
    allElems_.getElemNodes(ins, ielem);
    allNodes_.getSomeCoords(coords, ins);
    // TEST_CONTEXT(coords)
    inodes = rodNodes_[ins];
    // TEST_CONTEXT((Cubix(LambdaN_[inodes])))
    get_strains_(strains, weights, coords, null_mat,
                 (Cubix)LambdaN_[inodes], ie, false);
    mat_strain0_[ie] = strains;
  }
}

//-----------------------------------------------------------------------
//   init_rot_
//-----------------------------------------------------------------------
void specialCosseratRodModel::init_rot_()
{
  const idx_t nodeCount = rodElems_.getNodeIndices().size();
  const idx_t elemCount = rodElems_.size();
  const idx_t elemNodes = shapeK_->nodeCount();
  const idx_t ipCount = shapeK_->ipointCount();
  IdxVector ins(elemNodes);
  IdxVector inodes(elemNodes);
  IdxVector allnodes = rodElems_.getNodeIndices();
  Matrix node_dirs(TRANS_DOF_COUNT, nodeCount);
  Matrix ip_dirs(TRANS_DOF_COUNT, ipCount);
  Matrix coords(TRANS_DOF_COUNT, allNodes_.size());
  Matrix rotMat(TRANS_DOF_COUNT, TRANS_DOF_COUNT);
  Matrix oldMat(TRANS_DOF_COUNT, TRANS_DOF_COUNT);
  Matrix newMat(TRANS_DOF_COUNT, TRANS_DOF_COUNT);
  Cubix nodeLambda(TRANS_DOF_COUNT, TRANS_DOF_COUNT, nodeCount);
  Vector turnRot(ROT_DOF_COUNT);
  Vector delta_phi(TRANS_DOF_COUNT);
  Vector v(TRANS_DOF_COUNT);
  Vector e_x(TRANS_DOF_COUNT);
  Vector e_y(TRANS_DOF_COUNT);
  Vector e_z(TRANS_DOF_COUNT);
  double c;

  LambdaN_.resize(TRANS_DOF_COUNT, TRANS_DOF_COUNT, nodeCount);
  LambdaN_ = NAN;
  node_dirs = 0.;

  allNodes_.getCoords(coords);

  // get the direction for each element //ONLY APPROXIMATION FOR
  // NON-STRAIGHT ELEMENTS
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    // REPORT(ie)
    idx_t ielem = rodElems_.getIndex(ie);
    allElems_.getElemNodes(ins, ielem);

    inodes = rodNodes_[ins];

    // TEST_CONTEXT(inodes)
    node_dirs(ALL, inodes[0]) +=
        coords(ALL, ins[1]) - coords(ALL, ins[0]);
    for (idx_t in = 1; in < elemNodes - 1; in++)
      node_dirs(ALL, inodes[in]) +=
          coords(ALL, ins[in + 1]) - coords(ALL, ins[in - 1]);
    node_dirs(ALL, inodes[elemNodes - 1]) +=
        coords(ALL, ins[elemNodes - 1]) - coords(ALL, ins[elemNodes - 2]);
  }

  for (idx_t igiven = 0; igiven < givenNodes_.size(); igiven++)
    node_dirs[rodNodes_[givenNodes_[igiven]]] = givenDirs_[igiven];

  for (idx_t in = 0; in < nodeCount; in++)
  {
    node_dirs[in] = node_dirs[in] / norm2(node_dirs[in]);
  }

  for (idx_t in = 0; in < nodeCount; in++)
  {
    if (material_ey_.size()) // if the y-direction is given, construct
                             // the z direction and then the x-direction
    {
      e_y = material_ey_;
      e_z = node_dirs[in];
      e_x = matmul(skew(e_y), e_z);

      rotMat[0] = e_x;
      rotMat[1] = e_y;
      rotMat[2] = e_z;
    }
    else // no y-direction given
    {
      v = matmul(e3, skew(node_dirs[in]));
      c = dotProduct(node_dirs[in], e3);

      rotMat = eye();
      if (c != -1.) // 180 deg turn == point mirroring
        rotMat += skew(v) + 1 / (1 + c) * matmul(skew(v), skew(v));
      else
        rotMat *= -1.;
    }
    LambdaN_[in] = rotMat;
  }
}

void specialCosseratRodModel::get_geomStiff_(const Cubix &B,
                                             const Matrix &stresses,
                                             const Matrix &nodePhi_0,
                                             const Matrix &nodeU) const
{
  const idx_t dofCount = dofs_->typeCount();
  const idx_t globRank = shapeK_->globalRank();
  const idx_t nodeCount = shapeK_->nodeCount();
  const idx_t ipCount = shapeK_->ipointCount();

  Vector w(ipCount);
  Matrix shapeGrads(nodeCount, ipCount);
  Matrix phiP(globRank, ipCount);

  // get phi_prime
  shapeK_->getShapeGradients(shapeGrads, w, nodePhi_0);
  phiP = matmul((Matrix)(nodePhi_0 + nodeU), shapeGrads);

  // for every iPoint assemble the B-Matrix
  for (idx_t ip = 0; ip < ipCount; ip++)
  {
    B[ip] = 0.;
    B[ip](TRANS_PART, SliceFrom(dofCount)) -=
        skew(stresses[ip][TRANS_PART]);
    B[ip](ROT_PART, SliceFrom(dofCount)) -= skew(stresses[ip][ROT_PART]);
    B[ip](SliceFrom(dofCount), TRANS_PART) +=
        skew(stresses[ip][TRANS_PART]);
    B[ip](SliceFrom(dofCount), SliceFrom(dofCount)) +=
        matmul(stresses[ip][TRANS_PART], phiP[ip]);
    B[ip](SliceFrom(dofCount), SliceFrom(dofCount)) -=
        dot(stresses[ip][TRANS_PART], phiP[ip]) * eye();
  }
}

void specialCosseratRodModel::get_strains_(
    const Matrix &strains, const Vector &w, const Matrix &nodePhi_0,
    const Matrix &nodeU, const Cubix &nodeLambda, const idx_t ie,
    const bool spatial) const
{
  // TEST_CONTEXT(nodePhi_0)
  // TEST_CONTEXT(nodeU)

  const idx_t ipCount = shapeK_->ipointCount();
  const idx_t globRank = shapeK_->globalRank();
  const idx_t dofCount = dofs_->typeCount();

  const Cubix ipLambda(globRank, globRank, ipCount);
  const Cubix ipPI(dofCount, dofCount, ipCount);
  const Cubix ipLambdaP(globRank, globRank, ipCount);
  const Matrix ipPhi(globRank, ipCount);
  const Matrix ipPhiP(globRank, ipCount);
  const IdxVector inodes(shapeK_->nodeCount());

  Matrix shapes(shapeK_->shapeFuncCount(), shapeK_->ipointCount());
  Matrix grads(shapeK_->shapeFuncCount(), shapeK_->ipointCount());

  shapes = shapeK_->getShapeFunctions();
  shapeK_->getShapeGradients(grads, w, nodePhi_0);
  // TEST_CONTEXT(shapes)
  // TEST_CONTEXT(grads)

  ipPhi = matmul((Matrix)(nodePhi_0 + nodeU), shapes);
  ipPhiP = matmul((Matrix)(nodePhi_0 + nodeU), grads);
  shapeK_->getPi(ipPI, ipLambda, nodeLambda);
  shapeK_->getRotationGradients(ipLambdaP, w, nodePhi_0, nodeLambda);

  // get the strains (material + spatial );
  for (idx_t ip = 0; ip < ipCount; ip++)
  {
    strains[ip][TRANS_PART] =
        matmul(ipLambda[ip].transpose(), ipPhiP[ip]);
    // TEST_CONTEXT(matmul(ipLambda[ip].transpose(), ipLambdaP[ip]))
    strains[ip][ROT_PART] =
        unskew(matmul(ipLambda[ip].transpose(), ipLambdaP[ip]));
  }

  // TEST_CONTEXT(strains)
  strains -= mat_strain0_[ie];
  // TEST_CONTEXT(strains)

  if (spatial)
    for (idx_t ip = 0; ip < ipCount; ip++)
      strains[ip] = matmul(ipPI[ip], strains[ip]);
  // TEST_CONTEXT(strains)
}

void specialCosseratRodModel::get_stresses_(
    const Matrix &stresses, const Vector &w, const Matrix &nodePhi_0,
    const Matrix &nodeU, const Cubix &nodeLambda, const idx_t ie,
    const bool spatial, const String &loadCase) const
{
  const idx_t ipCount = shapeK_->ipointCount();
  const idx_t dofCount = dofs_->typeCount();
  const Matrix strains(stresses.shape());
  const Cubix stiffness(dofCount, dofCount, ipCount);
  const Cubix PI(dofCount, dofCount, ipCount);

  // get the (material) strains
  get_strains_(strains, w, nodePhi_0, nodeU, nodeLambda, ie, false);
  // TEST_CONTEXT(strains)

  for (idx_t ip = 0; ip < ipCount; ip++)
    material_->getStress(stresses[ip], strains[ip], ie, ip, loadCase != "output");

  // get the (spatial) stresses
  if (spatial)
  {
    shapeK_->getPi(PI, nodeLambda);
    for (idx_t ip = 0; ip < ipCount; ip++)
      stresses[ip] = matmul(PI[ip], stresses[ip]);
  }
}

void specialCosseratRodModel::get_disps_(const Matrix &nodePhi_0,
                                         const Matrix &nodeU,
                                         const Cubix &nodeLambda,
                                         const Vector &disp,
                                         const IdxVector &inodes) const
{
  const idx_t nodeCount = inodes.size();

  IdxVector idofs_trans(TRANS_DOF_COUNT);
  IdxVector idofs_rot(ROT_DOF_COUNT);

  // TEST_CONTEXT(inodes)
  allNodes_.getSomeCoords(nodePhi_0, inodes);
  // TEST_CONTEXT(nodePhi_0)

  for (idx_t inode = 0; inode < nodeCount; inode++)
  {
    dofs_->getDofIndices(idofs_trans, inodes[inode], trans_types_);
    dofs_->getDofIndices(idofs_rot, inodes[inode], rot_types_);

    nodeU[inode] = disp[idofs_trans];
    expVec(nodeLambda[inode], (Vector)disp[idofs_rot]);
    nodeLambda[inode] =
        matmul(nodeLambda[inode], LambdaN_[rodNodes_[inodes[inode]]]);
  }
}

void specialCosseratRodModel::assemble_(MatrixBuilder &mbld,
                                        const Vector &fint,
                                        const Vector &disp,
                                        const String &loadCase) const
{
  const idx_t ipCount = shapeK_->ipointCount();
  const idx_t nodeCount = shapeK_->nodeCount();
  const idx_t elemCount = rodElems_.size();
  const idx_t dofCount = dofs_->typeCount();
  const idx_t rank = shapeK_->globalRank();
  MatmulChain<double, 3> mc3;

  // PER ELEMENT VALUES
  Matrix nodeU(rank, nodeCount);
  Matrix nodePhi_0(rank, nodeCount);
  Cubix nodeLambda(rank, rank, nodeCount);
  Matrix stress(dofCount, ipCount);
  Vector weights(ipCount);
  Quadix XI(dofCount, dofCount, nodeCount, ipCount);
  Quadix PSI(dofCount, dofCount + TRANS_DOF_COUNT, nodeCount, ipCount);
  Cubix PI(dofCount, dofCount, ipCount);
  Matrix spatialC(dofCount, dofCount);
  Cubix geomStiff(dofCount + TRANS_DOF_COUNT, dofCount + TRANS_DOF_COUNT,
                  ipCount);

  // DOF INDICES
  IdxVector inodes(nodeCount);
  IdxVector Idofs(dofCount);
  IdxVector Jdofs(dofCount);

  // HELPERS
  Matrix addS(dofCount, dofCount);
  Matrix addT(dofCount, dofCount);

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    // REPORT(ie)
    allElems_.getElemNodes(inodes, rodElems_.getIndex(ie));
    // TEST_CONTEXT( rodElems_.getIndex(ie) )
    // TEST_CONTEXT( inodes )
    // get nice Positions
    get_disps_(nodePhi_0, nodeU, nodeLambda, disp, inodes);

    // TEST_CONTEXT(inodes)
    // TEST_CONTEXT(nodePhi_0)
    // TEST_CONTEXT(nodeU)
    // TEST_CONTEXT(nodeLambda)

    // get the XI, PSI and PI values for this
    shapeK_->getXi(XI, weights, nodeU, nodePhi_0);
    shapeK_->getPsi(PSI, weights, nodePhi_0);
    shapeK_->getPi(PI, nodeLambda);
    // TEST_CONTEXT(XI)
    // TEST_CONTEXT(PSI)
    // TEST_CONTEXT(PI)
    // get the (spatial) stresses
    get_stresses_(stress, weights, nodePhi_0, nodeU, nodeLambda, ie, true, loadCase);
    // get the gemetric stiffness
    get_geomStiff_(geomStiff, stress, nodePhi_0, nodeU);
    // TEST_CONTEXT(geomStiff)

    // iterate through the integration Points
    for (idx_t ip = 0; ip < ipCount; ip++)
    {
      // get the spatial stiffness
      spatialC = mc3.matmul(PI[ip], material_->getMaterialStiff(ie, ip), PI[ip].transpose());
      // TEST_CONTEXT(PI[ip])
      // TEST_CONTEXT(material_->getMaterialStiff(ie, ip))

      for (idx_t Inode = 0; Inode < nodeCount; Inode++)
      {
        dofs_->getDofIndices(Idofs, inodes[Inode], jtypes_);

        for (idx_t Jnode = 0; Jnode < nodeCount; Jnode++)
        {
          // SUBHEADER2( Inode, Jnode)
          dofs_->getDofIndices(Jdofs, inodes[Jnode], jtypes_);

          // Stiffness contribution S ( element stiffness matrix )
          addS = weights[ip] * mc3.matmul(XI[ip][Inode], spatialC,
                                          XI[ip][Jnode].transpose());
          // TEST_CONTEXT(addS)
          mbld.addBlock(Idofs, Jdofs, addS);

          // Stiffness contribution T ( element geometric
          // stiffness matrix)
          addT = weights[ip] * mc3.matmul(PSI[ip][Inode], geomStiff[ip],
                                          PSI[ip][Jnode].transpose());
          // TEST_CONTEXT(addT)
          if (!symmetric_only_)
            mbld.addBlock(Idofs, Jdofs, addT);
        }
        fint[Idofs] += weights[ip] * matmul(XI[ip][Inode], stress[ip]);
      }
    }
  }
}

void specialCosseratRodModel::assemble_(const Vector &fint,
                                        const Vector &disp,
                                        const String &loadCase) const
{
  const idx_t ipCount = shapeK_->ipointCount();
  const idx_t nodeCount = shapeK_->nodeCount();
  const idx_t elemCount = rodElems_.size();
  const idx_t dofCount = dofs_->typeCount();
  const idx_t rank = shapeK_->globalRank();
  MatmulChain<double, 3> mc3;

  // PER ELEMENT VALUES
  Matrix nodeU(rank, nodeCount);
  Matrix nodePhi_0(rank, nodeCount);
  Cubix nodeLambda(rank, rank, nodeCount);
  Matrix stress(dofCount, ipCount);
  Vector weights(ipCount);
  Quadix XI(dofCount, dofCount, nodeCount, ipCount);
  // DOF INDICES
  IdxVector inodes(nodeCount);
  IdxVector Idofs(dofCount);

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    allElems_.getElemNodes(inodes, rodElems_.getIndex(ie));
    // get the nice positions
    get_disps_(nodePhi_0, nodeU, nodeLambda, disp, inodes);

    // TEST_CONTEXT(inodes)
    // TEST_CONTEXT(nodePhi_0)
    // TEST_CONTEXT(nodeU)
    // TEST_CONTEXT(nodeLambda)

    // get the XI values for this
    shapeK_->getXi(XI, weights, nodeU, nodePhi_0);
    // get the (spatial) stresses
    get_stresses_(stress, weights, nodePhi_0, nodeU, nodeLambda, ie, true, loadCase);

    // iterate through the integration Points
    for (idx_t ip = 0; ip < ipCount; ip++)
    {
      for (idx_t Inode = 0; Inode < nodeCount; Inode++)
      {
        dofs_->getDofIndices(Idofs, inodes[Inode], jtypes_);

        fint[Idofs] += weights[ip] * matmul(XI[ip][Inode], stress[ip]);
      }
    }
  }
}

void specialCosseratRodModel::assembleGyro_(const Vector &fgyro,
                                            const Vector &velo,
                                            const Ref<AbstractMatrix> mass) const
{
  IdxVector idofs(ROT_DOF_COUNT);
  Matrix Theta(ROT_DOF_COUNT, ROT_DOF_COUNT);
  Vector temp(velo.size());

  mass->matmul(temp, velo);

  for (idx_t inode : rodElems_.getNodeIndices())
  {
    dofs_->getDofIndices(idofs, inode, rot_types_);

    fgyro[idofs] += matmul(skew((Vector)velo[idofs]), (Vector)temp[idofs]);
  }
}

void specialCosseratRodModel::assembleM_(MatrixBuilder &mbld, Vector &disp) const
{
  // REPORT(JEM_FUNC)
  MatmulChain<double, 3> mc3;

  const idx_t dofCount = dofs_->typeCount();
  const idx_t nodeCount = shapeM_->nodeCount();
  const idx_t elemCount = rodElems_.size();
  const idx_t rank = shapeM_->globalRank();
  const idx_t ipCount = shapeM_->ipointCount();

  // PER ELEMENT VALUES
  IdxVector inodes(nodeCount);
  IdxVector idofs(dofCount);
  IdxVector jdofs(dofCount);

  Vector weights(ipCount);
  Matrix shapes(nodeCount, ipCount);
  double l;
  Matrix nodePhi_0(rank, nodeCount);
  Matrix nodeU(rank, nodeCount);
  Cubix nodeLambda(rank, rank, nodeCount);

  Cubix ipLambda(rank, rank, ipCount);

  Matrix spatialInertia(dofCount, dofCount);

  // iterate through the elements
  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    allElems_.getElemNodes(inodes, rodElems_.getIndex(ie));
    get_disps_(nodePhi_0, nodeU, nodeLambda, disp, inodes);
    shapeM_->getRotations(ipLambda, nodeLambda);

    shapeM_->getIntegrationWeights(weights, nodePhi_0);
    shapes = shapeM_->getShapeFunctions();

    l = sum(weights) / (nodeCount - 1);

    for (idx_t inode = 0; inode < nodeCount; inode++)
    {
      dofs_->getDofIndices(idofs, inodes[inode], jtypes_);
      for (idx_t jnode = 0; jnode < nodeCount; jnode++)
      {
        dofs_->getDofIndices(jdofs, inodes[jnode], jtypes_);

        spatialInertia = 0;

        for (idx_t ip = 0; ip < ipCount; ip++)
          spatialInertia(TRANS_PART, TRANS_PART) += weights[ip] * shapes(inode, ip) * shapes(jnode, ip) *
                                                    mc3.matmul(ipLambda[ip], material_->getMaterialMass(ie, ip)(TRANS_PART, TRANS_PART), ipLambda[ip].transpose());
        if (inode == jnode)
          spatialInertia(ROT_PART, ROT_PART) += mc3.matmul(nodeLambda[inode], material_->getLumpedMass(l, ie)(ROT_PART, ROT_PART),
                                                           nodeLambda[inode].transpose());
        if ((inode == jnode) && (inode == 0 || inode == nodeCount - 1))
          spatialInertia(ROT_PART, ROT_PART) /= 2.;

        // TEST_CONTEXT(spatialInertia)
        mbld.addBlock(idofs, idofs, spatialInertia);
      }
    }
  }
}

void specialCosseratRodModel::calc_pot_Energy_(XTable &energy_table, const Vector &table_weights, const Vector &disp) const
{
  const idx_t elemCount = rodElems_.size();
  const idx_t ipCount = shapeK_->ipointCount();
  const idx_t nodeCount = shapeK_->nodeCount();
  const idx_t rank = shapeK_->globalRank();
  const idx_t dofCount = dofs_->typeCount();
  const idx_t jCol = energy_table.addColumn("potentialEnergy");

  // PER ELEMENT VALUES
  Matrix nodeU(rank, nodeCount);
  Matrix nodePhi_0(rank, nodeCount);
  Cubix nodeLambda(rank, rank, nodeCount);
  Matrix strain(dofCount, ipCount);
  Matrix stress(dofCount, ipCount);
  Vector weights(ipCount);
  Matrix shapes(shapeK_->shapeFuncCount(), ipCount);
  // DOF INDICES
  IdxVector inodes(nodeCount);

  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    allElems_.getElemNodes(inodes, rodElems_.getIndex(ie));
    get_disps_(nodePhi_0, nodeU, nodeLambda, disp, inodes);

    get_strains_(strain, weights, nodePhi_0, nodeU, nodeLambda, ie, false);
    get_stresses_(stress, weights, nodePhi_0, nodeU, nodeLambda, ie, false, "output");

    shapes = shapeK_->getShapeFunctions();

    for (idx_t iNode = 0; iNode < nodeCount; iNode++)
    {
      for (idx_t ip = 0; ip < ipCount; ip++)
      {
        energy_table.addValue(inodes[iNode], jCol, shapes(iNode, ip) * weights[ip] * material_->getPotentialEnergy(ie, ip));
        table_weights[inodes[iNode]] = 1.;
      }
    }
  }
}

double specialCosseratRodModel::calc_pot_Energy_(const Vector &disp) const
{
  const idx_t elemCount = rodElems_.size();
  const idx_t ipCount = shapeK_->ipointCount();
  const idx_t nodeCount = shapeK_->nodeCount();
  const idx_t rank = shapeK_->globalRank();
  const idx_t dofCount = dofs_->typeCount();
  double E_pot = 0.;

  // PER ELEMENT VALUES
  Matrix nodeU(rank, nodeCount);
  Matrix nodePhi_0(rank, nodeCount);
  Cubix nodeLambda(rank, rank, nodeCount);
  Matrix strain(dofCount, ipCount);
  Matrix stress(dofCount, ipCount);
  Vector weights(ipCount);
  Matrix shapes(shapeK_->shapeFuncCount(), ipCount);
  // DOF INDICES
  IdxVector inodes(nodeCount);

  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    allElems_.getElemNodes(inodes, rodElems_.getIndex(ie));
    get_disps_(nodePhi_0, nodeU, nodeLambda, disp, inodes);

    get_strains_(strain, weights, nodePhi_0, nodeU, nodeLambda, ie, false);
    get_stresses_(stress, weights, nodePhi_0, nodeU, nodeLambda, ie, false, "output");

    shapes = shapeK_->getShapeFunctions();

    for (idx_t iNode = 0; iNode < nodeCount; iNode++)
    {
      for (idx_t ip = 0; ip < ipCount; ip++)
      {
        E_pot += weights[ip] * shapes(iNode, ip) * material_->getPotentialEnergy(ie, ip);
      }
    }
  }

  return E_pot;
}

void specialCosseratRodModel::calc_diss_Energy_(XTable &energy_table, const Vector &table_weights, const Vector &disp) const
{
  const idx_t elemCount = rodElems_.size();
  const idx_t ipCount = shapeK_->ipointCount();
  const idx_t nodeCount = shapeK_->nodeCount();
  const idx_t rank = shapeK_->globalRank();
  const idx_t dofCount = dofs_->typeCount();
  const idx_t jCol = energy_table.addColumn("potentialEnergy");

  // PER ELEMENT VALUES
  Matrix nodeU(rank, nodeCount);
  Matrix nodePhi_0(rank, nodeCount);
  Cubix nodeLambda(rank, rank, nodeCount);
  Matrix strain(dofCount, ipCount);
  Matrix stress(dofCount, ipCount);
  Vector weights(ipCount);
  Matrix shapes(shapeK_->shapeFuncCount(), ipCount);
  // DOF INDICES
  IdxVector inodes(nodeCount);

  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    allElems_.getElemNodes(inodes, rodElems_.getIndex(ie));
    get_disps_(nodePhi_0, nodeU, nodeLambda, disp, inodes);

    get_strains_(strain, weights, nodePhi_0, nodeU, nodeLambda, ie, false);
    get_stresses_(stress, weights, nodePhi_0, nodeU, nodeLambda, ie, false, "output");

    shapes = shapeK_->getShapeFunctions();

    for (idx_t iNode = 0; iNode < nodeCount; iNode++)
    {
      for (idx_t ip = 0; ip < ipCount; ip++)
      {
        energy_table.addValue(inodes[iNode], jCol, shapes(iNode, ip) * weights[ip] * material_->getDissipatedEnergy(ie, ip));
        table_weights[inodes[iNode]] = 1.;
      }
    }
  }
}

double specialCosseratRodModel::calc_diss_Energy_(const Vector &disp) const
{
  const idx_t elemCount = rodElems_.size();
  const idx_t ipCount = shapeK_->ipointCount();
  const idx_t nodeCount = shapeK_->nodeCount();
  const idx_t rank = shapeK_->globalRank();
  const idx_t dofCount = dofs_->typeCount();
  double E_diss = 0.;

  // PER ELEMENT VALUES
  Matrix nodeU(rank, nodeCount);
  Matrix nodePhi_0(rank, nodeCount);
  Cubix nodeLambda(rank, rank, nodeCount);
  Matrix strain(dofCount, ipCount);
  Matrix stress(dofCount, ipCount);
  Vector weights(ipCount);
  Matrix shapes(shapeK_->shapeFuncCount(), ipCount);
  // DOF INDICES
  IdxVector inodes(nodeCount);

  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    allElems_.getElemNodes(inodes, rodElems_.getIndex(ie));
    get_disps_(nodePhi_0, nodeU, nodeLambda, disp, inodes);

    get_strains_(strain, weights, nodePhi_0, nodeU, nodeLambda, ie, false);
    get_stresses_(stress, weights, nodePhi_0, nodeU, nodeLambda, ie, false, "output");

    shapes = shapeK_->getShapeFunctions();

    for (idx_t iNode = 0; iNode < nodeCount; iNode++)
    {
      for (idx_t ip = 0; ip < ipCount; ip++)
      {
        E_diss += weights[ip] * shapes(iNode, ip) * material_->getDissipatedEnergy(ie, ip);
      }
    }
  }

  return E_diss;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Model> specialCosseratRodModel::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<specialCosseratRodModel>(name, conf, props, globdat);
}

//-----------------------------------------------------------------------
//   declarespecialCosseratRodModel
//-----------------------------------------------------------------------

// Registers the specialCosseratRodModel class with the ModelFactory.

void specialCosseratRodModel::declare()
{
  using jive::model::ModelFactory;

  ModelFactory::declare(TYPE_NAME, &makeNew);
}
