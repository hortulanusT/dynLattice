/**
 * @file MilneDeviceModule.h
 * @author Til Gärtner
 * @brief class that implements a leap frog algorithm for the explicit
 * solver
 *
 *
 */
#pragma once

#include "modules/ExplicitModule.h"

class MilneDeviceModule : public ExplicitModule
{
public:
  JEM_DECLARE_CLASS(MilneDeviceModule, ExplicitModule);

  static const char *TYPE_NAME;

  explicit MilneDeviceModule

      (const String &name = "MilneDevice");

  virtual void solve

      (const Properties &info, const Properties &globdat);

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

  virtual ~MilneDeviceModule();

protected:
  /// @brief updated the forces (in the corrector step only the internal forces change)
  /// @returns resulting forces = external - internal
  Vector updForce(const Vector &fint,
                  const Vector &fext,
                  const Properties &globdat);

private:
  /// @brief Adams Moulton 2 step corrector (Trapezoidal rule)
  inline void AMupdate_(const Vector &delta_y, const Vector &f_pre,
                        const Vector &f_cur) const;
  /// @brief Adams Moulton 1 step corrector (Euler Implizit)
  inline void AMupdate_(const Vector &delta_y, const Vector &f_pre) const;

private:
};

inline void MilneDeviceModule::AMupdate_(const Vector &delta_y,
                                         const Vector &f_pre,
                                         const Vector &f_cur) const
{
  delta_y = dtime_ / 2. * (1. * f_pre + 1. * f_cur);
}
inline void MilneDeviceModule::AMupdate_(const Vector &delta_y,
                                         const Vector &f_pre) const
{
  delta_y = dtime_ * f_pre;
}
