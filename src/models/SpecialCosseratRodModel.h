/**
 * @file specialCosseratRodModel.h
 * @author Til Gärtner
 * @brief Special Cosserat rod model implementation
 *
 * This model implements a special Cosserat rod finite element formulation
 * (also called Simo-Reissner rod or nonlinear Timoshenko beam).
 * It provides geometrically exact rod kinematics with rotational and
 * translational degrees of freedom.
 */

#pragma once

#include <jem/base/Object.h>

#include <materials/Material.h>
#include <materials/MaterialFactory.h>

#include <jem/base/Array.h>
#include <jem/base/IllegalInputException.h>
#include <jem/base/System.h>
#include <jem/numeric/Quaternion.h>
#include <jem/numeric/algebra.h>
#include <jem/numeric/algebra/matmul.h>
#include <jem/util/Properties.h>
#include <jem/util/StringUtils.h>

#include <jive/Array.h>
#include <jive/algebra/AbstractMatrix.h>
#include <jive/algebra/FlexMatrixBuilder.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/app/Names.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/fem/NodeGroup.h>
#include <jive/fem/NodeSet.h>
#include <jive/geom/ShapeFactory.h>
#include <jive/geom/StdShape.h>
#include <jive/implict/SolverInfo.h>
#include <jive/model/Actions.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/StateVector.h>
#include <jive/util/Assignable.h>
#include <jive/util/Globdat.h>
#include <jive/util/Printer.h>
#include <jive/util/XDofSpace.h>
#include <jive/util/XTable.h>
#include <jive/util/utilities.h>

#include <math.h>

#include "misc/Line3D.h"
#include "utils/helpers.h"
#include "utils/testing.h"

using jem::Slice;
using jem::SliceFrom;
using jem::numeric::matmul;
using jem::numeric::MatmulChain;
using jem::numeric::norm2;
using jem::numeric::Quaternion;
using jem::util::Properties;

using jive::BoolVector;
using jive::algebra::AbstractMatrix;
using jive::algebra::FlexMBuilder;
using jive::algebra::MatrixBuilder;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::fem::newNodeGroup;
using jive::fem::NodeGroup;
using jive::fem::NodeSet;
using jive::geom::IShape;
using jive::geom::ShapeFactory;
using jive::model::Model;
using jive::util::Assignable;
using jive::util::DofSpace;
using jive::util::Globdat;
using jive::util::joinNames;
using jive::util::XDofSpace;
using jive::util::XTable;

using jive_helpers::e3;
using jive_helpers::expVec;
using jive_helpers::eye;
using jive_helpers::skew;
using jive_helpers::unskew;
using jive_helpers::vec2mat;

//-----------------------------------------------------------------------
//   class SpecialCosseratRodModel
//-----------------------------------------------------------------------

/**
 * @class SpecialCosseratRodModel
 * @brief Special Cosserat rod finite element model with geometrically exact kinematics
 *
 * The SpecialCosseratRodModel implements a geometrically exact rod formulation
 * based on the Cosserat theory (Simo-Reissner rod). It features 6 DOF per node
 * (3 translational + 3 rotational) and supports large deformations, rotations,
 * and complex material behavior including plasticity.
 *
 * Features:
 * - Geometrically exact rod kinematics with 6 DOF per node
 * - Large deformation and rotation capabilities
 * - Material integration with plasticity support
 * - Hinge connection modeling
 * - Gyroscopic effects for dynamic analysis
 * - Initial strain and rotation specification
 * - Energy calculation (potential and dissipated)
 * - Strain and stress output tables
 *
 * @see [Reissner (1981)](https://doi.org/10.1007/BF00946983)
 * @see [Simo, Vu-Quoc (1986)](https://doi.org/10.1016/0045-7825(86)90079-4)
 * @see [Crisfield, Jelenić (1999)](https://doi.org/10.1098/rspa.1999.0352)
 * @see [Antman (2005)](https://doi.org/10.1007/0-387-27649-1)
 * @see [Eugster (2015)](https://doi.org/10.1007/978-3-319-16495-3)
 */
