///////////////////////////////////
/////// SIMO/VU-QUOC EX 7.3 ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug";
log.file = "$(CASE_NAME).log";

// PROGRAM_CONTROL
control.runWhile = "free.disp.dy > -0.5 && i < 5000";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Arclen";

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.young = 6.895e10;
params.rod_details.material.shear_modulus = 2.16e10;
params.rod_details.material.area = "1.27e-3";
params.rod_details.material.area_moment = "1.27e-3 ^ 3 / 12";
params.rod_details.material_ey = [0.,0.,-1.];

params.force_model.type = "StdArclen";
params.force_model.loadIncr = .1;
params.force_model.minIncr = .01;
params.force_model.maxIncr = 100.;
params.force_model.model.type = "Neumann" ;
params.force_model.model.initLoad = 1.;
params.force_model.model.loadIncr = 0.;
params.force_model.model.nodeGroups = "free" ;
params.force_model.model.dofs = "dy";
params.force_model.model.factors = -1.;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.lattice.child.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 

Output.paraview.sampleWhen = "i%100<1";
Output.paraview.beams.shape = "Line2";
