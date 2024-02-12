#include "materials/ElastoPlasticRodMaterial.h"
#include "utils/testing.h"
#include <jem/base/ClassTemplate.h>

JEM_DEFINE_CLASS(ElastoPlasticRodMaterial);

const char *ElastoPlasticRodMaterial::TYPE_NAME = "ElastoPlasticRod";
const char *ElastoPlasticRodMaterial::YIELD_PROP = "yieldCond";
const char *ElastoPlasticRodMaterial::YIELD_DERIV_PROP = "yieldDeriv";
const char *ElastoPlasticRodMaterial::ISO_HARD_PROP = "isotropicCoefficient";
const char *ElastoPlasticRodMaterial::KIN_HARD_PROP = "kinematicTensor";

ElastoPlasticRodMaterial::ElastoPlasticRodMaterial(const String &name,
                                                   const Properties &conf,
                                                   const Properties &props,
                                                   const Properties &globdat) : Super(name, conf, props, globdat)
{
  maxIter_ = 20;
  precision_ = 1e-5;
  materialH_.resize(0);
  argCount_ = 0;
  E_diss_ = 0.;

  configure(props, globdat);
  getConfig(conf, globdat);
}

ElastoPlasticRodMaterial::~ElastoPlasticRodMaterial()
{
}

void ElastoPlasticRodMaterial::configure(const Properties &props, const Properties &globdat)
{
  using jem::util::StringUtils;

  Properties myProps = props.findProps(myName_);
  myProps.setConverter(newInstance<jive::util::ObjConverter>(globdat));

  Ref<DofSpace> dofs = DofSpace::get(globdat, getContext());

  idx_t ipCount;
  idx_t elemCount;
  idx_t dofCount;
  StringVector dofNames = dofs->getTypeNames();
  String args = StringUtils::join(dofNames, ", ");

  myProps.find(ipCount, "ipCount");
  myProps.find(elemCount, "elemCount");
  dofCount = dofs->typeCount();
  argCount_ = dofCount;

  double isoCoeff;
  Vector kinHard;
  Matrix kinFacts(dofCount, dofCount);
  if (myProps.find(isoCoeff, ISO_HARD_PROP))
  {
    args = args + ", h_0";
    argCount_ += 1;
  }
  if (myProps.find(kinHard, KIN_HARD_PROP))
  {
    jive_helpers::vec2mat(kinFacts.transpose(), kinHard);
    for (String dofName : dofNames)
    {
      args = args + ", h_" + dofName;
      argCount_ += 1;
    }
  }

  stress_part_ = jem::SliceTo(dofCount);
  hard_part_ = jem::SliceFromTo(dofCount, argCount_);

  materialH_.resize(argCount_ - dofCount, argCount_ - dofCount);
  if (argCount_ == 7)
  {
    materialH_ = isoCoeff;
  }
  if (argCount_ == 12)
  {
    materialH_ = kinFacts;
  }
  if (argCount_ == 13)
  {
    materialH_ = 0.;
    materialH_(0, 0) = isoCoeff;
    materialH_(jem::SliceFrom(1), jem::SliceFrom(1)) = kinFacts;
  }

  jem::System::debug(myName_)
      << " ...Hardening matrix of the material '" << myName_ << "':\n"
      << materialH_ << "\n";

  old_hardParams_.resize(argCount_ - dofCount, ipCount, elemCount);
  old_hardParams_ = 0.;
  curr_hardParams_.resize(argCount_ - dofCount, ipCount, elemCount);
  curr_hardParams_ = 0.;

  old_plastStrains_.resize(dofCount, ipCount, elemCount);
  old_plastStrains_ = 0.;
  curr_plastStrains_.resize(dofCount, ipCount, elemCount);
  curr_plastStrains_ = 0.;

  old_Strains_.resize(dofCount, ipCount, elemCount);
  old_Strains_ = 0.;
  curr_Strains_.resize(dofCount, ipCount, elemCount);
  curr_Strains_ = 0.;

  curr_deltaFlow_.resize(ipCount, elemCount);
  curr_deltaFlow_ = 0.;

  if (!myProps.contains(YIELD_PROP))
    throw jem::util::PropertyException("Expected a yield function for an elasto-plastic material!");
  FuncUtils::configFunc(yieldCond_, args, YIELD_PROP, myProps, globdat);
  if (myProps.contains(YIELD_DERIV_PROP))
  {
    FuncUtils::configFuncs(yieldDeriv_, args, YIELD_DERIV_PROP, myProps, globdat);
    JEM_PRECHECK(yieldDeriv_.size() == argCount_);
  }
  else
  {
    yieldDeriv_.resize(0);
  }

  myProps.find(maxIter_, jive::implict::PropNames::MAX_ITER);
  myProps.find(precision_, jive::implict::PropNames::PRECISION);
}