class SpecialCosseratRodModel : public Model
{
public:
  /// @name Property identifiers
  /// @{
  static const char *TYPE_NAME;         ///< Model type name
  static const char *TRANS_DOF_DEFAULT; ///< Default translational DOF prefix
  static const char *ROT_DOF_DEFAULT;   ///< Default rotational DOF prefix
  static const char *TRANS_DOF_NAMES;   ///< Translational DOF names property
  static const char *ROT_DOF_NAMES;     ///< Rotational DOF names property
  static const char *SYMMETRIC_ONLY;    ///< Symmetric tangent stiffness property
  static const char *MATERIAL_Y_DIR;    ///< Material y-direction property
  static const char *GIVEN_NODES;       ///< Given direction nodes property
  static const char *GIVEN_DIRS;        ///< Given directions property
  static const char *LUMPED_MASS;       ///< Lumped mass property
  static const char *HINGES;            ///< Hinges property
  /// @}

  /// @name DOF constants
  /// @{
  static const idx_t TRANS_DOF_COUNT; ///< Number of translational DOFs
  static const idx_t ROT_DOF_COUNT;   ///< Number of rotational DOFs
  static const Slice TRANS_PART;      ///< Translational DOF slice
  static const Slice ROT_PART;        ///< Rotational DOF slice
  /// @}

  JEM_DECLARE_CLASS(SpecialCosseratRodModel, Model);

  /// @brief Constructor
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  explicit SpecialCosseratRodModel(const String &name,
                                   const Properties &conf,
                                   const Properties &props,
                                   const Properties &globdat);

  /// @brief Handle model actions
  /// @param action Action name
  /// @param params Action parameters
  /// @param globdat Global data container
  /// @return true if action was handled
  virtual bool takeAction(const String &action,
                          const Properties &params,
                          const Properties &globdat) override;

  /// @brief Create new SpecialCosseratRodModel instance
  /// @param name Model name
  /// @param conf Actually used configuration properties (output)
  /// @param props User-specified model properties
  /// @param globdat Global data container
  /// @return New model instance
  static Ref<Model> makeNew(const String &name,
                            const Properties &conf,
                            const Properties &props,
                            const Properties &globdat);

  /// @brief Declare model type to factory
  static void declare();

private:
  /// @brief Assemble stiffness matrix and internal forces
  /// @param mbld Tangent stiffness matrix builder
  /// @param fint Internal force vector
  /// @param disp Current DOF values
  /// @param loadCase Load case identifier
  void assemble_(MatrixBuilder &mbld,
                 const Vector &fint,
                 const Vector &disp,
                 const String &loadCase = "") const;

  /// @brief Construct internal force vector only
  /// @param fint Internal force vector
  /// @param disp Current DOF values
  /// @param loadCase Load case identifier
  void assemble_(const Vector &fint,
                 const Vector &disp,
                 const String &loadCase = "") const;

  /// @brief Construct gyroscopic forces (omega x Theta*omega)
  /// @param fint Gyroscopic force vector
  /// @param velo Current DOF velocities
  /// @param mass Current mass matrix
  void assembleGyro_(const Vector &fint,
                     const Vector &velo,
                     const Ref<AbstractMatrix> mass) const;

  /// @brief Assemble mass matrix
  /// @param mbld Mass matrix builder
  /// @param disp Current DOF values
  void assembleM_(MatrixBuilder &mbld,
                  Vector &disp) const;

  /// @brief Fill table with strain values per element
  /// @param strain_table Output strain table
  /// @param weights Table weights
  /// @param disp Current displacements
  /// @param mat_vals flag for the material frame of reference
  /// @note If mat_vals is true, the strain values are in the material frame of
  /// reference, otherwise in the spatial frame of reference
  void getStrainTable_(XTable &strain_table,
                       const Vector &weights,
                       const Vector &disp,
                       const bool mat_vals = false);

  /// @brief Fill table with material/plastic strain values per element
  /// @param mat_table Output material table
  /// @param weights Table weights
  /// @param name Table name
  void getMaterialTable_(XTable &mat_table,
                         const Vector &weights,
                         const String &name);

  /// @brief Fill table with stress values per element
  /// @param stress_table Output stress table
  /// @param weights Table weights
  /// @param disp Current displacements
  /// @param mat_vals flag for the material frame of reference
  /// @note If mat_vals is true, the strain values are in the material frame of
  /// reference, otherwise in the spatial frame of reference
  void getStressTable_(XTable &stress_table,
                       const Vector &weights,
                       const Vector &disp,
                       const bool mat_vals = false);

  /// @brief Initialize rotation of elements
  void initRotation_();

  /// @brief Initialize initial strain of elements
  void initStrain_();

