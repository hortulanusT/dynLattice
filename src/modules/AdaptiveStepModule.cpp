#include "modules/AdaptiveStepModule.h"
#include "utils/testing.h"

JEM_DEFINE_CLASS(AdaptiveStepModule);

//=======================================================================
//   class AdaptiveStepModule
//=======================================================================

const char *AdaptiveStepModule::TYPE_NAME = "AdaptiveStep";

//-----------------------------------------------------------------------
//   constructor & destructor
//-----------------------------------------------------------------------

AdaptiveStepModule::AdaptiveStepModule(const String &name,
                                       Ref<NonlinModule> solver) : Super(name), solver_(solver)

{
  oldLoadScale_ = loadScale_ = 0.;
  incr_ = minIncr_ = maxIncr_ = 0.;
  incrFact_ = 1.2;
  decrFact_ = 0.5;
}

AdaptiveStepModule::~AdaptiveStepModule()
{
}

//-----------------------------------------------------------------------
//   configure
//-----------------------------------------------------------------------

void AdaptiveStepModule::configure

    (const Properties &props,
     const Properties &globdat)
{
  using jive::implict::PropNames;

  Super::configure(props, globdat);
  solver_->configure(props, globdat);

  Properties myProps = props.findProps(myName_);

  myProps.find(incrFact_, "increaseFactor");
  myProps.find(decrFact_, "decreaseFactor");
}

//-----------------------------------------------------------------------
//   getConfig
//-----------------------------------------------------------------------

void AdaptiveStepModule::getConfig

    (const Properties &props,
     const Properties &globdat) const

{
  using jive::implict::PropNames;

  Super::getConfig(props, globdat);
  solver_->getConfig(props, globdat);

  Properties myProps = props.findProps(myName_);

  myProps.set("increaseFactor", incrFact_);
  myProps.set("decreaseFactor", decrFact_);
}

//-----------------------------------------------------------------------
//   advance
//-----------------------------------------------------------------------

void AdaptiveStepModule::advance(const Properties &globdat)
{
  Properties params;

  oldLoadScale_ = loadScale_;
  loadScale_ += incr_;

  Properties vars = Globdat::getVariables(globdat);
  vars.set(jive::model::RunvarNames::LOAD_SCALE, loadScale_);
  jem::System::info(myName_) << " ...Applying " << jive::model::RunvarNames::LOAD_SCALE << " of " << loadScale_ << "\n";

  Globdat::advanceStep(globdat);
  model_->takeAction(Actions::ADVANCE, params, globdat);
}

//-----------------------------------------------------------------------
//   init
//-----------------------------------------------------------------------

Module::Status AdaptiveStepModule::init(const Properties &conf,
                                        const Properties &props,
                                        const Properties &globdat)
{
  using jive::implict::PropNames;
  Properties myProps = props.findProps(myName_);
  Properties myConf = conf.findProps(myName_);

  try
  {
    myProps.get(incr_, PropNames::LOAD_INCR);
  }
  catch (const std::exception &e)
  {
    String incr;
    myProps.get(incr, PropNames::LOAD_INCR);
    incr_ = FuncUtils::evalExpr(incr, globdat);
  }
  minIncr_ = incr_ / 100.;
  maxIncr_ = incr_ * 100.;
  myProps.find(minIncr_, PropNames::MIN_INCR);
  myProps.find(maxIncr_, PropNames::MAX_INCR);

  myConf.set(PropNames::LOAD_INCR, incr_);
  myConf.set(PropNames::MIN_INCR, minIncr_);
  myConf.set(PropNames::MAX_INCR, maxIncr_);

  loadScale_ = 0.;
  Properties vars = Globdat::getVariables(globdat);
  vars.set(jive::model::RunvarNames::LOAD_SCALE, loadScale_);

  model_ = Model::get(globdat, getContext());
  dofs_ = DofSpace::get(globdat, getContext());

  return solver_->init(conf, props, globdat);
}

//-----------------------------------------------------------------------
//   solve
//-----------------------------------------------------------------------
void AdaptiveStepModule::solve(const Properties &info,
                               const Properties &globdat)
{
  try
  {
    solver_->solve(info, globdat);
  }
  catch (const jem::Exception &e)
  {
    jem::System::info(myName_) << e.name() << " occured in " << e.where() << "\n\t"
                               << e.what() << "\n";
#ifndef NDEBUG
    jem::System::debug(myName_) << "\n\n"
                                << e.getStackTrace() << "\n\n";
#endif
    info.set(SolverInfo::CONVERGED, false);
  }
}

//-----------------------------------------------------------------------
//   cancel
//-----------------------------------------------------------------------
void AdaptiveStepModule::cancel(const Properties &globdat)
{
  Properties params;

  loadScale_ = oldLoadScale_;
  Properties vars = Globdat::getVariables(globdat);
  vars.set(jive::model::RunvarNames::LOAD_SCALE, loadScale_);

  Globdat::restoreStep(globdat);
  StateVector::restoreNew(dofs_, globdat);
  model_->takeAction(Actions::CANCEL, params, globdat);
}

//-----------------------------------------------------------------------
//   commit
//-----------------------------------------------------------------------

bool AdaptiveStepModule::commit

    (const Properties &globdat)
{
  Properties params;
  bool accept;

  if (model_->takeAction(Actions::CHECK_COMMIT, params, globdat))
  {
    params.get(accept, ActionParams::ACCEPT);
  }
  else // if the model doesnt care accept the solution
  {
    accept = true;
  }

  // adapt step size
  if (accept)
  {
    incr_ = jem::min(maxIncr_, incr_ * incrFact_);
  }
  else if (incr_ == minIncr_) // if the step size is already minimal accept the current solution
  {
    jem::System::warn() << " ...Continuing with smallest possible step size\n";
    accept = true;
  }
  else
  {
    incr_ = jem::max(minIncr_, incr_ * decrFact_);
  }

  if (accept)
  {
    params.clear();
    model_->takeAction(Actions::COMMIT, params, globdat);
    Globdat::commitStep(globdat);
    StateVector::updateOld(dofs_, globdat);
  }

  jem::System::info(myName_) << " ...Adapting load step size to " << incr_ << "\n";
  if (incr_ == maxIncr_ && incr_ > minIncr_)
    jem::System::info(myName_) << " !!! Largest allowed load step !!!\n";
  if (incr_ == minIncr_ && incr_ < maxIncr_)
    jem::System::info(myName_) << " !!! Smallest allowed load step !!!\n";

  return accept;
}

//-----------------------------------------------------------------------
//   makeNew
//-----------------------------------------------------------------------

Ref<Module> AdaptiveStepModule::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  Ref<NonlinModule> solver = newInstance<NonlinModule>(jive::util::joinNames(name, "nonlin"));

  return newInstance<Self>(name, solver);
}

//-----------------------------------------------------------------------
//   declare
//-----------------------------------------------------------------------

void AdaptiveStepModule::declare()
{
  using jive::app::ModuleFactory;

  ModuleFactory::declare(TYPE_NAME, &AdaptiveStepModule::makeNew);
}
