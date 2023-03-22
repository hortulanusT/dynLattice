///////////////////////////////////
/////// SIMO/VU-QUOC EX 7.5 ///////
///////////////////////////////////

params.Steps = 30.;

// LOGGING
log.pattern = "*.info | *.debug"; // 
log.file = "$(CASE_NAME).log";

// PROGRAM_CONTROL
control.runWhile = "i<3000/$(params.Steps)";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.shape.numPoints = "2";
params.rod_details.young = 1e7;
params.rod_details.shear_modulus = .5e7;
params.rod_details.area = 1.;
params.rod_details.area_moment = "1/12";
params.rod_details.material_ey = [0., 0., 1. ];

params.force_model.type = "Neumann";
params.force_model.loadIncr = params.Steps;
params.force_model.nodeGroups = "free";
params.force_model.dofs = "dz";
params.force_model.factors = 1.;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.lattice.child.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ];
