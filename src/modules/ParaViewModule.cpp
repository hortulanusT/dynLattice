#include "ParaViewModule.h"
#include "utils/testing.h"

void vec2mat(const Matrix &mat, const Vector &vec)
{
  const idx_t rows = mat.size(0);
  const idx_t cols = mat.size(1);
  JEM_ASSERT2(rows * cols == vec.size(),
              "Vector and Matrix not of the same size!");

  for (idx_t irow = 0; irow < rows; irow++)
  {
    mat(irow, ALL) = vec[SliceFromTo(irow * cols, (irow + 1) * cols)];
  }
}

//=======================================================================
//   class ParaViewModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *ParaViewModule::TYPE_NAME = "ParaView";
const char *ParaViewModule::SPACING = "    ";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------

ParaViewModule::ParaViewModule

    (const String &name)
    : Module(name)

{
  // default Values for internal Variables
  nameFormat_ = "para_%d";
  fileType_ = "vtu";
  elemSets_.resize(0);
  setInfo_.resize(0);
}

//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------

Module::Status ParaViewModule::init

    (const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  myProps.find(nameFormat_, "output_format");
  myConf.set("output_format", nameFormat_);

  myProps.find(fileType_, "output_file");
  myConf.set("output_file", fileType_);

  sampleCond_ = jive::util::FuncUtils::newCond();
  jive::util::FuncUtils::configCond(
      sampleCond_, jive::app::PropNames::SAMPLE_COND, myProps, globdat);
  jive::util::FuncUtils::getConfig(myConf, sampleCond_,
                                   jive::app::PropNames::SAMPLE_COND);

  myProps.get(elemSets_, "groups");
  myConf.set("groups", elemSets_);

  setInfo_.resize(elemSets_.size());

  for (idx_t igroup = 0; igroup < elemSets_.size(); igroup++)
  {
    Properties groupProps = myProps.getProps(elemSets_[igroup]);
    Properties groupConf = myConf.makeProps(elemSets_[igroup]);

    setInfo_[igroup].name = elemSets_[igroup];

    groupProps.get(setInfo_[igroup].shape, "shape");
    nameToVTKNum(setInfo_[igroup].shape);
    groupConf.set("shape", setInfo_[igroup].shape);

    groupProps.get(setInfo_[igroup].dispData, "disps");
    groupConf.set("disps", setInfo_[igroup].dispData);

    if (groupProps.find(setInfo_[igroup].dofData, "otherDofs"))
      groupConf.set("otherDofs", setInfo_[igroup].dofData);

    groupProps.find(setInfo_[igroup].elemData, "el_data");
    groupConf.set("el_data", setInfo_[igroup].elemData);

    groupProps.find(setInfo_[igroup].nodeData, "node_data");
    groupConf.set("node_data", setInfo_[igroup].nodeData);
  }

  // construct the file folder, if it does not exist
  std::filesystem::path paraFolder = makeCString(nameFormat_).addr();
  paraFolder.remove_filename();
  std::filesystem::create_directories(paraFolder);

  // write the pvd file
  idx_t format_pos = nameFormat_.rfind("%i");
  pvd_print_ = format_pos > 0;
  pvd_time_buffer_.resize(0);
  pvd_name_buffer_.resize(0);
  if (pvd_print_)
    pvd_name_ = nameFormat_[SliceTo(format_pos)] +
                nameFormat_[SliceFrom(format_pos + 2)] + ".pvd";

  out_num_ = 0;

  return OK;
}

//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------

Module::Status ParaViewModule::run

    (const Properties &globdat)

