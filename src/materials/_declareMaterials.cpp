#include "materials/_declareMaterials.h"

//-----------------------------------------------------------------------
//   declareMaterials
//-----------------------------------------------------------------------

void declareMaterials()
{
  ElasticRodMaterial::declare();
  ElastoPlasticRodMaterial::declare();
}
