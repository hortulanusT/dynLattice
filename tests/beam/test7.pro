///////////////////////////////////
/////// ROLL-UP, PRESCRIBED ///////
///////////////////////////////////

// Cantilever rolled up by a prescribed total end rotation about z (to almost 4 pi), with a small
// dead transverse force; the prescribed value stays the total rotation past |theta| = 2 pi

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

params.force_model.type = "Multi";
params.force_model.models = [ "load", "twist" ];
params.force_model.load.type = "Neumann";
params.force_model.load.loadIncr = "0.2/$(params.Steps)";
params.force_model.load.nodeGroups = [ "free" ];
params.force_model.load.dofs = [ "dz" ];
params.force_model.load.factors = [ 1. ];
params.force_model.twist.type = "Dirichlet";
params.force_model.twist.dispIncr = "8*PI/$(params.Steps)";
params.force_model.twist.maxDisp = "8*PI";
params.force_model.twist.nodeGroups = "free";
params.force_model.twist.dofs = "rz";
params.force_model.twist.factors = 1.;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Output.modules = [ "loadextent", "disp", "resp" ];
model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.lattice.child.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ];
