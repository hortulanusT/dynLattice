/**
 * @file CorrectorAdaptModule.h
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

class CorrectorAdaptModule : public ExplicitModule
{
public:
  JEM_DECLARE_CLASS(CorrectorAdaptModule, ExplicitModule);

  static const char *TYPE_NAME;

  explicit CorrectorAdaptModule

      (const String &name = "CorrectorAdapt");

  virtual Status run

      (const Properties &globdat);

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

  virtual ~CorrectorAdaptModule();

private:
  inline double getQuality_(const Vector &y_pre, const Vector &y_cor);

  /// @brief Adams Moulton 2 step corrector (Trapezoidal rule)
  inline void AMupdate_(const Vector &delta_y, const Vector &f_pre,
                        const Vector &f_cur) const;
  /// @brief Adams Moulton 1 step corrector (Euler Implizit)
  inline void AMupdate_(const Vector &delta_y, const Vector &f_pre) const;

private:
};

inline double CorrectorAdaptModule::getQuality_(const Vector &y_pre,
                                                const Vector &y_cor)
{
  return norm2(Vector(y_pre - y_cor));
}

inline void CorrectorAdaptModule::AMupdate_(const Vector &delta_y,
                                            const Vector &f_pre,
                                            const Vector &f_cur) const
{
  delta_y = dtime_ / 2. * (1. * f_pre + 1. * f_cur);
}
inline void CorrectorAdaptModule::AMupdate_(const Vector &delta_y,
                                            const Vector &f_pre) const
{
  delta_y = dtime_ * f_pre;
}