/*
 * Copyright (C) 2023 TU Delft. All rights reserved.
 *
 * This class implements a general Material
 *
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: May 23
 *
 */

#pragma once

#include <jem/base/Array.h>
#include <jem/base/NamedObject.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>

using jem::NamedObject;
using jem::util::Properties;
using jive::Cubix;
using jive::idx_t;
using jive::Matrix;
using jive::String;
using jive::Vector;

class Material : public NamedObject
{
public:
  JEM_DECLARE_CLASS(Material, NamedObject);

  Material(const String &name,
           const Properties &conf,
           const Properties &props,
           const Properties &globdat);

  /**
   * @brief configure the material
   *
   * @param props properties to be used in the configuration
   */
  virtual void configure(const Properties &props, const Properties &globdat);

  /**
   * @brief get the configuration
   *
   * @param conf properties to store the configuration to
   */
  virtual void getConfig(const Properties &conf, const Properties &globdat) const;

  /**
   * @brief compute the stresses
   *
   * @param[out] stress
   * @param[in] strain
   */
  virtual void getStress(const Vector &stress, const Vector &strain) const = 0;

  /**
   * @brief compute the stresses
   *
   * @param[out] stress
   * @param[in] strain
   * @param[in] ielem
   * @param[in] ip
   */
  virtual void getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip) const;

  /**
   * @brief get the material stiffness matrix
   *
   * @return stiffness matrix
   */
  virtual Matrix getMaterialStiff() const = 0;

  virtual Matrix getConsistentStiff(const Vector &stress) const;

  /**
   * @brief get the material mass matrix
   *
   * @return mass matrix (per unit)
   */
  virtual Matrix getMaterialMass() const = 0;

  virtual Matrix getLumpedMass(double l) const;

protected:
  /**
   * @brief Destroy the Material object
   *
   */
  virtual ~Material();

public:
  Cubix stresses;
  Cubix plastStrains;
};
