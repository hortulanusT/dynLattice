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

  if (verbosity_ > 0)
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

  E_diss_.resize(ipCount, elemCount);
  E_diss_ = 0.;

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

void ElastoPlasticRodMaterial::getStress(const Vector &stress, const Vector &strain, const idx_t &ielem, const idx_t &ip, const bool inelastic)
{
  if (verbosity_ > 1)
    jem::System::debug(myName_) << "elastoplastic material behavior for element " << ielem << " and integration point " << ip << "\n";
  curr_Strains_(ALL, ip, ielem) = strain;
  // REPORT("Step 1")
  idx_t liter = 0;
  Vector plastStrain = old_plastStrains_(ALL, ip, ielem).clone();
  Vector hardParams = old_hardParams_(ALL, ip, ielem).clone();
  double deltaFlow = 0.;

  Vector hardStress(argCount_ - stress.size());
  Vector args(argCount_);
  double f = 0.;
  Vector f_deriv(argCount_);
  double deltaDeltaFlow = 0.;

  while (true)
  {
    // SUBHEADER2("Step 2", liter)
    Super::getStress(stress, Vector(strain - plastStrain), ielem, ip, false);
    getHardVals(hardStress, hardParams);

    if (!inelastic || ((edgeFact_ != 1.) && (ielem < edgeElems_ || ielem > nElem_ - edgeElems_ - 1)))
    {
      if (verbosity_ > 1)
        jem::System::debug(myName_) << "        elastic calculation\n";
      break;
    }

    args[stress_part_] = stress;
    args[hard_part_] = hardStress;

    f = yieldCond_->getValue(args.addr());

    if (verbosity_ > 2)
      jem::System::debug(myName_) << "        iter = " << liter << ", f = " << f << "\n";
    JEM_PRECHECK2(liter < maxIter_, "Too many iterations in plasticity loop");
    if (f < precision_)
    {
      if (verbosity_ > 1)
        jem::System::debug(myName_) << "        converged after " << liter << " iterations\n";
      break;
    }
    // SUBHEADER2("Step 3", liter)

    if (yieldDeriv_.size() > 0)
    {
      f_deriv = jive_helpers::evalFuncs(yieldDeriv_, args);
    }
    else
    {
      f_deriv = jive_helpers::funcGrad(yieldCond_, args);
      for (idx_t i = 0; i < strain.size(); i++)
        if (args[i] == 0.)
          f_deriv[i] = 0.;
    }

    deltaDeltaFlow = f / (dotProduct(f_deriv[stress_part_], matmul(materialK_, f_deriv[stress_part_])) + dotProduct(f_deriv[hard_part_], matmul(materialH_, f_deriv[hard_part_])));

    // SUBHEADER2("Step 4", liter)
    plastStrain += deltaDeltaFlow * f_deriv[stress_part_];
    hardParams += deltaDeltaFlow * f_deriv[hard_part_];
    deltaFlow += deltaDeltaFlow;

    liter++;
  }

  curr_plastStrains_(ALL, ip, ielem) = plastStrain;
  curr_hardParams_(ALL, ip, ielem) = hardParams;
  curr_deltaFlow_(ip, ielem) = deltaFlow;
}

void ElastoPlasticRodMaterial::apply_inelast_corr()
{
  for (idx_t ielem = 0; ielem < curr_Strains_.size(2); ielem++)
  {
    for (idx_t ip = 0; ip < curr_Strains_.size(1); ip++)
    {
      WARN_ASSERT2(curr_deltaFlow_(ip, ielem) >= 0., "Negative plastic multiplier");

      E_diss_(ip, ielem) += dotProduct(curr_plastStrains_(ALL, ip, ielem) - old_plastStrains_(ALL, ip, ielem),
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
  if (name == "plast_strain")
  {
    const idx_t elemCount = items.size();
    const idx_t ipCount = curr_plastStrains_.size(1);
    const IdxVector columns(strain_table.columnCount());
    columns = jem::iarray(strain_table.columnCount());

    for (idx_t ie = 0; ie < elemCount; ie++)
    {
      for (idx_t ip = 0; ip < ipCount; ip++)
      {
        strain_table.addRowValues(items[ie], columns, curr_plastStrains_(ALL, ip, ie));
        weights[items[ie]] += 1.;
      }
    }

    return;
  }

  if (name == "hard_params")
  {
    const idx_t elemCount = items.size();
    const idx_t ipCount = curr_hardParams_.size(1);
    const IdxVector columns(strain_table.columnCount());
    columns = jem::iarray(strain_table.columnCount());

    for (idx_t ie = 0; ie < elemCount; ie++)
    {
      for (idx_t ip = 0; ip < ipCount; ip++)
      {
        strain_table.addRowValues(items[ie], columns, curr_hardParams_(ALL, ip, ie));
        weights[items[ie]] += 1.;
      }
    }

    return;
  }

  WARN(name + " not supported by this material");
}

double ElastoPlasticRodMaterial::getDissipatedEnergy(const idx_t &ielem, const idx_t &ip) const
{
  return E_diss_(ip, ielem);
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