void ElastoPlasticRodMaterial::getConfig(const Properties &conf, const Properties &globdat) const
{
  Properties myConf = conf.makeProps(myName_);

  FuncUtils::getConfig(myConf, yieldCond_, YIELD_PROP);
  if (yieldDeriv_.size() > 0)
  {
    FuncUtils::getConfig(myConf, yieldDeriv_, YIELD_DERIV_PROP);
  }

  if (argCount_ == 7)
  {
    myConf.set(ISO_HARD_PROP, materialH_(0, 0));
  }
  if (argCount_ == 12)
  {
    Vector kinHard(materialH_.size(0) * materialH_.size(1));
    jive_helpers::mat2vec(kinHard, materialH_);
    myConf.set(KIN_HARD_PROP, kinHard);
  }
  if (argCount_ == 13)
  {
    myConf.set(ISO_HARD_PROP, materialH_(0, 0));

    Vector kinHard((materialH_.size(0) - 1) * (materialH_.size(1) - 1));
    Matrix kinFacts((materialH_.size(0) - 1), (materialH_.size(1) - 1));
    jive_helpers::vec2mat(kinFacts.transpose(), kinHard);
    myConf.set(KIN_HARD_PROP, kinHard);
  }

  myConf.set(jive::implict::PropNames::MAX_ITER, maxIter_);
  myConf.set(jive::implict::PropNames::PRECISION, precision_);
}

void ElastoPlasticRodMaterial::getHardVals(const Vector &hardVals, const Vector &hardParams) const
{
  hardVals = -1. * matmul(materialH_, hardParams);
}

// an Euler Forward explicit integration scheme (iterative until the yield function is satisfied)
// normaility is enforced at an lineraized ciritcal point
void ElastoPlasticRodMaterial::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip)
{
  jem::System::debug(myName_) << "elastoplastic material behavior for element " << ielem << " and integration point " << ip << "\n";
  curr_Strains_(ALL, ip, ielem) = strain;
  // REPORT("Step 1")
  idx_t liter = 0;
  Vector plastStrain_trial = curr_plastStrains_(ALL, ip, ielem).clone();
  Vector hardParams_trial = curr_hardParams_(ALL, ip, ielem).clone();
  double deltaFlow_trial = 0.;

  Vector hardStress_trial(argCount_ - stress.size());
  Vector args_trial(argCount_);
  double f_trial = 0.;
  Vector f_critDeriv(argCount_);
  double deltaDeltaFlow = 0.;

  Vector args_old(argCount_);
  Vector args_crit(argCount_);
  ElasticRodMaterial::getStress(args_old[stress_part_], Vector(old_Strains_(ALL, ip, ielem) - old_plastStrains_(ALL, ip, ielem)));
  getHardVals(args_old[hard_part_], old_hardParams_(ALL, ip, ielem));
  double f_old = yieldCond_->getValue(args_old.addr());

  while (true)
  {
    // SUBHEADER2("Step 2", liter)
    ElasticRodMaterial::getStress(stress, Vector(strain - plastStrain_trial));
    getHardVals(hardStress_trial, hardParams_trial);

    args_trial[stress_part_] = stress;
    args_trial[hard_part_] = hardStress_trial;

    f_trial = yieldCond_->getValue(args_trial.addr());

    jem::System::debug(myName_) << "        iter = " << liter << ", f = " << f_trial << "\n";
    JEM_PRECHECK2(liter < 20, "Too many iterations in plasticity loop"); // TODO make this a property
    if (f_trial < 1e-5)                                                  // TODO make those properties
    {
      break;
    }
    // SUBHEADER2("Step 3", liter)
    args_crit = args_old - (args_trial - args_old) * f_old / (f_trial - f_old);

    // TEST_CONTEXT(args_old)
    // TEST_CONTEXT(yieldCond_->getValue(args_old.addr()))
    // TEST_CONTEXT(args_trial)
    // TEST_CONTEXT(yieldCond_->getValue(args_trial.addr()))
    // TEST_CONTEXT(args_crit)
    // TEST_CONTEXT(yieldCond_->getValue(args_crit.addr()))

    if (yieldDeriv_.size() > 0)
    {
      f_critDeriv = jive_helpers::evalFuncs(yieldDeriv_, args_crit);
    }
    else
    {
      f_critDeriv = jive_helpers::funcGrad(yieldCond_, args_crit);
      for (idx_t i = 0; i < strain.size(); i++)
        if (args_crit[i] == 0.)
          f_critDeriv[i] = 0.;
    }

    // TEST_CONTEXT(f_critDeriv)

    deltaDeltaFlow = dotProduct(f_critDeriv[stress_part_], args_trial[stress_part_] - args_crit[stress_part_]) / (dotProduct(f_critDeriv[stress_part_], matmul(materialK_, f_critDeriv[stress_part_])) + dotProduct(f_critDeriv[hard_part_], matmul(materialH_, f_critDeriv[hard_part_])));

    // SUBHEADER2("Step 4", liter)
    plastStrain_trial += deltaDeltaFlow * f_critDeriv[stress_part_];
    hardParams_trial += deltaDeltaFlow * f_critDeriv[hard_part_];
    deltaFlow_trial += deltaDeltaFlow;

    // TEST_CONTEXT(plastStrain_trial)
    // TEST_CONTEXT(hardParams_trial)
    // TEST_CONTEXT(deltaFlow_trial)

    liter++;
  }

  jem::System::debug(myName_) << "        converged after " << liter << " iterations\n";

  curr_plastStrains_(ALL, ip, ielem) = plastStrain_trial;
  curr_hardParams_(ALL, ip, ielem) = hardParams_trial;
  curr_deltaFlow_(ip, ielem) = deltaFlow_trial;
}

