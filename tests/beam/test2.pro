///////////////////////////////////
/////// SIMO/VU-QUOC EX 7.2 ///////
///////////////////////////////////

params.Steps = 1.;

// LOGGING
log.pattern = "*.info | *.debug";
log.file = "$(CASE_NAME).log";

// PROGRAM_CONTROL
control.runWhile = "i<150000/$(params.Steps)";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";
// Solver.solver.precision = 1e-9;

// SETTINGS
params.rod_details.shape.numPoints = "3";
params.rod_details.young = 2.1e7;
params.rod_details.poission_ratio = .3;
params.rod_details.area = 20.;
params.rod_details.area_moment = 1.666666666666667;

params.force_model.type = "LoadScale";
params.force_model.scaleFunc = "(i-1)*$(params.Steps)";
params.force_model.model.type = "FollowerLoad" ;
params.force_model.model.nodeGroup = "free" ;
params.force_model.model.dofs = ["dx", "dy", "dz"];
params.force_model.model.rot_dofs = ["rx", "ry", "rz"];
params.force_model.model.start_dir = [0., 0., 1.];

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.cosseratRod.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ];

Output.resp.dataSets += "sqrt(free.resp.dx^2 + free.resp.dy^2 + free.resp.dz^2)";

Output.paraview.reportIntervall = "1000/$(params.Steps)";