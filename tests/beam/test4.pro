///////////////////////////////////
/////// SIMO/VU-QUOC EX 7.4 ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info "; //| *.debug
log.file = "$(CASE_NAME).log";

// PROGRAM_CONTROL
control.runWhile = "free.disp.dy > -95 && i < 25000";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Arclen";

// SETTINGS
params.rod_details.shape.numPoints = "3";
params.rod_details.young = 7.2e6;
params.rod_details.poission_ratio = .3;
params.rod_details.area = 2.;
params.rod_details.area_moment = 2.;

params.force_model.type = "StdArclen";
params.force_model.loadIncr = 150.;
params.force_model.minIncr = .01;
params.force_model.maxIncr = 5.;
// params.force_model.type = "LoadScale";
// params.force_model.scaleFunc = "(i-1)*.05";
params.force_model.model.type = "PointLoad" ;
params.force_model.model.loadTable = "force";

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Input.groupInput.nodeGroups = [ "fixed_left", "fixed_right", "free" ];
Input.groupInput.fixed_left.ytype = "min";
Input.groupInput.fixed_left.xtype = "min";
Input.groupInput.fixed_right.ytype = "max";
Input.groupInput.fixed_right.xtype = "max";
Input.groupInput.free = 
{
  xval = 24.;
  yval = 120.;
};

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
model.model.model.diriFixed.nodeGroups += [ "all", "all", "all" ];
model.model.model.diriFixed.dofs += ["dz", "rx", "ry"];
model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 

Output.disp.append = false;
Output.resp.append = false;
Output.load.append = false;

Output.paraview.reportIntervall = 100;

// Output.paraview.type = "None";