void ElastoPlasticRodMaterial::apply_inelast_corr()
{
  for (idx_t ielem = 0; ielem < curr_Strains_.size(2); ielem++)
  {
    for (idx_t ip = 0; ip < curr_Strains_.size(1); ip++)
    {
      E_diss_ += dotProduct(curr_plastStrains_(ALL, ip, ielem) - old_plastStrains_(ALL, ip, ielem),
                            matmul(materialK_, Vector(curr_Strains_(ALL, ip, ielem) - curr_plastStrains_(ALL, ip, ielem))));
    }
  }

  old_hardParams_ = curr_hardParams_;
  old_plastStrains_ = curr_plastStrains_;
  old_Strains_ = curr_Strains_;

  curr_deltaFlow_ = 0.;
}

void ElastoPlasticRodMaterial::reject_inelast_corr()
{
  curr_hardParams_ = old_hardParams_;
  curr_plastStrains_ = old_plastStrains_;
  curr_Strains_ = old_Strains_;

  curr_deltaFlow_ = 0.;
}

void ElastoPlasticRodMaterial::getTable(const String &name, XTable &strain_table, const IdxVector &items, const Vector &weights) const
{
  if (name != "plast_strain")
  {
    WARN(name + " not supported by this material");
    return;
  }

  const idx_t elemCount = items.size();
  const idx_t ipCount = old_plastStrains_.size(1);
  const IdxVector columns(strain_table.columnCount());
  columns = jem::iarray(strain_table.columnCount());

  for (idx_t ie = 0; ie < elemCount; ie++)
  {
    for (idx_t ip = 0; ip < ipCount; ip++)
    {
      strain_table.addRowValues(items[ie], columns, old_plastStrains_(ALL, ip, ie));
      weights[items[ie]] += 1.;
    }
  }
}

double ElastoPlasticRodMaterial::getDisspiatedEnergy() const
{
  return E_diss_;
}

Ref<Material> ElastoPlasticRodMaterial::makeNew

    (const String &name, const Properties &conf, const Properties &props,
     const Properties &globdat)

{
  return newInstance<ElastoPlasticRodMaterial>(name, conf, props, globdat);
}

void ElastoPlasticRodMaterial::declare()
{
  MaterialFactory::declare(TYPE_NAME, &makeNew);
  MaterialFactory::declare(CLASS_NAME, &makeNew);
}
