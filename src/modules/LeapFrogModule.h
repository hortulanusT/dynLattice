/**
 * @file LeapFrogModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief class that implements a leap frog algorithm for the explicit
 * solver
 * @version 0.1
 * @date 2023-01-27
 *
 * @copyright Copyright (C) 2023 TU Delft. All rights reserved.
 *
 */
#pragma once

#include "modules/ExplicitModule.h"

class LeapFrogModule : public ExplicitModule
{
public:
  JEM_DECLARE_CLASS(LeapFrogModule, ExplicitModule);

  static const char *TYPE_NAME;

  explicit LeapFrogModule

      (const String &name = "leapFrog");

  virtual void solve

    (const Properties& info, const Properties& globdat) override;

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

protected:
  virtual ~LeapFrogModule();
};