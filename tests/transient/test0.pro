// PROGRAM_CONTROL
control.runWhile = "t < 1e-2";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "Explicit";
Solver.integrator.deltaTime = 1e-6;

// settings
params.rod_details.cross_section = "circle";
params.rod_details.radius = 0.05;
params.rod_details.young = 205e9;
params.rod_details.poission_ratio = 0.33;
params.rod_details.density = 7850.;
params.rod_details.shape.numPoints = 2;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

// more settings
Output.disp.file = "$(CASE_NAME)/disp.csv";
Output.disp.dofs = "dz";
Output.disp.writeState1 = true;

model.model.model.force.type = "LoadScale";
model.model.model.force.scaleFunc = "if (i-100<=0, 1e10, 0) - if (i-99<=0, 1e10, 0)";
model.model.model.force.model.type = "Neumann";
model.model.model.force.model.nodeGroups =  [ "free" ] ;
model.model.model.force.model.factors = [ 1. ];
model.model.model.force.model.dofs = [ "dz" ];

model.model.model.disp.type = "None";