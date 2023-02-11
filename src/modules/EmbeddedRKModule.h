/**
 * @file EmbeddedRKModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief class that implements a leap frog algorithm for the explicit
 * solver
 * @version 0.1
 * @date 2023-01-27
 *
 * @copyright Copyright (C) 2023 TU Delft. All rights reserved.
 *
 */
#pragma once

#include "modules/ExplicitModule.h"

class EmbeddedRKModule : public ExplicitModule
{
public:
  JEM_DECLARE_CLASS(EmbeddedRKModule, ExplicitModule);

  static const char *TYPE_NAME;

  explicit EmbeddedRKModule

      (const String &name = "EmbeddedRK");

  virtual Status init

      (const Properties &conf, const Properties &props,
       const Properties &globdat);

  virtual void solve

    (const Properties& info, const Properties& globdat) override;

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

  virtual ~EmbeddedRKModule();

private:
  /// @brief initialize the butcher tableau of the Bogacki Shampine method
  /// (ode23 in matlab)
  void initODE23_();

  /// @brief initialize the butcher tableau of the Dormand Prince method
  /// (ode45 in matlab)
  void initODE45_();

  /// @brief corrects the rotation values for updates with the RKMK method
  /// see also
  /// https://link.springer.com/referenceworkentry/10.1007/978-3-540-70529-1_122#Sec1139
  void correctDisp_(const Vector &uncorrected, const Vector &delta);

  double bernoulliCoeff_(const idx_t i);
  Matrix adjoint_(const Matrix &point, const Matrix &about,
                  const idx_t iterate);

  void invDerivExpMap_(const Matrix &res, const Matrix &point,
                       const Matrix &about);

private:
  String kind_;
  idx_t order_;

  idx_t butchSize_;
  Matrix a_;
  Vector b_;
  bool fsal_;
  Vector c_;
};
