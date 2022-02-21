/*
 * Copyright (C) 2021 TU Delft. All rights reserved.
 *  
 * This class implements a special Cosserat Rod model
 * (also called Simo Reissner Rod or nonlinear
 * Timoshenko Rod)
 *
 * Author: T. Gaertner (t.gartner@tudelft.nl)
 * Date: July 21
 *
 */

#pragma once

#include <jem/base/Array.h>
#include <jem/base/System.h>
#include <jem/util/Properties.h>
#include <jem/numeric/algebra/matmul.h>
#include <jem/numeric/algebra.h>
#include <jem/numeric/Quaternion.h>

#include <jive/Array.h>
#include <jive/algebra/MatrixBuilder.h>
#include <jive/model/ModelFactory.h>
#include <jive/model/Model.h>
#include <jive/model/Actions.h>
#include <jive/model/StateVector.h>
#include <jive/util/XTable.h>
#include <jive/util/Assignable.h>
#include <jive/util/XDofSpace.h>
#include <jive/util/Printer.h>
#include <jive/fem/NodeSet.h>
#include <jive/fem/ElementSet.h>
#include <jive/fem/ElementGroup.h>

#include <math.h>

#include "Line3D.h"
#include "testing.h"
#include "helpers.h"

using jem::Slice;
using jem::SliceFrom;
using jem::util::Properties;
using jem::numeric::norm2;
using jem::numeric::matmul;
using jem::numeric::MatmulChain;
using jem::numeric::Quaternion;

using jive::algebra::MatrixBuilder;
using jive::model::Model;
using jive::util::XTable;
using jive::util::Assignable;
using jive::util::XDofSpace;
using jive::util::DofSpace;
using jive::fem::NodeSet;
using jive::fem::ElementSet;
using jive::fem::ElementGroup;
using jive::geom::IShape;
using jive::geom::ShapeFactory;

using namespace jive_helpers;

/**
 * @brief class, that implements a special cosserat Rod model
 * 
 * This class implements a special cosserat rod finite element class
 * strongly inspired by Simo/Vu-Quoc '85
 * 
 */
class specialCosseratRodModel : public Model
{
 public:

  static const char*      TYPE_NAME;
  static const char*      SHAPE_IDENTIFIER;
  static const char*      TRANS_DOF_DEFAULT;
  static const char*      ROT_DOF_DEFAULT;
  static const char*      YOUNGS_MODULUS;
  static const char*      SHEAR_MODULUS;
  static const char*      POISSION_RATIO;
  static const char*      AREA;
  static const char*      AREA_MOMENT;
  static const char*      SHEAR_FACTOR;
  static const char*      POLAR_MOMENT;
  static const char*      TRANS_DOF_NAMES;
  static const char*      ROT_DOF_NAMES;
  static const char*      INCREMENTAL;
  static const char*      MATERIAL_Y_DIR;
  static const idx_t      TRANS_DOF_COUNT;
  static const idx_t      ROT_DOF_COUNT;
  static const Slice      TRANS_PART;
  static const Slice      ROT_PART;

  explicit                specialCosseratRodModel

    ( const String&         name,
      const Properties&     conf,
      const Properties&     props,
      const Properties&     globdat );

  virtual bool            takeAction

    ( const String&         action,
      const Properties&     params,
      const Properties&     globdat );

  static Ref<Model>       makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat );

  static void             declare ();

 private:
  /**
   * @brief assemble the stiffness matrix
   * @param[out] mbld tanget stiffness matrix (via MatrixBuilder object)
   * @param[out] fint internal force Vector
   * @param[in]  disp current values for the DOFs 
   * @param[in]  dispOld last values for the DOFs 
   */
  void                    assemble_
  ( MatrixBuilder&        mbld,
    const Vector&         fint,
    const Vector&         disp,
    const Vector&         dispOld  ) const;

  /**
   * @brief construct the internal force vector
   * @param[out] fint internal force Vector
   * @param[in]  disp current values for the DOFs 
   * @param[in]  dispOld last values for the DOFs 
   */
  void                    assemble_
  ( const Vector&         fint,
    const Vector&         disp,
    const Vector&         dispOld  ) const;

  /**
   * @brief fill the table with the strain values per element 
   */
  void                    get_strain_table_

  ( XTable&               strain_table,
    const Vector&         weights,
    const Vector&         disp,
    const bool            mat_vals = false );

  /**
   * @brief fill the table with the stress values per element 
   */
  void                    get_stress_table_

  ( XTable&               stress_table,
    const Vector&         weights,
    const Vector&         disp,
    const bool            mat_vals = false  );

  /**
   * @brief initializes the rotation of the elements
   */
  void                    init_rot_ ();

  /**
   * @brief initializes the initial strain of the elements
   */
  void                    init_strain_ ();

  /**
   * @brief Get the geometric stiffness matrix
   */
  void get_geomStiff_
    ( const Cubix&        B,              ///< B(.,.,i), where it refers to the B-matrix at the i-th integration point
      const Matrix&       stresses,       ///< spatial stress(i,j), stress component i at the j-th integration points
      const Matrix&       nodePhi_0,      ///< location of the nodes
      const Matrix&       nodeU ) const;  ///< nodeU(.,j), translational displacement j-th node 

  /**
   * @brief Get the strains in the integration points of an element
   */
  void get_strains_
    ( const Matrix&       strains,        ///< strains(i,j), stress component i at the j-th integration points
      const Vector&       w,              ///< integration point weights
      const Matrix&       nodePhi_0,      ///< nodePhi_0(.,j), location of the j-th node
      const Matrix&       nodeU,          ///< nodeU(.,j), translational displacement j-th node 
      const Matrix&       nodeTheta,      ///< nodeTheta(.,j), rotational displacement j-th node
      const idx_t         ie,
      const bool          spatial = true ) const;  ///< rotational displacements

  /**
   * @brief Get the stresses in the integration points of an element
   */
  void get_stresses_
    ( const Matrix&       stresses,       ///< stress(i,j), stress component i at the j-th integration points
      const Vector&       w,              ///< integration point weights
      const Matrix&       nodePhi_0,      ///< nodePhi_0(.,j), location of the j-th node
      const Matrix&       nodeU,          ///< nodeU(.,j), translational displacement j-th node 
      const Matrix&       nodeTheta,      ///< nodeTheta(.,j), rotational displacement j-th node
      const idx_t         ie,
      const bool          spatial = true ) const;  ///< rotational displacements

  /**
   * @brief format the displacements nicely
   */
  void get_disps_
    ( const Matrix&       nodePhi_0,
      const Matrix&       nodeU,
      const Matrix&       nodeTheta,
      const Vector&       disp  ) const;

 private: 
  Assignable<ElementGroup>rodGroup_;
  Assignable<NodeSet>     allNodes_;
  Assignable<ElementSet>  allElems_;
  Ref<DofSpace>           dofs_;
  Ref<Line3D>             shape_;
  IdxVector               trans_types_;
  IdxVector               rot_types_;
  IdxVector               jtypes_;
  bool                    incremental_;

  double                  young_;
  double                  area_;
  double                  shearMod_;
  double                  areaMoment_;
  double                  polarMoment_;
  double                  shearParam_;

  Vector                  material_ey_;
  Matrix                  materialC_;

  Quadix                  LambdaN_; ///< reference rotations per node per element; LambdaN_(.,.,j,k) is for the j-th integration point at the k-th element
  Cubix                   mat_strain0_; ///< strains for the undeformed configuration; mat_strain0_(i,j,k) refers to the i-th strain in the k-th element on the j-th integration point
};