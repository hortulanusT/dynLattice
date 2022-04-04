#include "GMSHInputModule.h"

#include "utils/testing.h" //HACK remove me

//=======================================================================
//   class GMSHInputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------


const char*   GMSHInputModule::TYPE_NAME  = "GMSHInput";
const char*   GMSHInputModule::GEO_FILE   = "geo_file";
const char*   GMSHInputModule::ORDER      = "order";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------


GMSHInputModule::GMSHInputModule ( const String& name ) :

  Super ( name )

{}


GMSHInputModule::~GMSHInputModule ()
{}


//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------


Module::Status GMSHInputModule::init

  ( const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )
{  
  String      geoFile = "";
  idx_t       order   = 1;

  Properties  myProps = props.findProps ( myName_ );
  Properties  myConf  = conf .makeProps ( myName_ );

  myProps.get ( geoFile, GEO_FILE );
  myConf .set ( GEO_FILE, geoFile );

  myProps.find( order, ORDER );
  myConf .set ( ORDER, order );

  gmsh::initialize();
  gmsh::open ( makeCString(geoFile).addr() );
  
  createMesh_ ( order, globdat );

  gmsh::finalize();

  return EXIT;
}


//-----------------------------------------------------------------------
//   run
//-----------------------------------------------------------------------


Module::Status GMSHInputModule::run ( const Properties& globdat )
{
  return OK;
}


//-----------------------------------------------------------------------
//   shutdown
//-----------------------------------------------------------------------


void GMSHInputModule::shutdown ( const Properties& globdat )
{}


//-----------------------------------------------------------------------
//   createMesh_
//-----------------------------------------------------------------------


void GMSHInputModule::createMesh_

  ( const idx_t       order,
    const Properties& globdat ) const

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized before calling the function to store the mesh in the global database");

  std::vector<std::size_t> nodeTags;
  std::vector<double> coord;
  std::vector<double> parametricCoord;

  gmsh::vectorpair entities;

  std::vector<int> elementTypes;
  std::vector<std::vector<std::size_t> > elementTags;
  std::vector<std::vector<std::size_t> > elementNodeTags;

  Vector  node_coords ( 3 ); 

  Assignable<XNodeSet>    globalNodes = XNodeSet::find ( globdat );
  if ( !globalNodes ) 
  { 
    globalNodes = jive::fem::newXNodeSet();
    globalNodes.store( globdat );
  }
  Assignable<XElementSet> globalElements = XElementSet::find ( globdat );
  if ( !globalElements ) 
  { 
    globalElements = jive::fem::newXElementSet ( globalNodes );
    globalElements.store( globdat );
  }
  IdxBuffer entity_elems;

  std::string name;
  int dim, elOrder, numNodes, numPrimaryNodes;
  std::vector<double> localCoords;

  // generate the mesh and read out the nodes
  gmsh::model::mesh::generate ( 1 ); // TODO make somewhat adaptive?
  gmsh::model::mesh::setOrder ( order );
  gmsh::model::mesh::getNodes ( nodeTags, coord, parametricCoord );
  // store all the Nodes in the global database
  for (idx_t iNode = 0; iNode < (idx_t)nodeTags.size(); iNode++)
  {
    node_coords = { coord[iNode*3], coord[iNode*3+1], coord[iNode*3+2] };
    globalNodes.addNode(nodeTags[iNode], node_coords);
  }

  gmsh::model::getEntities( entities ); // TODO get the max generate order from the dims in the entities!  

  for (idx_t i = 0; i < entities.size(); i++)
  {  
    entity_elems.clear();
    gmsh::model::mesh::getElements(elementTypes, elementTags, elementNodeTags, entities[i].first, entities[i].second);  
    JEM_PRECHECK(elementTypes.size() == 1); // entities should only contain one element type!
    gmsh::model::mesh::getElementProperties( elementTypes[0], name, dim, elOrder, numNodes, localCoords, numPrimaryNodes );
    IdxVector elemNodes (numNodes);
    
    for (idx_t iElem = 0; iElem < (idx_t)elementTags[0].size(); iElem++)
    {
      for (idx_t inode = 0; inode < numNodes; inode++)
      {
        elemNodes[inode] = elementNodeTags[0][iElem*numNodes+inode]-1; // TODO reordering for higher order elements?
      }
      entity_elems.pushBack( globalElements.addElement( elementTags[0][iElem], elemNodes ));
    }    

    ElementGroup egroup = jive::fem::newElementGroup( entity_elems.toArray(), globalElements );
    egroup.store( String(name.c_str()) + i, globdat );
  }
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------


Ref<Module> GMSHInputModule::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  return jem::newInstance<Self> ( name );
}


//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------


void GMSHInputModule::declare ()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare ( TYPE_NAME,  & makeNew );
}