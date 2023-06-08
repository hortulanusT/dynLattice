

// PROGRAM_CONTROL
control.runWhile = "i<=10";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.young = 2.;
params.rod_details.material.shear_modulus = 2.;
params.rod_details.material.shear_correction = 1.;
params.rod_details.material.area = 1.;
params.rod_details.material.area_moment = 1.;
params.rod_details.material.polar_moment = 1.;
params.rod_details.hinges.type = "rigidHinge";
params.rod_details.hinges.limitLoads = [1., 1., 1., 10., 10., 10.];

params.force_model.type = "LoadScale";
params.force_model.scaleFunc = "if ((i-1)<=5, (i-1)*4/5*PI, (11-i)*4/5*PI)";
params.force_model.model.type = "Neumann";
params.force_model.model.nodeGroups =  [ "free", "free" ] ;
params.force_model.model.dofs = [ "rx", "rz" ];
params.force_model.model.factors = [ 0., 1. ];

// include model and i/o files
include "../beam/input.pro";
include "../beam/model.pro";
include "../beam/output.pro";

Output.paraview.beams.shape = "Line2";
model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.lattice.child.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ];

// LOGGING
log.pattern = "*"; //
log.file = "$(CASE_NAME).log";