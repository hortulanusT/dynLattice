/**
 * @file MaterialFactory.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Factory to register all the materials with
 * @version 0.1
 * @date 2023-05-31
 *
 * @copyright Copyright (C) 2023 TU Delft. All rights reserved.
 *
 */

#pragma once

#include "materials/Material.h"
#include <jem/util/Properties.h>
#include <jive/util/CtorMap.h>
#include <jive/util/Factory.h>

using jive::Properties;
using jive::Ref;
using jive::String;
using jive::StringVector;
using jive::util::Factory;

class MaterialFactory : public Factory
{
public:
  typedef MaterialFactory Self;
  typedef Factory Super;

  typedef Ref<Material> (*Constructor)

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  static void declare

      (const String &type,
       Constructor ctor);

  static bool exists

      (const String &type);

  static StringVector listKnownTypes();

  static Ref<Material> newInstance

      (const String &type,
       const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

  static Ref<Material> newInstance

      (const String &name,
       const Properties &conf,
       const Properties &props,
       const Properties &globdat);

private:
  class CtorMap_;
};
