// PROGRAM_CONTROL
control.runWhile = "t < 1e-2";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "Explicit";
Solver.integrator.deltaTime = 1e-7;
Solver.integrator.stepCount = 2;
Solver.integrator.reportEnergy = true;

// settings
params.X = "sin(PI/180) * sin(PI/4)";
params.Y = "sin(PI/180) * cos(PI/4)";
params.Z = "cos(PI/180)";

params.rod_details.cross_section = "circle";
params.rod_details.radius = 0.05;
params.rod_details.young = 205e9;
params.rod_details.poission_ratio = 0.33;
params.rod_details.density = 7850.;
params.rod_details.shape.numPoints = 2;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

// more settings
Input.input.onelab.X = params.X;
Input.input.onelab.Y = params.Y;
Input.input.onelab.Z = params.Z;

model.model.model.force.type = "LoadScale";
model.model.model.force.scaleFunc = "if (i-100<=0, 2e9, 0) - if (i-99<=0, 2e9, 0)";
model.model.model.force.model.type = "Neumann";
model.model.model.force.model.nodeGroups =  [ "free", "free", "free" ] ;
model.model.model.force.model.factors = [ "$(params.X)", "$(params.Y)", "$(params.Z)" ];
model.model.model.force.model.dofs = [ "dx", "dy", "dz" ];

model.model.model.disp.type = "None";

Output.disp.vectors += "state1 = velo"; 
Output.disp.vectors += "state2 = acce"; 

Output.modules += "enSample";
Output.enSample.type = "Sample";
Output.enSample.file = "$(CASE_NAME)/energy.csv";
Output.enSample.header = "time,E_kin,E_pot,E_tot";
Output.enSample.dataSets = ["t", "KineticEnergy", "PotentialEnergy", "TotalEnergy"];
Output.enSample.separator = ",";

Output.modules += "paraview";
Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%i";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.shape = "Line$(params.rod_details.shape.numPoints)";
Output.paraview.beams.disps = model.model.model.rodMesh.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.model.rodMesh.child.dofNamesRot;
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain"];
Output.paraview.sampleWhen = "t % 2.5e-5 < $(Solver.integrator.deltaTime)";