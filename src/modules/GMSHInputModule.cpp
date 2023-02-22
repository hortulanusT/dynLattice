// Possible Issues:
// - Conversion of node numbering between GMSH and JIVE
//

#include "GMSHInputModule.h"

//=======================================================================
//   class GMSHInputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *GMSHInputModule::TYPE_NAME = "GMSHInput";
const char *GMSHInputModule::ORDER = "order";
const char *GMSHInputModule::MESH_DIM = "mesh_dim";
const char *GMSHInputModule::SAVE_MSH = "mesh_file";
const char *GMSHInputModule::STORE_TANGENTS = "store_tangents";
const char *GMSHInputModule::ENTITY_NAMES[4] = {"point", "beam", "shell",
                                                "body"};
const char *GMSHInputModule::ONELAB_PROPS = "onelab";
const char *GMSHInputModule::VERBOSE = "verbose";
const char *GMSHInputModule::OUT_FILE = "out_file";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

GMSHInputModule::GMSHInputModule(const String &name)
    :

      Super(name)

{
  verbose_ = true;
  writeOutput_ = false;
}

GMSHInputModule::~GMSHInputModule()
{
}

//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------

Module::Status GMSHInputModule::init

    (const Properties &conf, const Properties &props,
     const Properties &globdat)
{
  String geoFile = "";
  idx_t order = 1;
  idx_t dim = 3;
  bool storeTan = false;
  String saveMsh;
  Properties onelab;

  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.makeProps(myName_);

  // READ OUT THE PROPERTIES
  myProps.find(geoFile, jive::app::PropNames::FILE);
  myConf.set(jive::app::PropNames::FILE, geoFile);

  myProps.find(order, ORDER);
  myConf.set(ORDER, order);

  myProps.find(dim, MESH_DIM, 1, 3);
  myConf.set(MESH_DIM, dim);

  myProps.find(storeTan, STORE_TANGENTS);
  myConf.set(STORE_TANGENTS, storeTan);

  myProps.find(onelab, ONELAB_PROPS);
  myConf.set(ONELAB_PROPS, onelab);

  myProps.find(verbose_, VERBOSE);
  myConf.set(VERBOSE, verbose_);

  writeOutput_ = myProps.find(outFile_, OUT_FILE);
  if (writeOutput_)
  {
    myConf.set(OUT_FILE, outFile_);
    sampleCond_ = jive::util::FuncUtils::newCond();
    jive::util::FuncUtils::configCond(
        sampleCond_, jive::app::PropNames::SAMPLE_COND, myProps, globdat);
    jive::util::FuncUtils::getConfig(myConf, sampleCond_,
                                     jive::app::PropNames::SAMPLE_COND);
  }

  // TRY GETTING THE GLOBAL ELEMENTS
  nodes_ = XNodeSet::find(globdat);
  elements_ = XElementSet::find(globdat);

  if (!nodes_)
  {
    nodes_ = jive::fem::newXNodeSet();
    nodes_.store(globdat);
  }
  if (!elements_)
  {
    elements_ = jive::fem::newXElementSet(nodes_);
    elements_.store(globdat);
  }

  // HANDLE GMSH
  gmsh::initialize();
  gmsh::option::setNumber("General.Verbosity",
                          2); // 2 corresponds to warning level

  if (geoFile.size())
  {
    prepareOnelab_(onelab);
    openMesh_(geoFile, order);
  }
  else
  {
    // LATER Leon's function
  }

  if (myProps.find(saveMsh, SAVE_MSH))
  {
    if (saveMsh.find(".") < 0)
      saveMsh = saveMsh + ".msh22";
    gmsh::write(makeCString(saveMsh).addr());
    myConf.set(SAVE_MSH, saveMsh);
  }

  createNodes_(dim);
  createElems_(globdat);

  if (storeTan)
    storeTangents_(globdat);

  if (writeOutput_)
  {
    outView_ = gmsh::view::add("myView");
    return OK;
  }
  else
  {
    gmsh::finalize();
    return DONE;
  }
}

//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------

Module::Status GMSHInputModule::run(const Properties &globdat)
{
  if (writeOutput_)
  {
    if (jive::util::FuncUtils::evalCond(*sampleCond_, globdat))
      writeOutFile_(globdat);
    return OK;
  }
  else
  {
    gmsh::finalize();
    return DONE;
  }
}

//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------

void GMSHInputModule::shutdown(const Properties &globdat)
{
  if (writeOutput_)
  {
    JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");
    if (jive::util::FuncUtils::evalCond(*sampleCond_, globdat))
      writeOutFile_(globdat);
    gmsh::finalize();
  }
}

//-----------------------------------------------------------------------
//   openMesh_
//-----------------------------------------------------------------------

