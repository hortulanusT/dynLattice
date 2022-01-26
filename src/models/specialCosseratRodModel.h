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
  void                    assembleFint_
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
   * @brief update the rotation of the elements
   */
  void                    update_rot_ 
    ( const Vector& disp );

  /**
   * @brief initializes the initial curvature of the elements
   */
  void                    init_strain_ ();

  /**
   * @brief Get the spatial stiffness matrix per integration point
   */
  void get_spatialC_ // LATER externalise to material?
    ( const Cubix&        c,      ///< c(.,.,i), where it refers to the spatial stiffness matrix at the i-th integration point
      const Vector&       w,      ///< integration point weights
      const idx_t&        ielem,  ///< element to investigate (local index)
      const Matrix&       theta) const; ///< matrix with the rotational displacments for the nodes of this element

  /**
   * @brief Get the geometric stiffness matrix
   */
  void get_geomStiff_
    ( const Cubix&        B,              ///< B(.,.,i), where it refers to the B-matrix at the i-th integration point
      const Vector&       w,              ///< integration point weights
      const Matrix&       spat_stresses,  ///< stress(i,j), stress component i at the j-th integration points
      const Matrix&       coords,         ///< translational displacements
      const Matrix&       u ) const;      ///< translational displacements

  /**
   * @brief Get the strains in the integration points of an element
   */
  void get_strains_
    ( const Matrix&       spat_strains, ///< strains(i,j), strain component i at the j-th integration points
      const Matrix&       mat_strains,  ///< strains(i,j), strain component i at the j-th integration points
      const Vector&       w,            ///< integration point weights
      const idx_t&        ielem,        ///< element to investigate (local index)
      const Matrix&       u,            ///< translational displacements
      const Matrix&       theta ) const; ///< rotational displacements

  /**
   * @brief Get the stresses in the integration points of an element
   */
  void get_stresses_
    ( const Matrix&       spat_stresses,  ///< stress(i,j), stress component i at the j-th integration points
      const Matrix&       mat_stresses,   ///< stress(i,j), stress component i at the j-th integration points
      const Vector&       w,              ///< integration point weights
      const idx_t&        ielem,          ///< element to investigate (local index)
      const Matrix&       u,              ///< translational displacements
      const Matrix&       theta ) const;  ///< rotational displacements

  /**
   * @brief Get the u and theta displacement for the given nodes 
   */
  void get_disps_
    ( const Matrix&       u,
      const Matrix&       theta,
      const IdxVector&    inodes,
      const Vector&       disp ) const;

 private: 
  Assignable<ElementGroup>egroup_;
  Assignable<NodeSet>     nodes_;
  Assignable<ElementSet>  elems_;
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
  Matrix                  C_material_;

  Quadix                  LambdaN_; ///< reference rotations per node per element; LambdaN_(.,.,i,j) is for the j-th node in the i-th element
  Cubix                   mat_strain0_; ///< strains for the undeformed configuration; mat_strain0_(i,j,k) refers to the i-th strain in the k-th element on the j-th integration point
};