{
  idx_t currentStep;
  double currentTime;
  String currentFile;
  idx_t folder_sep;

  // write everything to file
  bool cond = jive::util::FuncUtils::evalCond(*sampleCond_, globdat);

  if (cond)
  {
    // get the current timeStep and format the given format accordingly
    currentFile = String::format(makeCString(nameFormat_).addr(), out_num_++);
    currentFile = currentFile + "." + fileType_;
    writeFile_(currentFile, globdat);

    // report file to pvd
    globdat.get(currentStep, Globdat::TIME_STEP);
    if (!globdat.find(currentTime, Globdat::TIME))
      currentTime = currentStep;
    folder_sep =
        currentFile.rfind(std::filesystem::path::preferred_separator) + 1;

    if (pvd_print_)
    {
      pvd_time_buffer_.pushBack(currentTime);
      pvd_name_buffer_.pushBack(currentFile[SliceFrom(folder_sep)]);

      Ref<Writer> pvd_raw = newInstance<FileWriter>(pvd_name_);
      Ref<PrintWriter> pvd_printer = newInstance<PrintWriter>(pvd_raw);

      *pvd_printer << "<?xml version=\"1.0\"?>" << endl;
      *pvd_printer << "<VTKFile type=\"Collection\" version=\"0.1\" "
                      "byte_order=\"LittleEndian\">"
                   << endl;
      pvd_printer->incrIndentLevel();
      *pvd_printer << "<Collection>" << endl;
      pvd_printer->incrIndentLevel();
      pvd_printer->flush();

      for (idx_t i_out = 0; i_out < out_num_; i_out++)
      {
        *pvd_printer << "<DataSet ";
        *pvd_printer << "timestep=\"" << pvd_time_buffer_[i_out] << "\" ";
        *pvd_printer << "group=\"\" part=\"0\" ";
        *pvd_printer << "file=\"" << pvd_name_buffer_[i_out] << "\" ";
        *pvd_printer << "/>" << endl;
        pvd_printer->flush();
      }

      pvd_printer->decrIndentLevel();
      *pvd_printer << "</Collection>" << endl;
      pvd_printer->decrIndentLevel();
      *pvd_printer << "</VTKFile>" << endl;

      pvd_printer->close();
    }
  }

  return OK;
}

//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------

void ParaViewModule::shutdown

    (const Properties &globdat)
{
}

//-----------------------------------------------------------------------
//   writeFile_
//-----------------------------------------------------------------------
void ParaViewModule::writeFile_

    (const String &fileName, const Properties &globdat)

{
  Ref<Writer> file_raw = newInstance<FileWriter>(fileName);
  Ref<PrintWriter> file_frmt = newInstance<PrintWriter>(file_raw);

  // Output Formatting
  file_frmt->nformat.setScientific(true);
  file_frmt->nformat.setFractionDigits(8);
  file_frmt->nformat.setFloatWidth(
      15); // 1sign + 1digit + 1decimal + 8digits + 1e + 1sign + 2digits
  file_frmt->setIndentWidth(2);

  // Write Header
  // see http://www.vtk.org/wp-content/uploads/2015/04/file-formats.pdf
  *file_frmt << "<?xml version=\"1.0\"?>" << endl;
  *file_frmt << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\">"
             << endl;
  file_frmt->incrIndentLevel();
  *file_frmt << "<UnstructuredGrid>" << endl;
  file_frmt->incrIndentLevel();

  // iterate over all the groups
  for (idx_t igroup = 0; igroup < elemSets_.size(); igroup++)
  {
    // get the current standings elements and corresponding nodes
    Assignable<ElementSet> elems = ElementSet::get(globdat, getContext());
    Assignable<ElementGroup> egroup = ElementGroup::get(
        elemSets_[igroup], elems, globdat, getContext());
    Assignable<NodeSet> nodes = elems.getNodes();

    // get the current displacements
    Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());
    Vector disp, velo, acce;
    StateVector::get(disp, jive::model::STATE0, dofs, globdat);
    if (!StateVector::find(velo, jive::model::STATE1, dofs, globdat))
    {
      velo.resize(disp.shape());
      velo = 0.0;
    }
    if (!StateVector::find(acce, jive::model::STATE2, dofs, globdat))
    {
      acce.resize(disp.shape());
      acce = 0.0;
    }

    // get the current model
    Ref<Model> model = Model::get(globdat, getContext());

    writePiece_(file_frmt, nodes, elems, egroup, disp, velo, acce, dofs,
                model, globdat, setInfo_[igroup]);
  }

  file_frmt->decrIndentLevel();
  *file_frmt << "</UnstructuredGrid>" << endl;
  file_frmt->decrIndentLevel();
  *file_frmt << "</VTKFile>";

  // close the stream
  file_frmt->close();
}

