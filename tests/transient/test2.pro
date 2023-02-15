// Lang et al 2011 "Multibody Dynamics Simulation of geometrically exact Cossrat rods Example 1

// PROGRAM_CONTROL
control.runWhile = "t <= 1";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "MilneDevice";
Solver.integrator.deltaTime = 1e-5;
Solver.integrator.precision = 1e-7;

// settings
params.rod_details.cross_section = "circle";
params.rod_details.radius = 5e-3;
params.rod_details.young = 5e6;
params.rod_details.poission_ratio = 0.5;
params.rod_details.shear_correction = 1.;
params.rod_details.density = 1.1e3;
params.rod_details.shape.numPoints = 2;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

// more settings
Input.input.onelab.size = 0.1;

model.model.force.type = "Neumann";
model.model.force.loadIncr = 0.;
model.model.force.initLoad = "PI * (5e-3)^2 * $(Input.input.onelab.size) * 1.1e3 * 9.81";
model.model.force.nodeGroups =  [ "all", "free", "fixed" ] ;
model.model.force.factors = [ -1., 0.5, 0.5 ];
model.model.force.dofs = [ "dz", "dz", "dz" ];

model.model.fixed.nodeGroups = [ "fixed", "fixed", "fixed" ];
model.model.fixed.dofs = model.model.rodMesh.child.dofNamesTrans;
model.model.fixed.factors = [ 0., 0., 0. ]; 

model.model.disp.type = "None";

Output.modules += "paraview";
Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%i";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.shape = "Line$(params.rod_details.shape.numPoints)";
Output.paraview.beams.disps = model.model.rodMesh.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.rodMesh.child.dofNamesRot;
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain"];
Output.paraview.sampleWhen = "t % 0.01 < deltaTime";

log.pattern = "*";
log.file = "-";