void GMSHInputModule::openMesh_

    (const String &geoFile, const idx_t order)

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");
  gmsh::vectorpair gmsh_entities;

  gmsh::open(makeCString(geoFile).addr());
  gmsh::model::getEntities(gmsh_entities);

  entities_.resize(2, gmsh_entities.size());
  for (idx_t i = 0; i < entities_.size(1); i++)
    entities_[i] = {gmsh_entities[i].first, gmsh_entities[i].second};

  gmsh::model::mesh::generate(max(entities_(0, ALL)));
  gmsh::model::mesh::setOrder(order);
}

//-----------------------------------------------------------------------
//   prepareOnelab_
//-----------------------------------------------------------------------

void GMSHInputModule::prepareOnelab_

    (const Properties &onelabProps)

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");
  auto onelabSettings = onelabProps.getContents();
  auto onelabEnumerator = onelabSettings->getDictEnum();

  String onelabKey = "";
  std::vector<double> onelabVal{0.};

  if (verbose_)
    jem::System::info(myName_) << "\n";
  while (!onelabEnumerator->atEnd())
  {
    onelabKey = onelabEnumerator->getKey();
    onelabProps.get(onelabVal[0], onelabKey);

    gmsh::onelab::setNumber(makeCString(onelabKey).addr(), onelabVal);

    if (verbose_)
      jem::System::info(myName_)
          << " ...Set GMSH variable '" << onelabKey << "' to a value of "
          << onelabVal[0] << "\n";

    onelabEnumerator->toNext();
  }
  if (verbose_)
    jem::System::info(myName_) << "\n";
}

//-----------------------------------------------------------------------
//   createNodes_
//-----------------------------------------------------------------------

void GMSHInputModule::createNodes_

    (const idx_t dim)

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");

  std::vector<std::size_t> gmshNodeTags;
  std::vector<double> gmsh_coords;
  std::vector<double> gmsh_paraCoords;

  Vector coords(dim);

  gmsh::model::mesh::getNodes(gmshNodeTags, gmsh_coords, gmsh_paraCoords);

  for (idx_t inode = 0; inode < (idx_t)gmshNodeTags.size(); inode++)
  {
    for (idx_t icoord = 0; icoord < dim; icoord++)
      coords[icoord] = gmsh_coords[inode * dim + icoord];

    gmshToJiveNodeMap_[gmshNodeTags[inode]] = nodes_.addNode(coords);

    if (verbose_)
      jem::System::info(myName_)
          << " ...Created node " << gmshToJiveNodeMap_[gmshNodeTags[inode]] << " at coordinates " << coords << "\n";
  }
  if (verbose_)
    jem::System::info(myName_) << "\n";
}

//-----------------------------------------------------------------------
//   createElems_
//-----------------------------------------------------------------------

void GMSHInputModule::createElems_

    (const Properties &globdat)

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");

  std::vector<int> types;
  std::vector<std::vector<std::size_t>> elemTags;
  std::vector<std::vector<std::size_t>> nodeTags;

  std::string elemName;
  int dim, order, numNodes, numPrimaryNodes;
  std::vector<double> localCoords;

  idx_t addedElem;

  IdxBuffer groupElems;
  IdxVector elemNodes;

  Assignable<ElementGroup> elementGroup;
  String groupName;

  Array<IdxBuffer> entityBuffer(4);
  IdxVector entityNumbering(4);

  entityNumbering = 0;
  for (idx_t i = 0; i < 4; i++)
    entityBuffer[i].clear();

  for (idx_t ientity = 0; ientity < entities_.size(); ientity++)
  {
    gmsh::model::mesh::getElements(types, elemTags, nodeTags,
                                   entities_[ientity][0],
                                   entities_[ientity][1]);

    groupName = String(ENTITY_NAMES[entities_[ientity][0]]) +
                String('_') +
                String(++entityNumbering[entities_[ientity][0]]);
    groupElems.clear();

    for (size_t itype = 0; itype < types.size(); itype++)
    {
      gmsh::model::mesh::getElementProperties(
          types[itype], elemName, dim, order, numNodes, localCoords,
          numPrimaryNodes);

      for (size_t ielem = 0; ielem < elemTags[itype].size(); ielem++)
      {
        elemNodes.resize(numNodes);

        for (idx_t inode = 0; inode < numPrimaryNodes; inode++)
        {
          elemNodes[inode * order] = gmshToJiveNodeMap_[nodeTags[itype][ielem * numNodes + inode]];

          if (inode * order + 1 == numNodes)
            break;
          for (idx_t jnode = 1; jnode < order; jnode++)
          {
            elemNodes[inode * order + jnode] = gmshToJiveNodeMap_[nodeTags[itype][ielem * numNodes + numPrimaryNodes + inode * (order - 1) + jnode - 1]];
          }
        }

        addedElem = elements_.addElement(elemNodes);
        if (verbose_)
          jem::System::info(myName_)
              << " ...Created element " << addedElem << " with nodes "
              << elemNodes << "\n";

        groupElems.pushBack(addedElem);
        entityBuffer[entities_[ientity][0]].pushBack(addedElem);
      }
    }

    elementGroup = jive::fem::newElementGroup(
        groupElems.toArray().clone(), elements_);
    elementGroup.store(groupName, globdat);
    if (verbose_)
      jem::System::info(myName_)
          << " ...Created element group for geometry entity '"
          << groupName << "'\n";
  }
  if (verbose_)
    jem::System::info(myName_) << "\n";

  // store all the super element groups
  for (idx_t i = 0; i < 4; i++)
  {
    elementGroup =
        jive::fem::newElementGroup(entityBuffer[i].toArray(), elements_);
    if (elementGroup.size() > 0)
    {
      elementGroup.store(String(ENTITY_NAMES[i]) + String('s'), globdat);

      if (verbose_)
        jem::System::info(myName_)
            << " ...Created element group for geometry entities of type '"
            << String(ENTITY_NAMES[i]) + String('s') << "'\n";
    }
  }
  if (verbose_)
    jem::System::info(myName_) << "\n";
}

