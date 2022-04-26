#include "GMSHInputModule.h"

//=======================================================================
//   class GMSHInputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------


const char*   GMSHInputModule::TYPE_NAME        = "GMSHInput";
const char*   GMSHInputModule::ORDER            = "order";
const char*   GMSHInputModule::MESH_DIM         = "mesh_dim";
const char*   GMSHInputModule::ENTITY_NAMES[4]  = { "point", "beam", "shell", "body" };
const char*   GMSHInputModule::ONELAB_PROPS     = "onelab";

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
  idx_t       dim     = 3;
  Properties  onelab;

  Properties  myProps = props.findProps ( myName_ );
  Properties  myConf  = conf .makeProps ( myName_ );

  // READ OUT THE PROPERTIES
  myProps.find( geoFile, jive::app::PropNames::FILE );
  myConf .set ( jive::app::PropNames::FILE, geoFile );

  myProps.find( order, ORDER );
  myConf .set ( ORDER, order );

  myProps.find( dim, MESH_DIM, 1, 3 );
  myConf .set ( MESH_DIM, dim );

  myProps.find( onelab, ONELAB_PROPS );
  myConf. set ( ONELAB_PROPS, onelab );

  // TRY GETTING THE GLOBAL ELEMENTS
  nodes_    = XNodeSet::find( globdat );
  elements_ = XElementSet::find( globdat );
  
  if ( !nodes_ )
  {
    nodes_ = jive::fem::newXNodeSet();
    nodes_.store( globdat );
  }
  if ( !elements_ )
  {
    elements_ = jive::fem::newXElementSet( nodes_ );
    elements_.store( globdat );
  }

  // HANDLE GMSH
  gmsh::initialize(); 
  gmsh::option::setNumber( "General.Verbosity", 2 ); // 2 corresponds to warning level

  if ( geoFile.size() )
  {
    String mshFile = geoFile[jem::SliceTo(geoFile.size()-4)] + ".msh22";

    prepareOnelab_ ( onelab );
    openMesh_ ( geoFile, order ); 
    gmsh::write( makeCString(mshFile).addr() );
  }
  else
  {
    // TODO Leon's function
  }

  createNodes_ ( dim );
  createElems_ ( globdat ); 

  gmsh::finalize();

  // TODO record stored mesh

  return DONE;
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
//   openMesh_
//-----------------------------------------------------------------------


void GMSHInputModule::openMesh_

  ( const String&     geoFile,
    const idx_t       order )

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");
  gmsh::vectorpair gmsh_entities;

  gmsh::open( makeCString(geoFile).addr() );
  gmsh::model::getEntities( gmsh_entities );   

  entities_.resize( 2, gmsh_entities.size() );
  for (idx_t i = 0; i < entities_.size(1); i++) entities_[i] = { gmsh_entities[i].first, gmsh_entities[i].second };

  gmsh::model::mesh::generate( max( entities_(0, ALL) ) );  
  gmsh::model::mesh::setOrder( order );
}


//-----------------------------------------------------------------------
//   prepareOnelab_
//-----------------------------------------------------------------------


void GMSHInputModule::prepareOnelab_

  ( const Properties&   onelabProps )

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");
  auto onelabSettings = onelabProps.getContents();
  auto onelabEnumerator = onelabSettings->getDictEnum();

  String              onelabKey = "";
  std::vector<double> onelabVal { 0. };

  jem::System::info( myName_ ) << "\n";
  while ( !onelabEnumerator->atEnd() )
  {
    onelabKey = onelabEnumerator->getKey();
    onelabProps.get( onelabVal[0], onelabKey);

    gmsh::onelab::setNumber( makeCString(onelabKey).addr(), onelabVal );

    jem::System::info( myName_ ) << " ...Set GMSH variable '" << onelabKey << "' to a value of " << onelabVal[0] << "\n";
  
    onelabEnumerator->toNext();
  }
  jem::System::info( myName_ ) << "\n";
}

//-----------------------------------------------------------------------
//   createNodes_
//-----------------------------------------------------------------------


