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
  optIter_ = 4;
  incrFact_ = 1.5;
  decrFact_ = 0.5;
  minTried_ = maxTried_ = false;
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
    jem::System::info(myName_) << e.name() << " occured in the inner solver!\n\t"
                               << e.what() << "\n";
    info.set(SolverInfo::CONVERGED, false);
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
  bool accept = false;
  double optIncr = incr_ * decrFact_;
  idx_t currIter;

  Properties solverInfo = SolverInfo::get(globdat);
  solverInfo.find(accept, SolverInfo::CONVERGED);
  accept &= solver_->commit(globdat);

  // adapt step size based on the iterations (compare Module of Frans)
  if (accept && solverInfo.find(currIter, SolverInfo::ITER_COUNT))
    optIncr = incr_ * pow(0.5, (currIter - optIter_) / 4.);

  if (!accept && incr_ == minIncr_)
    minTried_ = true;
  if (!accept && incr_ == maxIncr_)
    maxTried_ = true;

  if (accept && currIter < optIter_) // increase step size
    incr_ = jem::max(jem::min(optIncr, incrFact_ * incr_, maxIncr_), decrFact_ * incr_, minIncr_);
  if (!accept || currIter > optIter_) // decrease step size
    incr_ = jem::max(optIncr, decrFact_ * incr_, minIncr_);

  if (accept)
    minTried_ = maxTried_ = false;
  else
  {
    if (minTried_ && maxTried_)
      throw jive::solver::SolverException("AdaptiveStepModule", "Solver is out of inspiration, sorry!");

    if (minTried_)
      incr_ = maxIncr_;
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
