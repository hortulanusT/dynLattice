///////////////////////////////////
/////// ROLL-UP, CHANGING AXIS ////
///////////////////////////////////

// Cantilever rolled up to 4 pi by an end moment, with a dead transverse force that makes the
// rotation axis change along the path; a node passes |theta| = 2 pi

// PROGRAM_CONTROL
params.Steps = 80;
control.runWhile = "i<$(params.Steps)";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";
Solver.solver.precision = 1e-8;
Solver.solver.maxIter = 40;

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.C_11 = 2.;
params.rod_details.material.C_22 = 2.;
params.rod_details.material.C_33 = 2.;
params.rod_details.material.C_44 = 2.;
params.rod_details.material.C_55 = 2.;
params.rod_details.material.C_66 = 2.;

params.force_model.type = "Neumann";
params.force_model.loadIncr = "8*PI/$(params.Steps)";
params.force_model.nodeGroups = [ "free", "free" ];
params.force_model.dofs = [ "dz", "rz" ];
params.force_model.factors = [ 0.2, 1. ];

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Output.modules = [ "loadextent", "disp", "resp" ];
model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.lattice.child.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ];
