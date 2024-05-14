///////////////////////////////////
/////// Litewka 2002 Example 3  ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //

// PROGRAM_CONTROL
control.runWhile = "i<=200";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.young = 210e9;
params.rod_details.material.poisson_ratio = .3;
params.rod_details.material.cross_section = "square";
params.rod_details.material.side_length = 0.05;

params.force_model.type = "Dirichlet";

params.force_model.dispIncr =  0.005;
params.force_model.nodeGroups = [ "top", "bottom", "end_beam2", "end_beam3" ];
params.force_model.dofs = ["dx", "dx", "dz", "dz"];
params.force_model.factors = [ 1., 1., -0.1, 0.1 ]; 

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

Input.groupInput.fixed_left = 
{
 xval = 1.;
 zval = .525;
};
Input.groupInput.fixed_right = 
{ 
  xval = 1.;
  zval = .475;
};

Input.groupInput.nodeGroups += [ "top", "bottom", "end_beam2", "end_beam3" ];
Input.groupInput.top.ztype = "max";
Input.groupInput.bottom.ztype = "min";
Input.groupInput.end_beam2.xval = 0.;
Input.groupInput.end_beam2.zval = .525;
Input.groupInput.end_beam3.xval = 0.;
Input.groupInput.end_beam3.zval = .475;

model.model.model.lattice.contact.penalty = 1e9;
model.model.model.lattice.contact.radius = 0.05;
model.model.model.lattice.contact.verbose = true;

Output.paraview.sampleWhen = true;
Output.paraview.beams.shape = "Line2";
