///////////////////////////////////
/////// X-Test  ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //

// PROGRAM_CONTROL
control.runWhile = "i<=20";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.young = 210e9;
params.rod_details.material.poisson_ratio = .3;
params.rod_details.material.cross_section = "circle";
params.rod_details.material.radius = 0.025;

params.force_model.type = "Dirichlet";

params.force_model.maxDisp = 0.2;
params.force_model.dispIncr =  0.01;
params.force_model.nodeGroups = [ "moving_left", "moving_left", "moving_left", "moving_left", "moving_left", "moving_left"];
params.force_model.dofs = ["dx", "dy", "dz", "rx", "ry", "rz"];
params.force_model.factors = [ 1e-6, 1e-6, -1., 0., 0., 0.]; 
params.force_model.nodeGroups += [ "moving_right", "moving_right", "moving_right", "moving_right", "moving_right", "moving_right"];
params.force_model.dofs += ["dx", "dy", "dz", "rx", "ry", "rz"];
params.force_model.factors += [ -1e-6, -1e-6, -1., 0., 0., 0.]; 

// NONCONVERGENCE IS FIXED BY ADDING MINIMAL STRETCH TO BEAM!

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Input.groupInput.nodeGroups += [ "moving_left", "moving_right" ];
Input.groupInput.moving_left.ytype = "min";
Input.groupInput.moving_left.xtype = "max";
Input.groupInput.moving_right.ytype = "max";
Input.groupInput.moving_right.xtype = "min";

Output.paraview.sampleWhen = true;
Output.paraview.beams.shape = "Line2";