//-----------------------------------------------------------------------
//   writePiece_
//-----------------------------------------------------------------------

void ParaViewModule::writePiece_

    (const Ref<PrintWriter> &file, const Assignable<NodeSet> &points,
     const Assignable<ElementSet> &cells,
     const Assignable<ElementGroup> &group, const Vector &disp,
     const Vector &velo, const Vector &acce, const Ref<DofSpace> &dofs,
     const Ref<Model> &model, const Properties &globdat,
     const elInfo &info)

{
  IdxVector groupNodes = group.getNodeIndices();
  IdxVector nodeNums(max(groupNodes) + 1);
  IdxVector groupElems = group.getIndices();

  nodeNums = -1;

  // TEST_CONTEXT(groupNodes)
  // TEST_CONTEXT(nodeNums)
  // TEST_CONTEXT(groupElems)

  *file << "<Piece "
        << "NumberOfPoints=\"" << groupNodes.size() << "\" "
        << "NumberOfCells=\"" << group.size() << "\""
        << ">" << endl;
  file->incrIndentLevel();

  // Write the points to the file
  *file << "<Points>" << endl;
  file->incrIndentLevel();
  *file << "<DataArray type=\"Float32\" NumberOfComponents=\"" << 3
        << "\">" << endl;
  file->incrIndentLevel();
  for (idx_t inode = 0; inode < groupNodes.size(); inode++)
  {
    Vector coords(points.rank());
    nodeNums[groupNodes[inode]] = inode;
    points.getNodeCoords(coords, groupNodes[inode]);
    *file << coords[0] << SPACING
          << (points.rank() >= 2 ? coords[1] : 0.0) << SPACING
          << (points.rank() >= 3 ? coords[2] : 0.0) << SPACING << endl;
  }
  file->decrIndentLevel();
  *file << "</DataArray>" << endl;
  file->decrIndentLevel();
  *file << "</Points>" << endl;

  // Write the elements to the file
  IdxVector offsets(group.size());
  IdxVector types(group.size());
  *file << "<Cells>" << endl;
  file->incrIndentLevel();

  *file << "<DataArray type=\"Int32\" Name=\"connectivity\">" << endl;
  file->incrIndentLevel();
  // iterate through the elements
  for (idx_t ie = 0; ie < group.size(); ie++)
  {
    idx_t ielem = group.getIndex(ie);

    IdxVector elNodes(cells.getElemNodeCount(ielem));
    cells.getElemNodes(elNodes, ielem);

    IdxVector paraNodes = gmsh2ParaNodeOrder(elNodes, info.shape);

    offsets[ie] = elNodes.size();
    types[ie] = nameToVTKNum(info.shape);

    for (idx_t inode = 0; inode < elNodes.size(); inode++)
    {
      *file << nodeNums[paraNodes[inode]] << SPACING;
    }
    *file << endl;
  }
  file->decrIndentLevel();
  *file << "</DataArray>" << endl;

  *file << "<DataArray type=\"Int32\" Name=\"offsets\">" << endl;
  file->incrIndentLevel();
  for (idx_t ielem = 0; ielem < group.size(); ielem++)
  {
    *file << sum(offsets[SliceFromTo(0, ielem + 1)]) << endl;
  }
  file->decrIndentLevel();
  *file << "</DataArray>" << endl;

  *file << "<DataArray type=\"UInt8\" Name=\"types\">" << endl;
  file->incrIndentLevel();
  for (idx_t ielem = 0; ielem < group.size(); ielem++)
  {
    *file << types[ielem] << endl;
  }
  file->decrIndentLevel();
  *file << "</DataArray>" << endl;

  file->decrIndentLevel();
  *file << "</Cells>" << endl;

  // Write the pointdata to the file
  *file << "<PointData Vectors=\"Displacement\">" << endl;
  file->incrIndentLevel();

  // Start by writing Displacements
  IdxVector iDofs(info.dispData.size());
  IdxVector iDisps(info.dispData.size());
  Matrix disp_mat(groupNodes.size(), info.dispData.size());
  Matrix velo_mat(groupNodes.size(), info.dispData.size());
  Matrix acce_mat(groupNodes.size(), info.dispData.size());

  for (idx_t idof = 0; idof < iDisps.size(); idof++)
    iDofs[idof] = dofs->findType(info.dispData[idof]);

  for (idx_t ipoint = 0; ipoint < groupNodes.size(); ipoint++)
  {
    dofs->getDofIndices(iDisps, groupNodes[ipoint], iDofs);
    disp_mat(ipoint, ALL) = disp[iDisps];
    velo_mat(ipoint, ALL) = velo[iDisps];
    acce_mat(ipoint, ALL) = acce[iDisps];
  }

  writeDataArray_(file, disp_mat, "Float32", "Displacement");
  writeDataArray_(file, velo_mat, "Float32", "Velocity");
  writeDataArray_(file, acce_mat, "Float32", "Acceleration");

  // Next write other Dofs
  if (info.dofData.size() > 0)
  {
    iDofs.resize(info.dofData.size());
    iDisps.resize(info.dofData.size());
    disp_mat.resize(groupNodes.size(), info.dofData.size());
    velo_mat.resize(groupNodes.size(), info.dofData.size());
    acce_mat.resize(groupNodes.size(), info.dofData.size());

    for (idx_t idof = 0; idof < iDisps.size(); idof++)
    {
      iDofs[idof] = dofs->findType(info.dofData[idof]);
    }

    for (idx_t ipoint = 0; ipoint < groupNodes.size(); ipoint++)
    {
      dofs->getDofIndices(iDisps, groupNodes[ipoint], iDofs);
      disp_mat(ipoint, ALL) = disp[iDisps];
      velo_mat(ipoint, ALL) = velo[iDisps];
      acce_mat(ipoint, ALL) = acce[iDisps];
    }

    writeDataArray_(file, disp_mat, "Float32", "otherDofsDisp");
    writeDataArray_(file, velo_mat, "Float32", "otherDofsVelo");
    writeDataArray_(file, acce_mat, "Float32", "otherDofsAcc");
  }

  // iterate through all other data ( see OutputModule for more advanced
  // features )

  for (idx_t iPtDatum = 0; iPtDatum < info.nodeData.size(); iPtDatum++)
  {
    using jive::model::ActionParams;
    Properties params("actionParams");

    StringVector dofNames = dofs->getTypeNames();
    iDofs.resize(dofNames.size());
    iDisps.resize(dofNames.size());
    for (idx_t idof = 0; idof < dofNames.size(); idof++)
    {
      iDofs[idof] = dofs->findType(dofNames[idof]);
    }

    if (info.nodeData[iPtDatum] == "fext")
    {
      Vector fext(dofs->dofCount());
      Matrix fext_mat(points.size(), dofs->typeCount());
      fext = 0.;

      params.set(ActionParams::EXT_VECTOR, fext);
      model->takeAction(Actions::GET_EXT_VECTOR, params, globdat);
      params.erase(ActionParams::EXT_VECTOR);

      for (idx_t ipoint = 0; ipoint < groupNodes.size(); ipoint++)
      {
        dofs->getDofIndices(iDisps, groupNodes[ipoint], iDofs);
        fext_mat(ipoint, ALL) = fext[iDisps];
      }
      writeDataArray_(file, fext_mat(ALL, SliceTo(3)), "Float32",
                      "External Forces");
      writeDataArray_(file, fext_mat(ALL, SliceFrom(3)), "Float32",
                      "External Torques");
    }
    else if (info.nodeData[iPtDatum] == "fint")
    {
      Vector fint(dofs->dofCount());
      Matrix fint_mat(points.size(), dofs->typeCount());
      fint = 0.;

      params.set(ActionParams::INT_VECTOR, fint);
      model->takeAction(Actions::GET_INT_VECTOR, params, globdat);
      params.erase(ActionParams::INT_VECTOR);

      for (idx_t ipoint = 0; ipoint < groupNodes.size(); ipoint++)
      {
        dofs->getDofIndices(iDisps, groupNodes[ipoint], iDofs);
        fint_mat(ipoint, ALL) = fint[iDisps];
      }
      writeDataArray_(file, fint_mat(ALL, SliceTo(3)), "Float32",
                      "Internal Forces");
      writeDataArray_(file, fint_mat(ALL, SliceFrom(3)), "Float32",
                      "Internal Torques");
    }
    else if (info.nodeData[iPtDatum] == "fres")
    {
      Vector fext(dofs->dofCount());
      Vector fint(dofs->dofCount());
      Vector fres(dofs->dofCount());
      Matrix fres_mat(points.size(), dofs->typeCount());
      fext = 0.;
      fint = 0.;
      fres = 0.;

      params.set(ActionParams::EXT_VECTOR, fext);
      model->takeAction(Actions::GET_EXT_VECTOR, params, globdat);
      params.erase(ActionParams::EXT_VECTOR);

      params.set(ActionParams::INT_VECTOR, fint);
      model->takeAction(Actions::GET_INT_VECTOR, params, globdat);
      params.erase(ActionParams::INT_VECTOR);

      fres = fext - fint;

      for (idx_t ipoint = 0; ipoint < groupNodes.size(); ipoint++)
      {
        dofs->getDofIndices(iDisps, groupNodes[ipoint], iDofs);
        fres_mat(ipoint, ALL) = fres[iDisps];
      }
      writeDataArray_(file, fres_mat(ALL, SliceTo(3)), "Float32",
                      "Resulting Forces");
      writeDataArray_(file, fres_mat(ALL, SliceFrom(3)), "Float32",
                      "Resulting Torques");
    }
    else
    {
      Ref<ItemSet> pointSet =
          ItemSet::get("nodes", globdat, getContext());

      Ref<XTable> datumTable =
          newInstance<SparseTable>("paraView", pointSet);
      Vector weights(datumTable->rowCount());

      weights = 0.;

      params.set(ActionParams::TABLE, datumTable);
      params.set(ActionParams::TABLE_NAME, info.nodeData[iPtDatum]);
      params.set(ActionParams::TABLE_WEIGHTS, weights);

      model->takeAction(Actions::GET_TABLE, params, globdat);

      params.erase(ActionParams::TABLE_NAME);
      params.erase(ActionParams::TABLE);
      params.erase(ActionParams::TABLE_WEIGHTS);

      datumTable->scaleRows(weights);

      writeDataArray_(file, datumTable, groupNodes, "Float32",
                      info.nodeData[iPtDatum]);
    }
  }

  file->decrIndentLevel();
  *file << "</PointData>" << endl;

  // Write the cell data to the file
  *file << "<CellData>" << endl;
  file->incrIndentLevel();

  // iterate through all desired cell (element) data ( see OutputModule
  // for more advanced features )

  for (idx_t iElDatum = 0; iElDatum < info.elemData.size(); iElDatum++)
  {
    using jive::model::ActionParams;

    Ref<ItemSet> cellSet =
        ItemSet::get("elements", globdat, getContext());

    Ref<XTable> datumTable =
        newInstance<SparseTable>(info.elemData[iElDatum], cellSet);
    // TEST_CONTEXT(datumTable->rowCount())
    Vector weights(datumTable->rowCount());
    Properties params("actionParams");

    weights = 0.;
    // REPORT(info.elemData[iElDatum])

    params.set(ActionParams::TABLE_NAME, info.elemData[iElDatum]);
    params.set(ActionParams::TABLE, datumTable);
    params.set(ActionParams::TABLE_WEIGHTS, weights);

    model->takeAction(Actions::GET_TABLE, params, globdat);

    params.erase(ActionParams::TABLE_NAME);
    params.erase(ActionParams::TABLE);
    params.erase(ActionParams::TABLE_WEIGHTS);

    datumTable->scaleRows(weights);

    writeDataArray_(file, datumTable, groupElems, "Float32",
                    info.elemData[iElDatum]);
  }

  file->decrIndentLevel();
  *file << "</CellData>" << endl;

  file->decrIndentLevel();
  *file << "</Piece>" << endl;
}

