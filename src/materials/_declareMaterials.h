/**
 * @file _declareMaterials.h
 * @author Til Gärtner
 * @brief Material registration utility for factory initialization.
 *
 */
#pragma once

#include "materials/ElasticRodMaterial.h"
#include "materials/ElastoPlasticRodMaterial.h"

//-----------------------------------------------------------------------
//   declareMaterials
//-----------------------------------------------------------------------

/**
 * @brief Register all available material types with MaterialFactory.
 *
 * Central registration point for all material types in the simulation framework.
 * Must be called during application initialization before material instantiation.
 *
 * @section Currently Registered Materials
 * - ElasticRodMaterial: Linear elastic material for rod elements
 * - ElastoPlasticRodMaterial: Elasto-plastic material with hardening
 *
 * @note Function is idempotent - multiple calls are safe
 * @warning Failure to call results in jive::util::noSuchTypeError during creation of materials
 * @see MaterialFactory::declare(), ElasticRodMaterial::declare(), ElastoPlasticRodMaterial::declare()
 */
void declareMaterials();
