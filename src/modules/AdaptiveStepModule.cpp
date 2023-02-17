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
  incr_ = 0.;
  minIncr_ = 0.;
  maxIncr_ = 0.;
  optIter_ = 4;
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

  myProps.find(optIter_, "optIter");
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

  myProps.set("optIter", optIter_);
  myProps.set("increaseFactor", incrFact_);
  myProps.set("decreaseFactor", decrFact_);
}

//-----------------------------------------------------------------------
//   advance
//-----------------------------------------------------------------------

void AdaptiveStepModule::advance(const Properties &globdat)
{
  oldLoadScale_ = loadScale_;
  loadScale_ += incr_;

  Properties vars = Globdat::getVariables(globdat);
  vars.set(jive::model::RunvarNames::LOAD_SCALE, loadScale_);

  solver_->advance(globdat);
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
    jem::System::info(myName_) << e.name() << " occured in the inner solver!\n"
                               << e.what() << "\n";
  }
}

//-----------------------------------------------------------------------
//   cancel
//-----------------------------------------------------------------------
void AdaptiveStepModule::cancel(const Properties &globdat)
{
  loadScale_ = oldLoadScale_;

  Properties vars = Globdat::getVariables(globdat);
  vars.set(jive::model::RunvarNames::LOAD_SCALE, loadScale_);

  solver_->cancel(globdat);
}

//-----------------------------------------------------------------------
//   commit
//-----------------------------------------------------------------------

bool AdaptiveStepModule::commit

    (const Properties &globdat)
{
  bool accept;
  idx_t currIter = 100.;

  Properties solverInfo = SolverInfo::get(globdat);
  accept = solver_->commit(globdat) && solverInfo.find(currIter, SolverInfo::ITER_COUNT);

  // adapt step size based on the iterations (compare Module of Frans)
  double optIncr = incr_ * pow(0.5, (currIter - optIter_) / 4.);

  if (accept && currIter < optIter_) // increase step size
    incr_ = jem::max(jem::min(optIncr, incrFact_ * incr_, maxIncr_), decrFact_ * incr_, minIncr_);
  if (!accept || currIter > optIter_) // decrease step size
    incr_ = jem::max(optIncr, decrFact_ * incr_, minIncr_);

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