//-----------------------------------------------------------------------
//   writeDataArray
//-----------------------------------------------------------------------

void ParaViewModule::writeDataArray_

    (const Ref<PrintWriter> &file, const Matrix &data, const String &type,
     const String &name)
{
  *file << "<DataArray type=\"" << type << "\" Name=\"" << name
        << "\" NumberOfComponents=\"" << data.shape()[1] << "\">" << endl;
  file->incrIndentLevel();

  for (idx_t iRow = 0; iRow < data.shape()[0]; iRow++)
  {
    for (idx_t iColumn = 0; iColumn < data.shape()[1]; iColumn++)
    {
      *file << (float)data(iRow, iColumn)
            << SPACING; // float since ParaView can only read single
                        // precision floats
    }
    *file << endl;
  }

  file->decrIndentLevel();
  *file << "</DataArray>" << endl;
}

void ParaViewModule::writeDataArray_

    (const Ref<PrintWriter> &file, const Ref<XTable> &data,
     const IdxVector &rows, const String &type, const String &name)
{
  const idx_t icolumns = data->columnCount();
  const idx_t irows = data->rowCount();
  Matrix mat(irows, icolumns);

  data->findAllValues(mat);
  writeDataArray_(file, Matrix(mat.transpose()[rows]).transpose(), type,
                  name);
}

