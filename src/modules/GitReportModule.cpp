/**
 * @file GitReportModule.cpp
 * @author Til Gärtner
 * @brief Implementation of GitReportModule class
 */

#include "modules/GitReportModule.h"

#include <jem/base/ClassTemplate.h>

//=======================================================================
//   class GitReportModule
//=======================================================================

JEM_DEFINE_CLASS(GitReportModule);

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------

const char *GitReportModule::TYPE_NAME = "GitReport";

GitReportModule::GitReportModule(const String &name)
    :

      Super(name)

{
}

GitReportModule::~GitReportModule()
{
}

Module::Status GitReportModule::init

    (const Properties &conf, const Properties &props,
     const Properties &globdat)
{
  (void)conf;    // unused
  (void)props;   // unused
  (void)globdat; // unused

  jem::System::info() << "Built with git hash " << GIT_HASH << "\n\n";
  return DONE;
}

Module::Status GitReportModule::run(const Properties &globdat)
{
  (void)globdat; // unused

  return DONE;
}

void GitReportModule::shutdown(const Properties &globdat)
{
  (void)globdat; // unused
}

Ref<Module> GitReportModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  (void)conf;    // unused
  (void)props;   // unused
  (void)globdat; // unused

  return jem::newInstance<Self>(name);
}

void GitReportModule::declare()
{
  ModuleFactory::declare(TYPE_NAME, &makeNew);
  ModuleFactory::declare(CLASS_NAME, &makeNew);
}