void GMSHInputModule::createNodes_ 

  ( const idx_t         dim )

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");

  std::vector<std::size_t>  gmsh_tags;
  std::vector<double>       gmsh_coords;
  std::vector<double>       gmsh_paraCoords;

  Vector                    coords ( dim );

  gmsh::model::mesh::getNodes ( gmsh_tags, gmsh_coords, gmsh_paraCoords);

  for (size_t inode = 0; inode < gmsh_tags.size(); inode++)
  {
    for (idx_t icoord = 0; icoord < dim; icoord++) 
      coords[icoord]  = gmsh_coords[inode*dim + icoord];
    
    jem::System::info( myName_ ) << " ...Created node " << nodes_.addNode( coords ) << " at coordinates " << coords << "\n";
  }
  jem::System::info( myName_ ) << "\n";
}

//-----------------------------------------------------------------------
//   createElems_
//-----------------------------------------------------------------------


void GMSHInputModule::createElems_ 

  ( const Properties&   globdat,
    const idx_t         offset )

{
  JEM_PRECHECK2(gmsh::isInitialized(), "GMSH was not initialized");

  std::vector<int>                      types;
  std::vector<std::vector<std::size_t>> elemTags;
  std::vector<std::vector<std::size_t>> nodeTags;

  std::string                           elemName;
  int                                   dim, order, numNodes, numPrimaryNodes;
  std::vector<double>                   localCoords;

  idx_t                                 addedElem;

  IdxBuffer                             groupElems;
  IdxVector                             elemNodes;

  Assignable<ElementGroup>              elementGroup;
  String                                groupName;

  Array<IdxBuffer>                      entityBuffer (4);
  IdxVector                             entityNumbering (4);

  entityNumbering                       = 0;
  for (idx_t i = 0; i < 4; i++ ) entityBuffer[i].clear();

  for (idx_t ientity = 0; ientity < entities_.size(); ientity++)
  {
    gmsh::model::mesh::getElements( types, elemTags, nodeTags, entities_[ientity][0], entities_[ientity][1]);
    
    groupName   = String(ENTITY_NAMES[entities_[ientity][0]]) + String('_') + String(++entityNumbering[entities_[ientity][0]]);    
    groupElems.clear();

    for (size_t itype = 0; itype < types.size(); itype++)
    {    
      gmsh::model::mesh::getElementProperties( types[itype], elemName, dim, order, numNodes, localCoords, numPrimaryNodes );
          
      for (size_t ielem = 0; ielem < elemTags[itype].size(); ielem++)
      {
        elemNodes.resize( numNodes );

        for (idx_t inode = 0; inode < numPrimaryNodes; inode++)
        {
          elemNodes[inode*order] = nodeTags[itype][ielem*numNodes + inode]-offset;

          if (inode*order+1 == numNodes) break;
          for (idx_t jnode = 1; jnode < order; jnode++)
            elemNodes[inode*order+jnode] = nodeTags[itype][ielem*numNodes + numPrimaryNodes + inode*(order-1) + jnode-1]-offset;
        }

        addedElem = elements_.addElement( elemNodes ); 
        jem::System::info( myName_ ) << " ...Created element " << addedElem << " with nodes " << elemNodes << "\n";

        groupElems.pushBack( addedElem );
        entityBuffer[entities_[ientity][0]].pushBack( addedElem ); 
      }
    }

    elementGroup =  jive::fem::newElementGroup( groupElems.toArray(), elements_ );
    elementGroup.store( groupName, globdat );
    jem::System::info( myName_ ) << " ...Created element group for geometry entity '" << groupName << "'\n";
  } 
  jem::System::info( myName_ ) << "\n";

  // store all the super element groups
  for (idx_t i = 0; i < 4; i++)
  {
    elementGroup = jive::fem::newElementGroup( entityBuffer[i].toArray(), elements_ );
    if (elementGroup.size() > 0)
    { 
      elementGroup.store( String(ENTITY_NAMES[i]) + String('s'), globdat );
    
      jem::System::info( myName_ ) << " ...Created element group for geometry entities of type '" << String(ENTITY_NAMES[i]) + String('s') << "'\n";
    }
  }  
  jem::System::info( myName_ ) << "\n";
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