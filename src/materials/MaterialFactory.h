/**
 * @file MaterialFactory.h
 * @author Til Gärtner
 * @brief Factory to register all the materials with
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

/**
 * @brief Factory for dynamic material creation and registration.
 *
 * Provides centralized registry for material types, enabling dynamic instantiation
 * based on string identifiers. Materials register with declare() and are created
 * with newInstance().
 *
 * @section SupportedMaterials Supported Materials
 * - ElasticRodMaterial: Linear elastic rod elements
 * - ElastoPlasticRodMaterial: Elasto-plastic rod elements with hardening
 *
 * @author Til Gärtner
 * @see Material, ElasticRodMaterial, ElastoPlasticRodMaterial, declareMaterials()
 */
class MaterialFactory : public Factory
{
public:
  /// @brief Type alias for the MaterialFactory class itself
  typedef MaterialFactory Self;

  /// @brief Type alias for the base Factory class
  typedef Factory Super;

  /**
   * @brief Constructor function pointer for material creation.
   *
   * @param name Material instance identifier
   * @param conf Configuration properties for output
   * @param props Input properties with material parameters
   * @param globdat Global simulation data
   * @returns Reference to newly created Material
   */
  typedef Ref<Material> (*Constructor)(const String &name,
                                       const Properties &conf,
                                       const Properties &props,
                                       const Properties &globdat);

  /**
   * @brief Register a material type with the factory.
   *
   * @param type Material type identifier (e.g., "ElasticRodMaterial")
   * @param ctor Constructor function matching Constructor signature
   */
  static void declare(const String &type, Constructor ctor);

  /**
   * @brief Check if a material type is registered.
   *
   * @param type Material type identifier to check
   * @returns true if registered, false otherwise
   */
  static bool exists(const String &type);

  /**
   * @brief Get list of all registered material types.
   *
   * @returns Vector of registered material type identifiers
   */
  static StringVector listKnownTypes();

  /**
   * @brief Create material instance with explicit type.
   *
   * @param type Material type identifier
   * @param name Material instance name
   * @param conf Configuration properties for output
   * @param props Input properties with parameters
   * @param globdat Global simulation data
   * @returns Reference to created Material (null if type not registered)
   */
  static Ref<Material> newInstance(const String &type,
                                   const String &name,
                                   const Properties &conf,
                                   const Properties &props,
                                   const Properties &globdat);

  /**
   * @brief Create material instance with type from properties.
   *
   * @param name Material instance name
   * @param conf Configuration properties for output
   * @param props Input properties containing type and parameters
   * @param globdat Global simulation data
   * @returns Reference to created Material
   * @throws jive::util::noSuchTypeError if type not registered
   */
  static Ref<Material> newInstance(const String &name,
                                   const Properties &conf,
                                   const Properties &props,
                                   const Properties &globdat);

private:
  /// @brief Forward declaration of internal constructor map class
  class CtorMap_;
};

/**
 * @brief Internal constructor map for MaterialFactory.
 *
 * @note Implementation detail - use MaterialFactory public methods instead.
 */
class MaterialFactory::CtorMap_ : public jive::util::CtorMap<Constructor>
{
};
