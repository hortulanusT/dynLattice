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
 * @brief Register all available material types with the MaterialFactory.
 *
 * This function serves as the central registration point for all material types
 * available in the simulation framework. It must be called during application
 * initialization to ensure that all material types are properly registered
 * with the MaterialFactory before any material instantiation attempts.
 *
 * @section declare_materials_purpose Purpose
 * The function coordinates the registration of all concrete material
 * implementations by calling their respective declare() methods. This
 * centralized approach ensures:
 * - All materials are registered consistently
 * - No material types are accidentally omitted
 * - Clear dependency management for material registration
 *
 * @section declare_materials_registered Currently Registered Materials
 * This function registers the following material types:
 * - **ElasticRodMaterial**: Linear elastic material for rod elements
 * - **ElastoPlasticRodMaterial**: Elasto-plastic material with hardening
 *
 * @section declare_materials_usage Usage
 * This function should be called once during application startup:
 * ```cpp
 * int main() {
 *   declareMaterials();  // Register all material types
 *   // ... rest of application initialization
 * }
 * ```
 *
 * @note This function must be called before any MaterialFactory::newInstance() calls
 * @note Adding new material types requires updating this function
 * @note The function is idempotent - multiple calls are safe but unnecessary
 *
 * @see MaterialFactory::declare()
 * @see ElasticRodMaterial::declare()
 * @see ElastoPlasticRodMaterial::declare()
 * @see MaterialFactory
 *
 * @warning Failure to call this function will result in jive::util::noSuchTypeError
 *          when attempting to create material instances
 */
void declareMaterials();
