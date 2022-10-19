/**
 * @file GitReportModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Print the Git Status to the start of the log-file
 * @version 0.1
 * @date 2022-10-11
 *
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 *
 */

#pragma once

#ifndef GIT_HASH
#error "Please provide the githash using -DGIT_HASH!"
#endif

#include <jem/base/System.h>
#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>

using jive::Properties;
using jive::Ref;
using jive::String;
using jive::app::Module;
using jive::app::ModuleFactory;

class GitReportModule : public Module
{
public:
  JEM_DECLARE_CLASS(GitReportModule, Module);

  static const char *TYPE_NAME;

  explicit GitReportModule

      (const String &name = "GitReport");

  virtual Status init

      (const Properties &conf, const Properties &props,
       const Properties &globdat) override;

  virtual Status run

      (const Properties &globdat) override;

  virtual void shutdown

      (const Properties &globdat) override;

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

protected:
  virtual ~GitReportModule();
};