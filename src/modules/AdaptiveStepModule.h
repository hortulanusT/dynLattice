/**
 * @file AdaptiveStepModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief class that implements adaptive time stepping with a NonlinModule for adapting to explicit plasicity stepping
 * @version 0.1
 * @date 2023-01-27
 *
 * @copyright Copyright (C) 2023 TU Delft. All rights reserved.
 *
 * Heaviliy influence by the module of the same name by F.P. van der Meer
 *
 */

#pragma once
#include <cmath>
#include <jem/base/ClassTemplate.h>
#include <jem/base/StringObject.h>
#include <jem/util/Properties.h>
#include <jive/app/ModuleFactory.h>
#include <jive/implict/Names.h>
#include <jive/implict/NonlinModule.h>
#include <jive/implict/SolverInfo.h>
#include <jive/implict/SolverModule.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/Names.h>
#include <jive/model/StateVector.h>
#include <jive/solver/SolverException.h>
#include <jive/util/DofSpace.h>
#include <jive/util/FuncUtils.h>
#include <jive/util/Globdat.h>
#include <jive/util/utilities.h>

using jem::newInstance;
using jive::idx_t;
using jive::Properties;
using jive::Ref;
using jive::String;
using jive::app::Module;
using jive::implict::NonlinModule;
using jive::implict::SolverInfo;
using jive::implict::SolverModule;
using jive::model::ActionParams;
using jive::model::Actions;
using jive::model::Model;
using jive::model::StateVector;
using jive::util::DofSpace;
using jive::util::FuncUtils;
using jive::util::Globdat;

//-----------------------------------------------------------------------
//   class AdaptiveStepModule
//-----------------------------------------------------------------------

class AdaptiveStepModule : public SolverModule
{
public:
  JEM_DECLARE_CLASS(AdaptiveStepModule, SolverModule);

  static const char *TYPE_NAME;

  explicit AdaptiveStepModule

      (const String &name = "AdaptiveStep",
       Ref<NonlinModule> solver = nullptr);

  virtual Status init

      (const Properties &conf,
       const Properties &props,
       const Properties &globdat) override;

  virtual void configure

      (const Properties &props,
       const Properties &globdat) override;

  virtual void getConfig

      (const Properties &props,
       const Properties &globdat) const override;

  virtual void advance

      (const Properties &globdat) override;

  virtual void solve

      (const Properties &info,
       const Properties &globdat) override;

  virtual void cancel

      (const Properties &globdat) override;

  virtual bool commit

      (const Properties &globdat) override;

  virtual inline void setPrecision

      (double eps) override;

  virtual inline double getPrecision() const override;

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

protected:
  virtual ~AdaptiveStepModule();

private:
  Ref<NonlinModule> solver_;
  Ref<Model> model_;
  Ref<DofSpace> dofs_;

  double oldLoadScale_;
  double loadScale_;
  double incr_;
  double minIncr_;
  double maxIncr_;
  double incrFact_;
  double decrFact_;
};

//-----------------------------------------------------------------------
//   inline definitions
//-----------------------------------------------------------------------
void AdaptiveStepModule::setPrecision(double eps)
{
  solver_->setPrecision(eps);
}

double AdaptiveStepModule::getPrecision() const
{
  return solver_->getPrecision();
}
