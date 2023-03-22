/**
 * @file CSVOutputModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Module to enable the Standard Output Module to Output CSV Files
 * @version 0.1
 * @date 2022-06-22
 *
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 *
 */

#include <jem/base/ClassTemplate.h>

#include "modules/CSVOutputModule.h"
#include "misc/CSVDataPrinter.h"

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
