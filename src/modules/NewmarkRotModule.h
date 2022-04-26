/**
 * @file NewmarkRotModule.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief 
 * @version 0.1
 * @date 2022-04-25
 * 
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 * 
 */

#pragma once

#include <jem/util/Properties.h>
#include <jive/app/ModuleFactory.h>
#include <jive/util/DofSpace.h>
#include <jive/util/utilities.h>
#include <jive/implict/Names.h>
#include <jive/implict/TransientModel.h>
#include <jive/implict/NewmarkModule.h>
#include <jive/implict/TimeStepperFactory.h>
#include <jive/implict/TimeStepper.h>

using jem::newInstance;
using jive::String;
using jive::Ref;
using jive::Properties;
using jive::util::DofSpace;
using jive::app::Module;
using jive::model::Model;
using jive::implict::SolverModule;
using jive::implict::NewmarkModule;
using jive::implict::TimeStepper;
using jive::implict::TSFactory;
using jive::implict::TransientModel;

class NewmarkRotModule : public NewmarkModule
{
 public:
 
  typedef   NewmarkRotModule  Self;
  typedef   NewmarkModule     Super;

  static const char*      TYPE_NAME;

                          NewmarkRotModule

    ( const String&         name,
      int                   order,
      Ref<SolverModule>     solver );

  static Ref<Module>      makeNew

    ( const String&         name,
      const Properties&     conf,
      const Properties&     props,
      const Properties&     globdat );

  static void             declare ();

 protected:

  virtual Ref<Model>      newTransModel_

    ( const Properties&     conf,
      const Properties&     props,
      const Properties&     globdat ) override;
};