void ParaViewModule::writeDataArray_

    (const Ref<PrintWriter> &file, const Vector &data, const String &type,
     const String &name)
{
  Matrix mat(data.size(), 1);

  mat(ALL, 0) = data;

  writeDataArray_(file, mat, type, name);
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> ParaViewModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<ParaViewModule>(name);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void ParaViewModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &makeNew);
}

//-----------------------------------------------------------------------
//   nameToVTKNum
//-----------------------------------------------------------------------

idx_t ParaViewModule::nameToVTKNum

    (const String &name)
{
  // https://vtk.org/doc/nightly/html/vtkCellType_8h_source.html
  idx_t cellType = 0;

  // 1D elements
  if (name == "Line2")
  {
    cellType = 3;
  }
  else if (name == "Line3")
  {
    cellType = 21;
  }
  else if (name == "Line4")
  {
    cellType = 35;
  }
  // 2D elements
  else if (name == "Quad4")
  {
    cellType = 9;
  }
  else if (name == "Quad8")
  {
    cellType = 23;
  }
  else if (name == "Triangle3")
  {
    cellType = 5;
  }
  else if (name == "Triangle6")
  {
    cellType = 22;
  }
  // 3D elements
  else if (name == "Hex8")
  {
    cellType = 12;
  }
  else if (name == "Hex20")
  {
    cellType = 25;
  }
  else if (name == "Tet4")
  {
    cellType = 10;
  }
  else if (name == "Tet10")
  {
    cellType = 24;
  }
  else
  {
    throw IllegalArgumentException("ParaViewModule",
                                   "Model type " + name + " unkown");
  }

  return cellType;
}

