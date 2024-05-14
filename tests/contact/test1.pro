///////////////////////////////////
//////  Zavarise/Wriggers(2000) Example1  ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //

// PROGRAM_CONTROL
control.runWhile = "i<9";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.young = 210e9;
params.rod_details.material.poisson_ratio = .3;
params.rod_details.material.cross_section = "square";
params.rod_details.material.side_length = 0.01;

params.force_model.type = "Dirichlet";

params.force_model.dispIncr =  0.02;
params.force_model.nodeGroups = [ "moving_left", "moving_left", "moving_left", "moving_left", "moving_left", "moving_left"];
params.force_model.dofs = ["dx", "dy", "dz", "rx", "ry", "rz"];
params.force_model.factors = [ 0., 0., -1., 0., 0., 0.]; 
params.force_model.nodeGroups += [ "moving_right", "moving_right", "moving_right", "moving_right", "moving_right", "moving_right"];
params.force_model.dofs += ["dx", "dy", "dz", "rx", "ry", "rz"];
params.force_model.factors += [ 0., 0., -1., 0., 0., 0.]; 

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Input.groupInput.nodeGroups += [ "moving_left", "moving_right" ];
Input.groupInput.moving_left.xtype = "min";
Input.groupInput.moving_right.xtype = "max";

model.model.model.lattice.contact.penalty = 1e9;
model.model.model.lattice.contact.radius = 0.1;

Output.paraview.sampleWhen = true;
Output.paraview.beams.shape = "Line2";