  /// @brief Get the geometric stiffness matrix
  /// @param B B-matrix at integration points
  /// @param stresses Spatial stress components at integration points
  /// @param nodePhi_0 Location of the nodes
  /// @param nodeU Translational displacement of nodes
  void getGeomtericStiffness_(const Cubix &B,
                              const Matrix &stresses,
                              const Matrix &nodePhi_0,
                              const Matrix &nodeU) const;

  /// @brief Get the strains in the integration points of an element
  /// @param strains Strain components at integration points
  /// @param w Integration point weights
  /// @param nodePhi_0 Location of the nodes
  /// @param nodeU Translational displacement of nodes
  /// @param nodeLambda Rotational orientation of nodes
  /// @param ie Element index
  /// @param spatial use inertial frame of reference (true) or spatial frame of reference (false)
  void getStrains_(const Matrix &strains,
                   const Vector &w,
                   const Matrix &nodePhi_0,
                   const Matrix &nodeU,
                   const Cubix &nodeLambda,
                   const idx_t ie,
                   const bool spatial = true) const;

  /// @brief Get the stresses in the integration points of an element
  /// @param stresses Stress components at integration points
  /// @param w Integration point weights
  /// @param nodePhi_0 Location of the nodes
  /// @param nodeU Translational displacement of nodes
  /// @param nodeLambda Rotational orientation of nodes
  /// @param ie Element index
  /// @param spatial use inertial frame of reference (true) or spatial frame of reference (false)
  /// @param loadCase Load case identifier
  void getStresses_(const Matrix &stresses,
                    const Vector &w,
                    const Matrix &nodePhi_0,
                    const Matrix &nodeU,
                    const Cubix &nodeLambda,
                    const idx_t ie,
                    const bool spatial = true,
                    const String &loadCase = "") const;

  /// @brief Format the displacements nicely
  /// @param nodePhi_0 Node positions
  /// @param nodeU Node displacements
  /// @param nodeLambda Node rotations
  /// @param disp Displacement vector
  /// @param inodes Node indices
  void getDisplacments_(const Matrix &nodePhi_0,
                        const Matrix &nodeU,
                        const Cubix &nodeLambda,
                        const Vector &disp,
                        const IdxVector &inodes) const;

  /// @brief Calculate potential energy of the rod
  /// @param disp Displacement vector
  /// @return Potential energy value
  double getPotentialEnergy_(const Vector &disp) const;

  /// @brief Calculate potential energy and fill table
  /// @param energy_table Output energy table
  /// @param table_weights Table weights
  /// @param disp Displacement vector
  void getPotentialEnergy_(XTable &energy_table,
                           const Vector &table_weights,
                           const Vector &disp) const;

  /// @brief Calculate dissipated energy of the material
  /// @param disp Displacement vector
  /// @return Dissipated energy value
  double getDissipatedEnergy_(const Vector &disp) const;

  /// @brief Calculate dissipated energy and fill table
  /// @param energy_table Output energy table
  /// @param table_weights Table weights
  /// @param disp Displacement vector
  void getDissipatedEnergy_(XTable &energy_table,
                            const Vector &table_weights,
                            const Vector &disp) const;

private:
  Assignable<ElementGroup> rodElems_; ///< Rod element group
  IdxVector rodNodes_;                ///< Rod node indices
  Assignable<ElementSet> allElems_;   ///< All elements
  Assignable<NodeSet> allNodes_;      ///< All nodes

  Ref<DofSpace> dofs_;     ///< DOF space
  Ref<Line3D> shapeK_;     ///< Shape functions for stiffness
  Ref<Line3D> shapeM_;     ///< Shape functions for mass
  Ref<Material> material_; ///< Material model
  Ref<Model> hinges_;      ///< Hinge model

  IdxVector transTypes_; ///< Translational DOF types
  IdxVector rotTypes_;   ///< Rotational DOF types
  IdxVector jtypes_;     ///< Joint DOF types

  bool symOnly_;        ///< Symmetric tangent stiffness flag
  Vector thickFact_;    ///< Thickening factors
  Vector materialYDir_; ///< Material y-direction

  IdxVector givenNodes_; ///< Nodes with given directions
  Matrix givenDirs_;     ///< Given directions for nodes

  Cubix LambdaN_;    ///< Reference rotations per node
  Cubix matStrain0_; ///< Initial strain configuration
};