//-----------------------------------------------------------------------
//   gmsh2ParaNodeOrder
//-----------------------------------------------------------------------

IdxVector ParaViewModule::gmsh2ParaNodeOrder

    (const IdxVector elNodes, const String &name)
{
  IdxVector paraViewNodes(elNodes.size());

  // paraview :
  // http://www.vtk.org/wp-content/uploads/2015/04/file-formats.pdf --->
  // pages 9 and 10 gmsh     : http://gmsh.info/doc/texinfo/gmsh.html --->
  // section 9.3

  /*
  1D elements


          Line2:                  Line3:
  Gmsh:
      0----------1 --> u      0-----2-----1

  ParaView:

      0----------1 --> u      0-----1-----2

  */

  if (name == "Line2")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[1];
  }
  else if (name == "Line3")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[2];
    paraViewNodes[2] = elNodes[1];
  }
  else if (name == "Line4")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[3];
    paraViewNodes[2] = elNodes[1];
    paraViewNodes[3] = elNodes[2];
  }

  /*
  2D elements


  Quadrangle:            Quadrangle8:            Quadrangle9:
  Gmsh:
        v
        ^
        |
  3-----------2          3-----6-----2           3-----6-----2
  |     |     |          |           |           |           |
  |     |     |          |           |           |           |
  |     +---- | --> u    7           5           7     8     5
  |           |          |           |           |           |
  |           |          |           |           |           |
  0-----------1          0-----4-----1           0-----4-----1

  ParaView:
        v
        ^
        |
  3-----------2          6-----5-----4           6-----5-----4
  |     |     |          |           |           |           |
  |     |     |          |           |           |           |
  |     +---- | --> u    7           3           7     8     3
  |           |          |           |           |           |
  |           |          |           |           |           |
  0-----------1          0-----1-----2           0-----1-----2
  */
  else if (name == "Quad4")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[1];
    paraViewNodes[2] = elNodes[2];
    paraViewNodes[3] = elNodes[3];
  }
  else if (name == "Quad8")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[2];
    paraViewNodes[2] = elNodes[4];
    paraViewNodes[3] = elNodes[6];
    paraViewNodes[4] = elNodes[1];
    paraViewNodes[5] = elNodes[3];
    paraViewNodes[6] = elNodes[5];
    paraViewNodes[7] = elNodes[7];
  }
  else if (name == "Quad9")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[2];
    paraViewNodes[2] = elNodes[4];
    paraViewNodes[3] = elNodes[6];
    paraViewNodes[4] = elNodes[1];
    paraViewNodes[5] = elNodes[3];
    paraViewNodes[6] = elNodes[5];
    paraViewNodes[7] = elNodes[7];
    paraViewNodes[8] = elNodes[8];
  }

  /*
  Triangle:               Triangle6:          Triangle9/10: Triangle12/15:

  Gmsh:

  v
  ^                                                                   2
  |                                                                   | \
  2                       2                    2                      9 8
  |`\                     |`\                  | \                    | \
  |  `\                   |  `\                7   6                 10
  (14)  7
  |    `\                 5    `4              |     \                | \
  |      `\               |      `\            8  (9)  5             11
  (12) (13) 6
  |        `\             |        `\          |         \            | \
  0----------1 --> u      0-----3----1         0---3---4---1
  0---3---4---5---1

  ParaView:

  v
  ^                                                                   8
  |                                                                   | \
  2                       4                    6                      9 7
  |`\                     |`\                  | \                    | \
  |  `\                   |  `\                7   5                 10
  (14)  6
  |    `\                 5    `3              |     \                | \
  |      `\               |      `\            8  (9)  4             11
  (12) (13) 5
  |        `\             |        `\          |         \            | \
  0----------1 --> u      0-----1----2         0---1---2---3
  0---1---2---3---4

  */
  else if (name == "Triangle3")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[1];
    paraViewNodes[2] = elNodes[2];
  }
  else if (name == "Triangle6")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[2];
    paraViewNodes[2] = elNodes[4];
    paraViewNodes[3] = elNodes[1];
    paraViewNodes[4] = elNodes[3];
    paraViewNodes[5] = elNodes[5];
  }

  /*

  3D elements

  Hexahedron8:            Hexahedron20:          Hexahedron27:

  Gmsh:
          v
  3----------2            3----13----2           3----13----2
  |\     ^   |\           |\         |\          |\         |\
  | \    |   | \          | 15       | 14        |15    24  | 14
  |  \   |   |  \         9  \       11 \        9  \ 20    11 \
  |   7------+---6        |   7----19+---6       |   7----19+---6
  |   |  +-- |-- | -> u   |   |      |   |       |22 |  26  | 23|
  0---+---\--1   |        0---+-8----1   |       0---+-8----1   |
    \  |    \  \  |         \  17      \  18       \ 17    25 \  18
    \ |     \  \ |         10 |        12|        10 |  21    12|
      \|      w  \|           \|         \|          \|         \|
      4----------5            4----16----5           4----16----5

  ParaView:


  */

  else if (name == "Hex8")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[1];
    paraViewNodes[2] = elNodes[3];
    paraViewNodes[3] = elNodes[2];
    paraViewNodes[4] = elNodes[4];
    paraViewNodes[5] = elNodes[5];
    paraViewNodes[6] = elNodes[7];
    paraViewNodes[7] = elNodes[6];
  }
  else if (name == "Hex20")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[2];
    paraViewNodes[2] = elNodes[4];
    paraViewNodes[3] = elNodes[6];
    paraViewNodes[4] = elNodes[12];
    paraViewNodes[5] = elNodes[14];
    paraViewNodes[6] = elNodes[16];
    paraViewNodes[7] = elNodes[18];
    paraViewNodes[8] = elNodes[1];
    paraViewNodes[9] = elNodes[3];
    paraViewNodes[10] = elNodes[5];
    paraViewNodes[11] = elNodes[7];
    paraViewNodes[12] = elNodes[13];
    paraViewNodes[13] = elNodes[15];
    paraViewNodes[14] = elNodes[17];
    paraViewNodes[15] = elNodes[19];
    paraViewNodes[16] = elNodes[8];
    paraViewNodes[17] = elNodes[9];
    paraViewNodes[18] = elNodes[10];
    paraViewNodes[19] = elNodes[11];
  }

  /*

        Tetrahedron4:                         Tetrahedron10:

  Gmsh:
                      v
                    .
                  ,/
                /
              2                                     2
            / | \                                 / | \
          ,/  |  `\                             ,/  |  `\
        ,/    '.   `\                         ,6    '.   `5
      ,/       |     `\                     ,/       8     `\
    ,/         |       `\                 ,/         |       `\
  0-----------'.--------1 --> u         0--------4--'.--------1
    `\.         |      ,/                 `\.         |      ,/
      `\.      |    ,/                      `\.      |    ,9
          `\.   '. ,/                           `7.   '. ,/
            `\. |/                                `\. |/
                `3                                    `3
                  `\.
                      ` w

  ParaView:

  */

  else if (name == "Tet4")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[1];
    paraViewNodes[2] = elNodes[2];
    paraViewNodes[3] = elNodes[3];
  }
  else if (name == "Tet10")
  {
    paraViewNodes[0] = elNodes[0];
    paraViewNodes[1] = elNodes[1];
    paraViewNodes[2] = elNodes[2];
    paraViewNodes[3] = elNodes[3];
    paraViewNodes[4] = elNodes[4];
    paraViewNodes[5] = elNodes[5];
    paraViewNodes[6] = elNodes[6];
    paraViewNodes[7] = elNodes[8]; // here
    paraViewNodes[8] = elNodes[7]; // here
  }
  else
  {
    throw IllegalArgumentException("Model type " + name + " unkown");
  }

  return paraViewNodes;
}
