///////////////////////////////////
//////  Zavarise/Wriggers(2000) Example 2  ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //

// PROGRAM_CONTROL
control.runWhile = "i<=6";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";
Solver.solver.tiny = 1e-6;

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.young = 1e8;
params.rod_details.material.poisson_ratio = .0;
params.rod_details.material.area = 4e-2;
params.rod_details.material.area_moment = 2e-4;
params.rod_details.material.shear_correction = 1.;

params.force_model.type = "Dirichlet";

params.force_model.dispIncr = 0.3;
params.force_model.nodeGroups = [ "moving_right", "moving_right", "moving_right", "moving_right", "moving_right", "moving_right" ];
params.force_model.dofs = ["dx", "dy", "dz", "rx", "ry", "rz" ];
params.force_model.factors = [ 0.1, 0., 1., 0., 0., 0.]; 

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Input.groupInput.nodeGroups += [ "moving_left", "moving_right" ];
Input.groupInput.moving_left.xtype = "min";
Input.groupInput.moving_right.xtype = "max";

model.model.model.lattice.contact.penaltySTS = 1e4;
model.model.model.lattice.contact.penaltyNTS = 1e4;
model.model.model.lattice.contact.radius = 0.2;
model.model.model.lattice.contact.verbose = true;

Output.loadextent.nodeGroups = ["moving_left", "moving_right"];
Output.disp.dataSets = [ "moving_right.disp.dx", "moving_right.disp.dy", "moving_right.disp.dz", "moving_right.disp.rx", "moving_right.disp.ry", "moving_right.disp.rz" ];
Output.resp.dataSets = [ "moving_right.resp.dx", "moving_right.resp.dy", "moving_right.resp.dz", "moving_right.resp.rx", "moving_right.resp.ry", "moving_right.resp.rz" ];

Output.paraview.sampleWhen = true;
Output.paraview.beams.shape = "Line2";
