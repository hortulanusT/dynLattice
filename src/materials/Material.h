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

#include <jem/base/NamedObject.h>
#include <jem/util/Properties.h>
#include <jive/Array.h>

using jem::NamedObject;
using jem::util::Properties;
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
  virtual void configure(const Properties &props);

  /**
   * @brief get the configuration
   *
   * @param conf properties to store the configuration to
   */
  virtual void getConfig(const Properties &conf) const;

  /**
   * @brief compute the stresses
   *
   * @param[out] stress
   * @param[in] strain
   */
  virtual void getStress(const Vector &stress, const Vector &strain) const = 0;

  /**
   * @brief get the material stiffness matrix
   *
   * @return stiffness matrix
   */
  virtual Matrix getMaterialStiff() const = 0;

  /**
   * @brief get the rigid body mass (assuming cantilever at end)
   *
   * @param[out] M ridig body mass matrix
   * @param[in] l length of cantilever
   * @param[in] border wether to calculate the mass for a border element (half the length + steiner contribution)
   */
  virtual void getLumpedMaterialMass(const Matrix &M, const double l, const bool border) const = 0;

protected:
  /**
   * @brief Destroy the Material object
   *
   */
  virtual ~Material();
};
