///////////////////////////////////
/////// SIMO/VU-QUOC EX 7.3 ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug";
log.file = "-$(CASE_NAME).log";

// PROGRAM_CONTROL
control.runWhile = "i<41";

// SOLVER
Solver.modules = [ "solver" ];

// SETTINGS
params.rod_details.shape.numPoints = 2;
params.rod_details.young = 200e6;
params.rod_details.poission_ratio = .3;
params.rod_details.area = 1e-4;
params.rod_details.area_moment = 8.333333333333333333333333e-10;
params.rod_details.material_ey = [0.,0.,1.];

// Solver.solver.type = "Arclen";
// params.force_model.type = "StdArclen";
// params.force_model.loadIncr = 8.064;
// params.force_model.minIncr = .01;
// params.force_model.maxIncr = 10.;

Solver.solver.type = "Nonlin";
params.force_model.type = "Dirichlet";
params.force_model.dispIncr = -5e-3;
params.force_model.dofs = "dy";
params.force_model.nodeGroups = "free";
params.force_model.factors = 1.;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Input.groupInput.nodeGroups = [ "fixed_left", "fixed_right", "free" ];
Input.groupInput.fixed_left.ytype = "min";
Input.groupInput.fixed_left.xtype = "min";
Input.groupInput.fixed_right.ytype = "min";
Input.groupInput.fixed_right.xtype = "max";
Input.groupInput.free.ytype = "max";

model.noiseLevel = 0;

model.model.model.cosseratRod += params.rod_details;
model.model.model.force = params.force_model;

model.model.model.diriFixed.type = "Dirichlet";
model.model.model.diriFixed.maxDisp = 0.;
model.model.model.diriFixed.dispIncr =  0.;
model.model.model.diriFixed.nodeGroups = [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs = model.model.model.cosseratRod.dofNamesTrans;
model.model.model.diriFixed.factors = [ 0., 0., 0. ];  
model.model.model.diriFixed.nodeGroups += [ "fixed_left", "fixed_left", "fixed_left" ];
model.model.model.diriFixed.dofs += model.model.model.cosseratRod.dofNamesTrans;
model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 
// model.model.model.diriFixed.nodeGroups += [ "all", "all", "all" ];
// model.model.model.diriFixed.dofs += ["dz", "rx", "ry"];
// model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 

Output.disp.append = false;
Output.resp.append = false;
Output.load.append = false;

// Output.paraview.type = "None";