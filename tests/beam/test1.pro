///////////////////////////////////
/////// SIMO/VU-QUOC EX 7.1 ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //
log.file = "$(CASE_NAME).log";

// PROGRAM_CONTROL
control.runWhile = false;

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";
Solver.solver.maxIter = 1;

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.C_11 = 2.;
params.rod_details.material.C_22 = 2.;
params.rod_details.material.C_33 = 2.;
params.rod_details.material.C_44 = 2.;
params.rod_details.material.C_55 = 2.;
params.rod_details.material.C_66 = 2.;

params.force_model.type = "Neumann";
params.force_model.loadIncr = "8*PI";
params.force_model.nodeGroups =  [ "free", "free" ] ;
params.force_model.dofs = [ "rx", "rz" ];
params.force_model.factors = [ 0., 1. ];

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Output.paraview.beams.shape = "Line2";
model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.lattice.child.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ];
