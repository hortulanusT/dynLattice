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

#include "NewmarkRotModule.h"

const char* NewmarkRotModule::TYPE_NAME = "NewmarkRot";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------

NewmarkRotModule::NewmarkRotModule

  ( const String&         name,
    int                   order,
    Ref<SolverModule>     solver ) :

    Super ( name, order, solver )
{}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> NewmarkRotModule::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )
{
  return Super::makeNew( name, conf, props, globdat );
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void NewmarkRotModule::declare ()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare ( TYPE_NAME,  & makeNew );
}

//-----------------------------------------------------------------------
//   newTransModel_
//-----------------------------------------------------------------------

Ref<Model> NewmarkRotModule::newTransModel_

  ( const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )

{
  const String         context = getContext ();

  Ref<DofSpace>        dofs    =

    DofSpace::get ( globdat, context );

  Ref<TimeStepper>  stepper =

    TSFactory::newInstance ( myName_, conf, props, globdat );

  Ref<TransientModel>  tmodel  =

    newInstance<TransientModel> (
      myName_,
      Model::get ( globdat, context ),
      stepper,
      TransientModel::MANAGED
    );

  tmodel ->setContext ( context );
  tmodel ->init       ( globdat );

  return tmodel;
}