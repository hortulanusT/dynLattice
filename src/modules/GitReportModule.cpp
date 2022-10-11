/**
 * @file GitReportModule.cpp
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief see GitReportModule.h
 * @version 0.1
 * @date 2022-10-11
 *
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
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
  jem::System::info() << "Build with git hash " << GIT_HASH << "\n\n";
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