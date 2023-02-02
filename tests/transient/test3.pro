// SIMO et al Example 5.2

// PROGRAM_CONTROL
control.runWhile = "t <= 30";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "MilneDevice";
Solver.integrator.deltaTime = 1e-7;
Solver.integrator.precision = 1e-15;
Solver.integrator.dofs_SO3 = [ "rx", "ry", "rz" ];
Solver.integrator.updateWhen = true;
Solver.integrator.reportEnergy = true;

// settings
params.rod_details.cross_section = "square";
params.rod_details.side_length = "sqrt(12/1e3)";
params.rod_details.young = "1e9/12";
params.rod_details.shear_modulus = "1e9/24";
params.rod_details.shear_correction	= 2.;
params.rod_details.density = "1e3/12";
params.rod_details.shape.numPoints = 3;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

// more settings
Input.input.order = 2;

Input.groupInput.fixed.ytype = "max";
Input.groupInput.free.ytype = "min";
Input.groupInput.nodeGroups += "elbow";
Input.groupInput.elbow.xtype = "max";
Input.groupInput.elbow.ytype = "max";

model.model.model.force.type = "LoadScale";
model.model.model.force.scaleFunc = "t*50 - if (t<=1, 0, (t-1)*100) + if (t<=2, 0, (t-2)*50)";
model.model.model.force.model.type = "Neumann";
model.model.model.force.model.nodeGroups =  [ "elbow" ] ;
model.model.model.force.model.factors = [ 1. ];
model.model.model.force.model.dofs = [ "dz" ];

model.model.model.disp.type = "None";

Output.disp.sampleWhen = "t % 0.01 < $(Solver.integrator.deltaTime)";

Output.modules += "enSample";
Output.enSample.type = "Sample";
Output.enSample.file = "$(CASE_NAME)/energy.csv";
Output.enSample.header = "time,time_step,load,E_kin,E_pot,E_tot";
Output.enSample.dataSets = ["t", "deltaTime", "$(model.model.model.force.scaleFunc)", "KineticEnergy", "PotentialEnergy", "TotalEnergy"];
Output.enSample.separator = ",";

Output.modules += "paraview";
Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%d";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.shape = "Line$(params.rod_details.shape.numPoints)";
Output.paraview.beams.disps = model.model.model.rodMesh.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.model.rodMesh.child.dofNamesRot;
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain"];
Output.paraview.sampleWhen = Output.disp.sampleWhen;

log.pattern = "*";
log.file = "-";