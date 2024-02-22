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
#include <jive/util/XTable.h>

using jem::NamedObject;
using jem::util::Properties;
using jive::Cubix;
using jive::idx_t;
using jive::IdxVector;
using jive::Matrix;
using jive::String;
using jive::Vector;
using jive::util::XTable;

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
  virtual void getStress(const Vector &stress, const Vector &strain) = 0;

  virtual inline void getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip);

  /**
   * @brief get the material stiffness matrix
   *
   * @return material stiffness matrix
   */
  virtual Matrix getMaterialStiff() const = 0;

  virtual inline Matrix getMaterialStiff(const idx_t &ielem, const idx_t &ip) const;

  /**
   * @brief get the material mass matrix
   *
   * @return mass matrix (per unit)
   */
  virtual Matrix getMaterialMass() const = 0;

  virtual inline Matrix getMaterialMass(const idx_t &ielem, const idx_t &ip) const;

  virtual Matrix getLumpedMass(double l) const;

  virtual void apply_inelast_corr() = 0;

  virtual void reject_inelast_corr() = 0;

  virtual void getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const = 0;

  virtual double getDissipatedEnergy(const idx_t &ielem, const idx_t &ip) const = 0;

  virtual String getContext() const override;

protected:
  /**
   * @brief Destroy the Material object
   *
   */
  virtual ~Material();
};

void Material::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip)
{
  getStress(stress, strain);
}

Matrix Material::getMaterialStiff(const idx_t &ielem, const idx_t &ip) const
{
  return getMaterialStiff();
}

Matrix Material::getMaterialMass(const idx_t &ielem, const idx_t &ip) const
{
  return getMaterialMass();
}