///////////////////////////////////
/////// SIMO/VU-QUOC EX 7.4 ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //
log.file = "$(CASE_NAME).log";

// PROGRAM_CONTROL
control.runWhile = "free.disp.dy > -95 && i < 2500";

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
params.force_model.loadIncr = 10.;
params.force_model.minIncr = .5;
params.force_model.maxIncr = 50.;
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

Input.input.order = 2;

Input.groupInput.fixed_left.ytype = "min";
Input.groupInput.fixed_left.xtype = "min";
Input.groupInput.fixed_right.ytype = "max";
Input.groupInput.fixed_right.xtype = "max";
Input.groupInput.free = 
{
  xval = 24.;
  yval = 120.;
};

model.model.model.lattice.child += params.rod_details;
model.model.model.force = params.force_model;

Output.paraview.reportIntervall = 10;