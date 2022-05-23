// LOGGING
log.pattern = "*.info | *.debug";
log.file = "-";

// PROGRAM_CONTROL
control.runWhile = "t < 30";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "EulerForward";
Solver.solver.deltaTime = 1e-4;

// settings
params.rod_details.cross_section = "square";
params.rod_details.side_length = 0.1;
params.rod_details.young = 2.8e9;
params.rod_details.shear_modulus = 1.2e9;
params.rod_details.density = 100.;
params.rod_details.shape.numPoints = 3;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

// more settings
Input.input.order = 2;

model.model.model.force.type = "None";

model.model.model.disp.type = "LoadScale";
model.model.model.disp.scaleFunc = "if (t<15, 6/15 * (1 - cos(2*PI/15 * t)), 0)";
model.model.model.disp.model.type = "Dirichlet";
model.model.model.disp.model.nodeGroups =  [ "fixed" ] ;
model.model.model.disp.model.factors = [ 1. ];
model.model.model.disp.model.dofs = [ "rz" ];

Output.modules = [ "loadextent", "paraview", "graph" ];
Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual%d";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.shape = "Line$(params.rod_details.shape.numPoints)";
Output.paraview.beams.disps = model.model.model.rodMesh.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.model.rodMesh.child.dofNamesRot;
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain"];
Output.paraview.reportIntervall = "0.1 / $(Solver.solver.deltaTime)";

Output.graph.type = "Graph";
Output.graph.dataSets = "rot";
Output.graph.rot.xData = "t";
Output.graph.rot.yData = "fixed.disp.rz";