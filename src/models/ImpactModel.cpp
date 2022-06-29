#include <jem/base/ClassTemplate.h>
#include "models/ImpactModel.h"

JEM_DEFINE_CLASS( ImpactModel );

//=======================================================================
//   class ImpactModel
//=======================================================================

//-----------------------------------------------------------------------
//   static data
//-----------------------------------------------------------------------


const char*  ImpactModel::TYPE_NAME   = "ImpactBC";
const char*  ImpactModel::NODES_PROP  = "nodeGroups";
const char*  ImpactModel::DOF_PROP    = "dofs";
const char*  ImpactModel::WEIGHTS_PROP= "weights";


//-----------------------------------------------------------------------
//   constructors & destructor
//-----------------------------------------------------------------------


ImpactModel::ImpactModel

  ( const String&      name,
    const Properties&  conf,
    const Properties&  props,
    const Properties&  globdat  ) :

    Super     ( name )

{
  // Get the Properties associated with this model
  Properties  myProps = props.findProps ( myName_ );
  Properties  myConf  = conf .makeProps ( myName_ ); 

  // get the node Groups
  myProps.get ( nodeGroups_, NODES_PROP );
  myConf .set ( NODES_PROP, nodeGroups_ );
  // get the dofs
  myProps.get ( dofNames_, DOF_PROP );
  myConf .set ( DOF_PROP, dofNames_ );
  // get the weights
  myProps.get ( weights_, WEIGHTS_PROP );
  myConf .set ( WEIGHTS_PROP, weights_ );

  JEM_PRECHECK2( nodeGroups_.size() == dofNames_.size(), "NodeGroups and Dofs need to have the same length! ");
  JEM_PRECHECK2( nodeGroups_.size() == weights_.size(), "NodeGroups and Weights need to have the same length! ");

  nodes_ = NodeSet::get     ( globdat, getContext() );
  dofs_  = DofSpace::get    ( globdat, getContext() );
  cons_  = Constraints::get ( dofs_, globdat );
}


//-----------------------------------------------------------------------
//   takeAction
//-----------------------------------------------------------------------


bool ImpactModel::takeAction

  ( const String&      action,
    const Properties&  params,
    const Properties&  globdat )

{
  if ( action == Actions::GET_CONSTRAINTS )
  {
    String  varName;
    double  force;
    
    Assignable<NodeGroup> group;
    IdxVector             inodes;

    idx_t                 itype;
    IdxVector             idofs;

    for (idx_t i = 0; i < nodeGroups_.size(); i++)
    {
      if (Globdat::hasVariable ( nodeGroups_[i], globdat ) )
      {
        Globdat::getVariables( nodeGroups_[i], globdat ).getProps( "resp" ).get( force, dofNames_[i] );

        group = NodeGroup::get( nodeGroups_[i], nodes_, globdat, getContext() );
        inodes.resize( group.size() );
        inodes = group.getIndices();

        itype  = dofs_->findType ( dofNames_[i] );
        idofs.resize( group.size() );
        dofs_->getDofIndices( idofs, inodes, itype );

        for (idx_t idof : idofs )
          cons_->addConstraint ( idof, -1. * force/weights_[i] );
      }
      else
        jem::System::warn() << "No Force data for " << nodeGroups_[i] << " in global Database!\n";
    }
  }

  return false;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------


Ref<Model> ImpactModel::makeNew

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


void ImpactModel::declare ()
{
  using jive::model::ModelFactory;

  ModelFactory::declare ( TYPE_NAME,  & makeNew );
  ModelFactory::declare ( CLASS_NAME, & makeNew );
}