// SIMO et al Example 5.2

// PROGRAM_CONTROL
control.runWhile = "t <= 30";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "MilneDevice";
Solver.integrator.deltaTime = 1e-5;
Solver.integrator.dofs_SO3 = [ "rx", "ry", "rz" ];
Solver.integrator.updateWhen = "i%20 < 1";
Solver.integrator.reportEnergy = true;
Solver.integrator.precision = 1e-4;

// settings
params.rod_details.cross_section = "square";
params.rod_details.side_length = "sqrt(12e-3)";
params.rod_details.young = "1e9/12";
params.rod_details.shear_modulus = "5e8/12";
params.rod_details.shear_correction	= 2.;
params.rod_details.density = "1e3/12";
params.rod_details.inertia_correct = 1e4;
params.rod_details.shape.numPoints = 2;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

// more settings
// Input.input.order = 2;

Input.groupInput.fixed.ytype = "max";
Input.groupInput.free.ytype = "min";
Input.groupInput.nodeGroups += "elbow";
Input.groupInput.elbow.xtype = "max";
Input.groupInput.elbow.ytype = "max";

model.model.force.type = "LoadScale";
model.model.force.scaleFunc = "if (t<=2, if (t<=1, t*50, (2-t)*50), 0)";
model.model.force.model.type = "Neumann";
model.model.force.model.nodeGroups =  [ "elbow" ] ;
model.model.force.model.factors = [ 1. ];
model.model.force.model.dofs = [ "dz" ];

model.model.disp.type = "None";

Output.disp.saveWhen = "t % 0.001 < deltaTime";

Output.modules += "enSample";
Output.enSample.type = "Sample";
Output.enSample.file = "$(CASE_NAME)/energy.csv";
Output.enSample.header = "time,time_step,run_time,load,E_kin,E_pot,E_tot";
Output.enSample.dataSets = ["t", "deltaTime", "runtime", "$(model.model.force.scaleFunc)", "KineticEnergy", "PotentialEnergy", "TotalEnergy"];
Output.enSample.separator = ",";
Output.enSample.sampleWhen = Output.disp.saveWhen;

Output.modules += "paraview";
Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%i";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.shape = "Line$(params.rod_details.shape.numPoints)";
Output.paraview.beams.disps = model.model.rodMesh.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.rodMesh.child.dofNamesRot;
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain"];
Output.paraview.sampleWhen = "t % 0.05 < deltaTime";

log.pattern = "*";
log.file = "-";
