/**
 * @file MaterialFactory.h
 * @author Til Gärtner
 * @brief Factory to register all the materials with
 *
 */
#include "materials/MaterialFactory.h"

//=======================================================================
//   class MaterialFactory
//=======================================================================

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void MaterialFactory::declare

    (const String &type,
     Constructor ctor)

{
  JEM_PRECHECK(ctor);

  CtorMap_::insert(type, ctor);
}

//-----------------------------------------------------------------------
//   exists
//-----------------------------------------------------------------------

bool MaterialFactory::exists(const String &type)
{
  return CtorMap_::contains(type);
}

//-----------------------------------------------------------------------
//   listKnownTypes
//-----------------------------------------------------------------------

StringVector MaterialFactory::listKnownTypes()
{
  return CtorMap_::listKnownTypes();
}

//-----------------------------------------------------------------------
//   newInstance
//-----------------------------------------------------------------------

Ref<Material> MaterialFactory::newInstance

    (const String &type,
     const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)

{
  Constructor ctor = CtorMap_::find(type);

  if (ctor)
  {
    return ctor(name, conf, props, globdat);
  }
  else
  {
    return nullptr;
  }
}

Ref<Material> MaterialFactory::newInstance

    (const String &name,
     const Properties &conf,
     const Properties &props,
     const Properties &globdat)

{
  Properties subConf = conf.makeProps(name);
  Properties subProps = props.getProps(name);

  Ref<Material> material;
  String type;

  subProps.get(type, TYPE_PROP);
  subConf.set(TYPE_PROP, type);

  material = newInstance(type, name, conf, props, globdat);

  if (!material)
  {
    noSuchTypeError(
        subProps.getContext(TYPE_PROP),
        type, "material",
        CtorMap_::listKnownTypes());
  }

  return material;
}
