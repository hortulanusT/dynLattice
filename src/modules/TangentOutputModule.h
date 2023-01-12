/**
 * @file TangentOutputModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief Module, that calculates the tangent linear elastic equivalent
 * properties for the material
 * @version 0.1
 * @date 2022-11-21
 *
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 *
 */

#pragma once

#include <jem/base/Error.h>
#include <jem/numeric/Sparse.h>
#include <jem/numeric/sparse/select.h>
#include <jive/algebra/AbstractMatrix.h>
#include <jive/algebra/SparseMatrixObject.h>
#include <jive/app/Module.h>
#include <jive/app/ModuleFactory.h>
#include <jive/fem/FEMatrixBuilder.h>
#include <jive/implict/Names.h>
#include <jive/implict/NonlinModule.h>
#include <jive/implict/SolverModule.h>
#include <jive/implict/utilities.h>
#include <jive/model/ModelFactory.h>
#include <jive/solver/Solver.h>
#include <jive/solver/declare.h>
#include <jive/util/FuncUtils.h>

#include "models/DirichletModel.h"
#include "models/PeriodicBCModel.h"
#include "modules/PBCGroupOutputModule.h"

using jem::Ref;
using jem::numeric::Function;
using jive::Properties;
using jive::SparseMatrix;
using jive::String;
using jive::algebra::AbstractMatrix;
using jive::algebra::ConstrainedMatrix;
using jive::app::Module;
using jive::app::ModuleFactory;
using jive::fem::FEMatrixBuilder;
using jive::implict::SolverModule;
using jive::model::Model;
using jive::util::FuncUtils;

class TangentOutputModule : public Module
{
public:
  JEM_DECLARE_CLASS(TangentOutputModule, Module);

  static const char *TYPE_NAME;

  explicit TangentOutputModule

      (const String &name = "tangentOutput");

  virtual Status init

      (const Properties &conf, const Properties &props,
       const Properties &globdat) override;

  virtual Status run(const Properties &globdat) override;

  virtual void shutdown(const Properties &globdat) override;

  static Ref<Module> makeNew

      (const String &name, const Properties &conf,
       const Properties &props, const Properties &globdat);

  static void declare();

private:
  void readStrainStress_(const Vector &strains, const Vector &stresses,
                         const Properties &globdat);
  void reportStrainStress_(const Vector &H, const Vector &P);
  void getStrainStress_(const Matrix &strains, const Matrix &stresses,
                        const Properties &globdat);
  void storeTangentProps_(const Matrix &strains, const Matrix &stresses,
                          const Properties &globdat);

  void condenseMatrix_(const Matrix &resStiff, const Properties &globdat);
  void storeTangentProps_(const Matrix &resStiff,
                          const Properties &globdat);

protected:
  virtual ~TangentOutputModule();

private:
  String mode_;

  // general
  Ref<Model> masterModel_;
  Ref<Function> sampleCond_;
  idx_t rank_;

  // matrix condensation
  Ref<Constraints> cons_;
  IdxMatrix cornerDofs_;

  // finite differences
  Ref<SolverModule> solver_;
  Ref<GroupOutputModule> groupUpdate_;
  double thickness_;
  double perturb_;
  StringVector strains_;
  StringVector stresses_;
  StringVector sizes_;
};