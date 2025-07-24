/**
 * @file LeapFrogModule.h
 * @author Til Gärtner
 * @brief class that implements a leap frog algorithm for the explicit
 * solver
 *
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

      (const Properties &info, const Properties &globdat) override;

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

protected:
  virtual ~LeapFrogModule();
};
