/**
 * @file InitLoadModel.cpp
 * @author Til Gärtner (t.gartner@tudelft.nl)
 * @brief InitLoadModel for initial loads
 * @version 0.1
 * @date 2022-04-26
 * 
 * @copyright Copyright (C) 2022 TU Delft. All rights reserved.
 * 
 */

#include "models/InitLoadModel.h"
#include "utils/testing.h"

//=======================================================================
//   class InitLoadModel
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------
const char*   InitLoadModel::TYPE_NAME   = "InitLoad";
const char*   InitLoadModel::DISP_GROUPS = "dispGroups";
const char*   InitLoadModel::DISP_DOFS   = "dispDofs";     
const char*   InitLoadModel::DISP_VALS   = "dispVals"; 
const char*   InitLoadModel::VELO_GROUPS = "veloGroups";
const char*   InitLoadModel::VELO_DOFS   = "veloDofs";     
const char*   InitLoadModel::VELO_VALS   = "veloVals";     

//-----------------------------------------------------------------------
//   constructor
//-----------------------------------------------------------------------
InitLoadModel::InitLoadModel

( const String&         name,
  const Properties&     conf,
  const Properties&     props,
  const Properties&     globdat ) :
        Model ( name )
{  
  Properties    myProps = props.getProps ( myName_ );
  Properties    myConf  = conf .makeProps( myName_ );

  if (myProps.find( dgroups_, DISP_GROUPS ))
  {
    myProps.get( ddofs_, DISP_DOFS );
    myProps.get( dvals_, DISP_VALS );

    JEM_PRECHECK2( dgroups_.size() == ddofs_.size(), "groups and dofs need to have the same size!");  
    JEM_PRECHECK2( dgroups_.size() == dvals_.size(), "groups and vals need to have the same size!"); 

    myConf.set( DISP_GROUPS, dgroups_ );
    myConf.set( DISP_DOFS, ddofs_ );
    myConf.set( DISP_VALS, dvals_ );
  }

  
  if (myProps.find( vgroups_, VELO_GROUPS ))
  {
    myProps.get( vdofs_, VELO_DOFS );
    myProps.get( vvals_, VELO_VALS );

    JEM_PRECHECK2( vgroups_.size() == vdofs_.size(), "groups and dofs need to have the same size!");  
    JEM_PRECHECK2( vgroups_.size() == vvals_.size(), "groups and vals need to have the same size!"); 

    myConf.set( VELO_GROUPS, vgroups_ );
    myConf.set( VELO_DOFS, vdofs_ );
    myConf.set( VELO_VALS, vvals_ );
  }
}

//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------

bool InitLoadModel::takeAction

  ( const String&      action,
    const Properties&  params,
    const Properties&  globdat )

{
  if (action == Actions::INIT)
  {
    init_ ( globdat );
  }

  return false;
}

//-----------------------------------------------------------------------
//   init_
//-----------------------------------------------------------------------

void InitLoadModel::init_ 

  ( const Properties& globdat )

{
  Ref<DofSpace>         dofs;
  Assignable<NodeSet>   nodes;
  Vector                disp;
  Vector                velo;

  Assignable<NodeGroup> group;
  idx_t                 jdof;
  IdxVector             gdofs;
  
  dofs            = DofSpace::get( globdat, getContext() );
  nodes           = NodeSet::get( globdat, getContext() );
  StateVector::get( disp, jive::model::STATE0, dofs, globdat );
  StateVector::get( velo, jive::model::STATE1, dofs, globdat );

  for (idx_t igroup = 0; igroup < dgroups_.size(); igroup++)
  {
    group = NodeGroup::get( dgroups_[igroup], nodes, globdat, getContext() );

    gdofs.resize( group.size() );
    jdof = dofs->getTypeIndex( ddofs_[igroup] );
    dofs->getDofIndices( gdofs, group.getIndices(), jdof );

    for (idx_t idof = 0; idof < gdofs.size(); idof++) 
      disp[gdofs[idof]] = dvals_[igroup];
  }

  for (idx_t igroup = 0; igroup < vgroups_.size(); igroup++)
  {
    group = NodeGroup::get( vgroups_[igroup], nodes, globdat, getContext() );

    gdofs.resize( group.size() );
    jdof = dofs->getTypeIndex( vdofs_[igroup] );
    dofs->getDofIndices( gdofs, group.getIndices(), jdof );

    for (idx_t idof = 0; idof < gdofs.size(); idof++) 
      velo[gdofs[idof]] = vvals_[igroup];
  }
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------
Ref<Model> InitLoadModel::makeNew

( const String&      name,
  const Properties&  conf,
  const Properties&  props,
  const Properties&  globdat )
{
  return newInstance<InitLoadModel> ( name, conf, props, globdat );
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------
void InitLoadModel::declare ()
{
  using jive::model::ModelFactory;

  ModelFactory::declare ( TYPE_NAME, & makeNew );
}