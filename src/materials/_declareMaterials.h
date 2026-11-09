/**
 * @file _declareMaterials.h
 * @author Til Gärtner
 * @brief Material registration utility for factory initialization.
 *
 */
#pragma once

#include "materials/ElasticRodMaterial.h"
#include "materials/ElastoPlasticRodMaterial.h"
#include "materials/ElasticSolidMaterial.h"
#include "materials/NeoHookeanSolidMaterial.h"
#include "materials/SaintVenantKirchhoffSolidMaterial.h"

//-----------------------------------------------------------------------
//   declareMaterials
//-----------------------------------------------------------------------

/// @brief Register all available material types with MaterialFactory.
///
/// Central registration point for all material types in the simulation framework.
/// Must be called during application initialization before material instantiation.
///
/// @section Currently Registered Materials
/// - ElasticRodMaterial: Linear elastic material for rod elements
/// - ElastoPlasticRodMaterial: Elasto-plastic material with hardening
/// - ElasticSolidMaterial: Small-strain linear elastic solid
/// - SaintVenantKirchhoffSolidMaterial: Finite-strain solid, linear in the Green-Lagrange strain
/// - NeoHookeanSolidMaterial: Compressible Neo-Hookean solid
///
/// @note Function is idempotent - multiple calls are safe
/// @warning Failure to call results in jive::util::noSuchTypeError during creation of materials
/// @see MaterialFactory::declare()
void declareMaterials();
