///////////////////////////////////
/////// SIMO/VU-QUOC EX 7.3 ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug";
log.file = "-$(CASE_NAME).log";

// PROGRAM_CONTROL
control.runWhile = "i<200";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.shape.numPoints = "2";
params.rod_details.young = 100.;
params.rod_details.poission_ratio = .3;
params.rod_details.area = 34.641;
params.rod_details.area_moment = 100.; // square with side length of 5.88566

// params.force_model.type = "StdArclen";
// params.force_model.loadIncr = 8.064;
// params.force_model.minIncr = .01;
// params.force_model.maxIncr = 10.;
// params.force_model.model.type = "Neumann" ;
// params.force_model.model.initLoad = 1.;
// params.force_model.model.loadIncr = 0.;
// params.force_model.model.nodeGroups = "free" ;
// params.force_model.model.dofs = "dy";
// params.force_model.model.factors = -1.;

params.force_model.type = "Neumann" ;
params.force_model.initLoad = 0.;
params.force_model.loadIncr = 100.;
params.force_model.nodeGroups = "free" ;
params.force_model.dofs = "dy";
params.force_model.factors = -1.;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.noiseLevel=0;

Input.groupInput.nodeGroups = [ "fixed_left", "fixed_right", "free" ];
Input.groupInput.fixed_left.ytype = "min";
Input.groupInput.fixed_left.xtype = "min";
Input.groupInput.fixed_right.ytype = "min";
Input.groupInput.fixed_right.xtype = "max";
Input.groupInput.free.ytype = "max";

model.model.model.cosseratRod += params.rod_details;
model.model.model.force = params.force_model;

Output.disp.append = false;
Output.resp.append = false;
Output.load.append = false;

model.model.model.diriFixed.type = "Dirichlet";
model.model.model.diriFixed.maxDisp = 0.;
model.model.model.diriFixed.dispIncr =  0.;
model.model.model.diriFixed.nodeGroups = [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs = model.model.model.cosseratRod.dofNamesTrans;
model.model.model.diriFixed.factors = [ 0., 0., 0. ]; 
model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.cosseratRod.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 
model.model.model.diriFixed.nodeGroups += [ "fixed_left", "fixed_left", "fixed_left" ];
model.model.model.diriFixed.dofs += model.model.model.cosseratRod.dofNamesTrans;
model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 
model.model.model.diriFixed.nodeGroups += [ "all", "all", "all" ];
model.model.model.diriFixed.dofs += ["dz", "rx", "ry"];
model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 