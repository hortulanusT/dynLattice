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
 * @brief Factory for dynamic creation and registration of material types.
 *
 * The MaterialFactory provides a centralized registry for all available material
 * types in the simulation framework. It enables dynamic instantiation of materials
 * based on string identifiers, supporting a plugin-like architecture for material
 * models.
 *
 * @section factory_usage Factory Usage
 * The factory operates in two phases:
 * 1. **Registration Phase**: Material types register their constructors using declare()
 * 2. **Creation Phase**: Materials are instantiated using newInstance() methods
 *
 * @section factory_registration Material Registration
 * Each material type must register itself with the factory:
 * ```cpp
 * // In material implementation:
 * void MyMaterial::declare() {
 *   MaterialFactory::declare("MyMaterial", &makeNew);
 * }
 * ```
 *
 * @section factory_creation Material Creation
 * Materials can be created by type name:
 * ```cpp
 * Material material = MaterialFactory::newInstance("ElasticRodMaterial",
 *                                              name, conf, props, globdat);
 * ```
 *
 * @section factory_hierarchy Supported Material Types
 * The factory supports all Material-derived classes:
 * - ElasticRodMaterial: Linear elastic rod elements
 * - ElastoPlasticRodMaterial: Elasto-plastic rod elements with hardening
 *
 * @note This factory inherits from jive::util::Factory providing base functionality
 * @see Material
 * @see ElasticRodMaterial
 * @see ElastoPlasticRodMaterial
 * @see declareMaterials()
 * @see <a href="https://jive-manual.dynaflow.com/classjive_1_1util_1_1Factory.html">jive::util::Factory</a>
 *
 * @author Til Gärtner
 */
class MaterialFactory : public Factory
{
public:
  /// @brief Type alias for the MaterialFactory class itself
  typedef MaterialFactory Self;

  /// @brief Type alias for the base Factory class
  typedef Factory Super;

  /**
   * @brief Constructor function pointer type for material creation.
   *
   * This function signature defines the interface that all material constructors
   * must implement. The factory uses these function pointers to create material
   * instances dynamically without compile-time dependencies.
   *
   * @param name Unique identifier for the material instance
   * @param conf Configuration properties container for output
   * @param props Input properties containing material parameters
   * @param globdat Global simulation data and context
   * @returns Reference to newly created Material object
   *
   * @note Constructor functions must return valid Material references
   * @note All registered constructors must match this exact signature
   * @see Material::Material()
   */
  typedef Ref<Material> (*Constructor)(const String &name,
                                       const Properties &conf,
                                       const Properties &props,
                                       const Properties &globdat);

  /**
   * @brief Register a new material type with the factory.
   *
   * Associates a material type identifier with its constructor function,
   * enabling dynamic creation of that material type. This method is typically
   * called during application initialization by each material's declare() method.
   *
   * @param type String identifier for the material type (e.g., "ElasticRodMaterial")
   * @param ctor Constructor function pointer matching the Constructor signature
   *
   * @note Type identifiers should be unique across all registered materials
   * @note Constructor functions must match the exact Constructor signature
   * @note Registration should occur before any creation attempts
   *
   * @see Constructor
   * @see exists()
   * @see newInstance()
   *
   * @par Example:
   * ```cpp
   * MaterialFactory::declare("ElasticRodMaterial", &ElasticRodMaterial::makeNew);
   * ```
   */
  static void declare(const String &type, Constructor ctor);

  /**
   * @brief Check if a material type is registered with the factory.
   *
   * Queries the factory registry to determine if a given material type
   * identifier has been registered and is available for instantiation.
   *
   * @param type String identifier for the material type to check
   * @returns true if the material type is registered, false otherwise
   *
   * @note Case-sensitive string comparison is used
   * @see declare()
   * @see listKnownTypes()
   *
   * @par Example:
   * ```cpp
   * if (MaterialFactory::exists("ElasticRodMaterial")) {
   *   // Safe to create instances of this type
   * }
   * ```
   */
  static bool exists(const String &type);

  /**
   * @brief Get a list of all registered material type identifiers.
   *
   * Returns a vector containing the string identifiers of all material types
   * that have been successfully registered with the factory. Useful for
   * debugging, user interfaces, and validation purposes.
   *
   * @returns StringVector containing all registered material type identifiers
   *
   * @note The returned vector contains only the type identifiers, not instances
   * @see declare()
   * @see exists()
   */
  static StringVector listKnownTypes();

  /**
   * @brief Create a material instance with explicit type specification.
   *
   * Creates a new material instance by explicitly specifying the material type.
   * This is the primary method for material instantiation when the type is
   * known at the point of creation.
   *
   * @param type String identifier for the material type to instantiate
   * @param name Unique name for the material instance
   * @param conf Configuration properties container for output and state storage
   * @param props Input properties containing material parameters and settings
   * @param globdat Global simulation data and context information
   * @returns Reference to the newly created Material instance
   *
   * @note Returns nullpointer if the type is not registered
   * @note Configuration output will be written to the conf parameter
   *
   * @see declare()
   * @see exists()
   * @see Constructor
   */
  static Ref<Material> newInstance(const String &type,
                                   const String &name,
                                   const Properties &conf,
                                   const Properties &props,
                                   const Properties &globdat);

  /**
   * @brief Create a material instance with type inferred from properties.
   *
   * Creates a new material instance by reading the material type from the
   * properties object. The type identifier should be specified in the props
   * under a conventional property name (typically "type").
   *
   * @param name Unique name for the material instance
   * @param conf Configuration properties container for output and state storage
   * @param props Input properties containing material type and parameters
   * @param globdat Global simulation data and context information
   * @returns Reference to the newly created Material instance
   * @throws jive::util::noSuchTypeError if the type is not registered
   * @note This method provides convenience when material type is specified in input files
   * @note The material type must be specified in the properties object
   *
   * @see declare()
   * @see exists()
   * @see Constructor
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
 * @brief Internal constructor map for MaterialFactory type registration.
 *
 * This class provides the underlying storage and lookup mechanism for
 * material type constructors. It inherits from jive::util::CtorMap to
 * leverage the framework's constructor mapping infrastructure.
 *
 * @note This is an implementation detail and should not be used directly
 * @note All interaction should go through MaterialFactory public methods
 * @see MaterialFactory
 * @see <a href="https://jive-manual.dynaflow.com/classjive_1_1util_1_1CtorMap.html">jive::util::CtorMap</a>
 */
class MaterialFactory::CtorMap_ : public jive::util::CtorMap<Constructor>
{
};
