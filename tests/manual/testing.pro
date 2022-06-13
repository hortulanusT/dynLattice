// PROGRAM_CONTROL
control.runWhile = "t <= 30";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "EulerForward";
Solver.integrator.deltaTime = 5e-5;
Solver.integrator.dofs_SO3 = [ "rx", "ry", "rz" ];
Solver.integrator.updateWhen = true;

// settings
params.rod_details.cross_section = "square";
params.rod_details.side_length = "sqrt(12/2e3)";
params.rod_details.young = "5.6e10/12";
params.rod_details.shear_modulus = 2e9;
params.rod_details.density = 200.;
params.rod_details.shape.numPoints = 2;

// include model and i/o files
include "../transient/input.pro";
include "../transient/model.pro";
include "../transient/output.pro";

// more settings
Input.input.order = 1;

model.model.matrix2.type = "FEM";

model.model.model.force.type = "None";

model.model.model.disp.type = "LoadScale";
model.model.model.disp.scaleFunc = "if (t<15, 6/15 * (1 - cos(2*PI/15 * t)), 0)";
model.model.model.disp.model.type = "Dirichlet";
model.model.model.disp.model.nodeGroups =  [ "fixed", "fixed" ] ;
model.model.model.disp.model.factors = [ "1", "0 * 1/sqrt(2)" ];
model.model.model.disp.model.dofs = [ "ry", "rz" ];

Output.disp.type = "Sample";
Output.disp.file = "$(CASE_NAME)/disp.csv";
Output.disp.dataSets = [ "free.disp.dx", "free.disp.dy", "free.disp.dz", "free.disp.rx", "free.disp.ry", "free.disp.rz" ];
Output.disp.dataSets += "sqrt(fixed.disp.ry^2 + fixed.disp.rz^2)";
Output.disp.dataSets += "t";
Output.disp.separator	= ",";

Output.modules += "paraview";
Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%d";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.shape = "Line$(params.rod_details.shape.numPoints)";
Output.paraview.beams.disps = model.model.model.rodMesh.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.model.rodMesh.child.dofNamesRot;
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain"];
Output.paraview.sampleWhen = "t % 0.1 < $(Solver.integrator.deltaTime)";

log.pattern = "*";
log.file = "$(CASE_NAME).log";