/**
 * @file LatticeModel.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief LatticeModel for collection of rods
 * @version 0.1
 * @date 2022-04-26
 * 
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 * 
 */

#pragma once

#include <jem/base/IllegalInputException.h>
#include <jem/base/System.h>
#include <jem/base/CString.h>
#include <jem/base/array/Array.h>
#include <jem/util/Properties.h>
#include <jem/util/ArrayBuffer.h>
#include <jive/model/Model.h>
#include <jive/model/ModelFactory.h>
#include <jive/fem/ElementGroup.h>
#include <jive/fem/ElementSet.h>
#include <jive/util/Assignable.h>
#include <jive/util/utilities.h>

using jem::newInstance;
using jem::Ref;
using jem::Array;
using jem::idx_t;
using jem::util::ArrayBuffer;
using jive::String;
using jive::Properties;
using jive::model::Model;
using jive::model::ModelFactory;
using jive::fem::ElementGroup;
using jive::fem::ElementSet;
using jive::util::Assignable;

class LatticeModel : public Model
{
 public:
  static const char*      TYPE_NAME;
  static const char*      CHILD_PROPS;
  static const char*      NAME_PREFIX;

  explicit                LatticeModel

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
  Array<Ref<Model>>            children_;
};