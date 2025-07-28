/**
 * @file TangentOutputModule.h
 * @author Til Gärtner
 * @brief Module for calculating tangent elastic properties via homogenization
 *
 * Computes effective elastic properties of heterogeneous materials using
 * periodic boundary conditions and either finite differences or matrix
 * condensation techniques for homogenization analysis.
 */

#pragma once

#include <jem/base/Error.h>
#include <jem/numeric/Sparse.h>
#include <jem/numeric/sparse/select.h>
#include <jive/algebra/AbstractMatrix.h>
#include <jive/algebra/ConstrainedMatrix.h>
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

/// @brief Module for tangent elastic property calculation via homogenization
/// @details Computes effective elastic properties of heterogeneous materials
/// using periodic boundary conditions. Supports two computation modes:
/// - **Finite differences**: Applies strain perturbations and measures stress response
/// - **Matrix condensation**: Direct extraction from tangent stiffness matrix *(NOT IMPLEMENTED)*
///
/// The module performs strain-stress analysis on boundary nodes and calculates
/// homogenized material properties for multi-scale modeling applications.
class TangentOutputModule : public Module
{
public:
  JEM_DECLARE_CLASS(TangentOutputModule, Module);

  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME; ///< Module type name
  /// @}

  /// @brief Constructor
  /// @param name Module name (default: "tangentOutput")
  explicit TangentOutputModule(const String &name = "tangentOutput");

  /// @brief Initialize tangent property calculation
  /// @param conf Configuration properties (output)
  /// @param props User properties
  /// @param globdat Global data container
  /// @return Module status
  virtual Status init(const Properties &conf, const Properties &props,
                      const Properties &globdat) override;

  /// @brief Run tangent property calculation
  /// @param globdat Global data container
  /// @return Module status
  virtual Status run(const Properties &globdat) override;

  /// @brief Shutdown module and finalize output
  /// @param globdat Global data container
  virtual void shutdown(const Properties &globdat) override;

  /// @brief Factory method for creating new instances
  /// @param name Module name
  /// @param conf Configuration properties (output)
  /// @param props User properties
  /// @param globdat Global data container
  /// @return New module instance
  static Ref<Module> makeNew(const String &name, const Properties &conf,
                             const Properties &props, const Properties &globdat);

  /// @brief Register module type with factory
  static void declare();

private:
  /// @name Strain-stress analysis methods
  /// @{

  /// @brief Read current strain and stress from boundary
  /// @param strains Current strain vector
  /// @param stresses Current stress vector
  /// @param globdat Global data container
  void readStrainStress_(const Vector &strains, const Vector &stresses,
                         const Properties &globdat);

  /// @brief Report strain-stress data to output
  /// @param H Strain tensor components (displacement gradient)
  /// @param N Stress tensor components (nominal stress)
  void reportStrainStress_(const Vector &H, const Vector &N);

  /// @brief Calculate strain-stress relationship
  /// @param strains Strain matrix for different load cases
  /// @param stresses Corresponding stress matrix
  /// @param globdat Global data container
  void getStrainStress_(const Matrix &strains, const Matrix &stresses,
                        const Properties &globdat);

  /// @brief Store computed tangent properties
  /// @param strains Applied strain matrix
  /// @param stresses Resulting stress matrix
  /// @param globdat Global data container
  void storeTangentProps_(const Matrix &strains, const Matrix &stresses,
                          const Properties &globdat);

  /// @brief Extract properties via matrix condensation
  /// @param strains Applied strain matrix
  /// @param stresses Resulting stress matrix
  /// @param globdat Global data container
  void condenseMatrix_(const Matrix &strains, const Matrix &stresses,
                       const Properties &globdat);
  /// @}

protected:
  /// @brief Protected destructor
  virtual ~TangentOutputModule();

private:
  /// @name Configuration parameters
  /// @{
  String mode_;      ///< Calculation mode ("finDiff" or "matCond")
  idx_t rank_;       ///< Spatial dimension
  double thickness_; ///< Material thickness (for 2D)
  double perturb_;   ///< Perturbation magnitude for finite differences
  /// @}

  /// @name Analysis components
  /// @{
  Ref<Model> masterModel_;             ///< Reference model for analysis
  Ref<Function> sampleCond_;           ///< Sampling condition function
  Ref<GroupOutputModule> groupUpdate_; ///< Group output for boundary data
  Ref<SolverModule> solver_;           ///< Solver for finite difference method
  Ref<Constraints> cons_;              ///< Constraints for matrix condensation
  /// @}

  /// @name Data specifications
  /// @{
  StringVector strains_;  ///< Strain measure expressions
  StringVector stresses_; ///< Stress measure expressions
  StringVector sizes_;    ///< Size measure expressions
  IdxMatrix strainDofs_;  ///< DOF indices for strain application
  /// @}
};
