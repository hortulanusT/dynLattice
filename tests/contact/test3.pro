///////////////////////////////////
/////// ---  ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //

// PROGRAM_CONTROL
control.runWhile = "i<=10";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";
Solver.solver.tiny = 1e-6;

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.young = 210e9;
params.rod_details.material.poisson_ratio = .3;
params.rod_details.material.cross_section = "square";
params.rod_details.material.side_length = 0.05;

params.force_model.type = "Dirichlet";

params.force_model.dispIncr =  0.02;
params.force_model.nodeGroups = [ "moving_left", "moving_right" ];
params.force_model.dofs = [ "dy", "dy" ];
params.force_model.factors = [ -1., -1. ]; 

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Input.groupInput.fixed_left = 
{
  xbounds=[-1.,0.];
  ybounds=[-1.,0.];
  restrictToGroup = "points";
};
Input.groupInput.fixed_right = 
{
  xbounds=[0.,1.];
  ybounds=[-1.,0.];
  restrictToGroup = "points";
};
Input.groupInput.nodeGroups += [ "moving_left", "moving_right" ];
Input.groupInput.moving_left.xbounds=[-1.,0.];
Input.groupInput.moving_left.ybounds=[0.,1.];
Input.groupInput.moving_left.restrictToGroup = "points";
Input.groupInput.moving_right.xbounds=[0.,1.];
Input.groupInput.moving_right.ybounds=[0.,1.];
Input.groupInput.moving_right.restrictToGroup = "points";

model.model.model.lattice.contact.penalty = 1e9;
model.model.model.lattice.contact.radius = 0.05;
model.model.model.lattice.contact.verbose = true;

Output.paraview.sampleWhen = true;
Output.paraview.beams.shape = "Line2";
