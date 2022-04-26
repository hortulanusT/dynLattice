/**
 * @file rotationStepper.cpp
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief contains a class, that enables correct calculation of rotational velocities
 * @version 0.1
 * @date 2022-04-25
 * 
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 * 
 */

#include "rotationStepper.h"
#include <jem/base/System.h>

const char*   rotationStepper::TYPE_NAME = "rotationStepper";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------

rotationStepper::rotationStepper

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat ) :

    Super ( name, conf, props, globdat )
{}


//-----------------------------------------------------------------------
//   updateState
//-----------------------------------------------------------------------

void          rotationStepper::updateState

  ( const Properties&  globdat ) 
{
  Super::updateState(globdat);

  jem::System::warn() << "> > > STATE VARIABLES UPDATED! \n ";
}


//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<TimeStepper> rotationStepper::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  return newInstance<Self> ( name, conf, props, globdat );
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void rotationStepper::declare ()
{
  TSFactory::declare ( TYPE_NAME,  Self::makeNew );
}