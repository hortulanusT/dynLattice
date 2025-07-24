/**
 * @file _declareMaterials.cpp
 * @author Til Gärtner
 * @brief Implementation of material registration utility for factory initialization.
 * 
 */
#include "materials/_declareMaterials.h"

//-----------------------------------------------------------------------
//   declareMaterials
//-----------------------------------------------------------------------

void declareMaterials()
{
  ElasticRodMaterial::declare();
  ElastoPlasticRodMaterial::declare();
}
