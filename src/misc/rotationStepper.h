/**
 * @file rotationStepper.h
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief contains a class, that enables correct calculation of rotational velocities
 * @version 0.1
 * @date 2022-04-25
 * 
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 * 
 */
#pragma once

#include <jive/implict/NewmarkStepper.h>
#include <jive/implict/TimeStepperFactory.h>

using jem::newInstance;

using jive::Ref;
using jive::String;
using jive::Properties;
using jive::implict::TimeStepper;
using jive::implict::NewmarkStepper;
using jive::implict::TSFactory;

class rotationStepper: public NewmarkStepper
{
 public:

  typedef   rotationStepper   Self;
  typedef   NewmarkStepper    Super;

  static const char*          TYPE_NAME;

                            rotationStepper

    ( const String&           name,
      const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  virtual void              updateState

    ( const Properties&       globdat ) override;

  static Ref<TimeStepper>   makeNew
  
    ( const String&           name,
      const Properties&       conf,
      const Properties&       props,
      const Properties&       globdat );

  static void               declare ();
};