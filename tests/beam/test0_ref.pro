///////////////////////////////////
//////// SHALLOW  TRIANGLE ////////
///////////////////////////////////

// PROGRAM_CONTROL
control.runWhile = "free.disp.dy >= -.4";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.shape.numPoints = 2;
params.rod_details.young = 200e6;
params.rod_details.poission_ratio = .3;
params.rod_details.area = 1e-4;
params.rod_details.area_moment = 8.333333333333333333333333e-10;
params.rod_details.material_ey = [0.,0.,1.];

params.force_model.type = "StdArclen";
params.force_model.loadIncr = 1.;
params.force_model.minIncr = .1;
params.force_model.maxIncr = 5.;
params.force_model.model.type = "Neumann";
params.force_model.model.initLoad = 1.;
params.force_model.model.loadIncr = 0.;
params.force_model.model.dofs = "dy";
params.force_model.model.nodeGroups = "free";
params.force_model.model.factors = -1.;

params.force_model_disp.type = "Dirichlet";
params.force_model_disp.dispIncr = 0.005;
params.force_model_disp.dofs = "dy";
params.force_model_disp.nodeGroups = "free";
params.force_model_disp.factors = -1.;


// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Input.groupInput.free.restrictToGroup = "points";
Input.groupInput.free.restrictPartial = 0;
model.model.model.force=params.force_model_disp;

Output.modules = [  "loadextent", "disp", "resp" ]; //HACK enable real paraview again