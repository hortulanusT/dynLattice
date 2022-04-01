#include "GMSHInputModule.h"

//=======================================================================
//   class GMSHInputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------


const char*  GMSHInputModule::TYPE_NAME = "None";


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
  return OK;
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
  ModuleFactory::declare ( CLASS_NAME, & makeNew );
}