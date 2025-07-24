/**
 * @file GitReportModule.cpp
 * @author Til Gärtner
 * @brief see GitReportModule.h
 *
 *
 */

#include "modules/GitReportModule.h"

#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(GitReportModule);

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
  jem::System::info() << "Built with git hash " << GIT_HASH << "\n\n";
  return DONE;
}

Module::Status GitReportModule::run(const Properties &globdat)
{
  return DONE;
}

void GitReportModule::shutdown(const Properties &globdat)
{
}

Ref<Module> GitReportModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return jem::newInstance<Self>(name);
}

void GitReportModule::declare()
{
  ModuleFactory::declare(TYPE_NAME, &makeNew);
  ModuleFactory::declare(CLASS_NAME, &makeNew);
}