//-----------------------------------------------------------------------
//   storeTangents_
//-----------------------------------------------------------------------

void GMSHInputModule::storeTangents_

    (const Properties &globdat)

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");

  std::vector<std::size_t> gmsh_tags;
  std::vector<double> gmsh_coords;
  std::vector<double> gmsh_localCoords;
  std::vector<double> gmsh_paras;
  std::vector<double> gmsh_derivatives;
  idx_t ibeam = 0;

  Properties tangentVars =
      jive::util::Globdat::getVariables("tangents", globdat);
  Properties entityVars;

  IdxVector jive_tags;
  Vector jive_derivatives;

  for (idx_t ientity = 0; ientity < entities_.size(); ientity++)
  {
    if (entities_[ientity][0] != 1)
      continue;

    entityVars = tangentVars.makeProps(String(ENTITY_NAMES[1]) +
                                       String("_") + String(++ibeam));

    gmsh::model::mesh::getNodes(gmsh_tags, gmsh_coords, gmsh_localCoords,
                                entities_[ientity][0],
                                entities_[ientity][1], true);
    gmsh::model::getParametrization(entities_[ientity][0],
                                    entities_[ientity][1], gmsh_coords,
                                    gmsh_paras);
    gmsh::model::getDerivative(entities_[ientity][0],
                               entities_[ientity][1], gmsh_paras,
                               gmsh_derivatives);

    jive_tags.resize(gmsh_tags.size());
    jive_derivatives.resize(3 * gmsh_tags.size());

    for (idx_t inode = 0; inode < (idx_t)gmsh_tags.size(); inode++)
    {
      jive_tags[inode] = gmshToJiveNodeMap_[gmsh_tags[inode]];

      for (idx_t icoord = 0; icoord < 3; icoord++)
        jive_derivatives[inode * 3 + icoord] =
            gmsh_derivatives[inode * 3 + icoord];
    }

    entityVars.set("given_dir_nodes", jive_tags.clone());
    entityVars.set("given_dir_dirs", jive_derivatives.clone());

    if (verbose_)
      jem::System::info(myName_) << " ...Stored derivatives in '"
                                 << entityVars.getName() << "'\n";
  }
}

//-----------------------------------------------------------------------
//   writeOutFile_
//-----------------------------------------------------------------------

void GMSHInputModule::writeOutFile_

    (const Properties &globdat) const
{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");

  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());
  Vector disp;
  IdxVector jtypes = {0, 1, 2};
  IdxVector idofs(3);
  idx_t step;
  double time = 0.0;
  std::string modelName;
  std::vector<std::size_t> gmshNodes;
  std::vector<std::vector<double>> gmshData;
  Vector nodeData(3);

  StateVector::get(disp, dofs, globdat);
  globdat.get(step, Globdat::TIME_STEP);
  globdat.find(time, Globdat::TIME);
  gmsh::model::getCurrent(modelName);

  for (const std::pair<const std::size_t, idx_t> &node : gmshToJiveNodeMap_)
  {
    dofs->getDofIndices(idofs, node.second, jtypes);
    nodeData = disp[idofs];
    gmshData.push_back(std::vector<double>(nodeData.begin(), nodeData.end()));
    gmshNodes.push_back(node.first);
  }

  gmsh::view::addModelData(outView_, step, modelName, "NodeData", gmshNodes, gmshData, time, 3);
  gmsh::view::write(outView_, makeCString(outFile_).addr());
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> GMSHInputModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return jem::newInstance<Self>(name);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void GMSHInputModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &makeNew);
}