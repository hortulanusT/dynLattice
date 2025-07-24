/**
 * @file CSVOutputModule.h
 * @author Til Gärtner
 * @brief Module to enable the Standard Output Module to Output CSV Files
 *
 *
 */

#include <jem/base/ClassTemplate.h>

#include "misc/CSVDataPrinter.h"
#include "modules/CSVOutputModule.h"

JEM_DEFINE_CLASS(CSVOutputModule);

//=======================================================================
//   class CSVOutputModule
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *CSVOutputModule::TYPE_NAME = "CSVOutput";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

CSVOutputModule::CSVOutputModule(
    const String &name,
    const Ref<DataPrinter> printer) :

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
