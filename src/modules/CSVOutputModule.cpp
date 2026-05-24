/**
 * @file CSVOutputModule.cpp
 * @author Til Gärtner
 * @brief Implementation of CSVOutputModule class
 */

#include <jem/base/ClassTemplate.h>

#include "misc/CSVDataPrinter.h"
#include "modules/CSVOutputModule.h"

//=======================================================================
//   class CSVOutputModule
//=======================================================================

JEM_DEFINE_CLASS(CSVOutputModule);

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *CSVOutputModule::TYPE_NAME = "CSVOutput";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

CSVOutputModule::CSVOutputModule(
    const String &name,
    const Ref<DataPrinter>) :

                              Super(name, newInstance<CSVDataPrinter>(jive::util::joinNames(name, "printer")))

{
}

CSVOutputModule::~CSVOutputModule()
{
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> CSVOutputModule::makeNew

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)

{
  (void)conf;    // unused
  (void)props;   // unused
  (void)globdat; // unused

  return jem::newInstance<Self>(name);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void CSVOutputModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &makeNew);
  ModuleFactory::declare(CLASS_NAME, &makeNew);
}
