///////////////////////////////////
/////// ---  ///////
///////////////////////////////////

// LOGGING
log.pattern = "*"; //

// PROGRAM_CONTROL
control.runWhile = "i<=50";

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

params.force_model.dispIncr =  0.01;
params.force_model.nodeGroups = [ "fixed_right", "fixed_right", "fixed_right" ];
params.force_model.dofs = ["dx", "dy", "dz"];
params.force_model.factors = [ -1., 0., 0. ]; 
params.force_model.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
params.force_model.dofs += ["rx", "ry", "rz"];
params.force_model.factors += [ 0., 0., 0. ]; 

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.model.model.diriFixed.nodeGroups = [ "fixed_left", "fixed_left", "fixed_left" ];
model.model.model.diriFixed.dofs = model.model.model.lattice.child.dofNamesTrans;
model.model.model.diriFixed.factors = [ 0., 0., 0. ]; 
model.model.model.diriFixed.nodeGroups += [ "fixed_left", "fixed_left", "fixed_left" ];
model.model.model.diriFixed.dofs += model.model.model.lattice.child.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 

model.model.model.lattice.contact.penalty = 1e6;
model.model.model.lattice.contact.radius = 0.05;
model.model.model.lattice.contact.verbose = true;

Output.paraview.sampleWhen = true;
Output.paraview.beams.shape = "Line2";

log.file = "$(CASE_NAME)/run.log";
