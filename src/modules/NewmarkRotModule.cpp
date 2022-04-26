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
#include <jem/base/System.h>

const char* NewmarkRotModule::TYPE_NAME = "NewmarkRot";

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------

NewmarkRotModule::NewmarkRotModule

  ( const String&         name,
    int                   order,
    Ref<SolverModule>     solver ) :

    Super ( name, order, solver )
{
  jem::System::warn() << "> > > NewmarkRotModule created!\n";
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> NewmarkRotModule::makeNew

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat )
{
  using jive::util::joinNames;

  Ref<SolverModule>  solver =

    jive::implict::newSolverModule ( joinNames( name, jive::implict::PropNames::SOLVER ),
                      conf, props, globdat );

  Properties  myConf  = conf .makeProps ( name );
  Properties  myProps = props.findProps ( name );

  int         order   = 2;

  myProps.find ( order, jive::implict::PropNames::ORDER, 1, 2 );
  myConf .set  ( jive::implict::PropNames::ORDER, order );

  return newInstance<Self> ( name, order, solver );
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
  using jive::util::joinNames;
  
  const String         context = getContext ();

  Ref<DofSpace>        dofs    =

    DofSpace::get ( globdat, context );

  Ref<TimeStepper>  stepper =

    TSFactory::newInstance ( joinNames ( myName_, jive::implict::PropNames::TIME_STEPPER ), conf, props, globdat );

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