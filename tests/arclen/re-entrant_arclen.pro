// LOGGING
log.pattern = "*.info | *.debug"; // 
log.file = "-$(CASE_NAME).log";

// PROGRAM CONTROL
control.runWhile = "(xmax.disp.dx - xmin.disp.dx) / all.extent.dx > -0.7";

// SOLVER
Solver.modules = [ "solver" ];

// SETUP
params.dofNames = ["dx", "dy", "dz"];
params.rotNames = ["rx", "ry", "rz"];
// material settings
params.material_rod.shape.numPoints = 2;
params.material_rod.young = 210e9;
params.material_rod.poission_ratio = 0.3;
params.material_rod.area = ".05e-3 ^ 2";
params.material_rod.area_moment = ".05e-3 ^ 4 / 12";
params.material_rod.dofNamesTrans = params.dofNames;
params.material_rod.dofNamesRot = params.rotNames;

params.material_joints.shape.numPoints = 2;
params.material_joints.young = 210e9;
params.material_joints.poission_ratio = 0.3;
params.material_joints.area = ".1e-3 ^ 2";
params.material_joints.area_moment = ".1e-3 ^ 4 / 12";
params.material_joints.dofNamesTrans = params.dofNames;
params.material_joints.dofNamesRot = params.rotNames;
// pbc settings
params.pbcDofs = [ "dx", "dy" ];

// load settings

Solver.solver.type = "Arclen";
params.load.type = "StdArclen";
params.load.weightTable = "arclenWeights";
params.load.loadIncr = .05;
params.load.minIncr = .01;
params.load.maxIncr = 100.;
params.load.model.type = "PeriodicBC";
params.load.model.mode = "load";
params.load.model.dofs = ["dx", "dy"];
params.load.model.rotDofs = [ "rz" ];
params.load.model.P11 = -1.;

// Solver.solver.type = "Nonlin";
// params.load.type = "LoadScale";
// params.load.scaleFunc = "i*1e-3";
// params.load.model.type = "PeriodicBC";
// params.load.model.mode = "disp";
// params.load.model.dofs = ["dx", "dy"];
// params.load.model.rotDofs = [ "rz" ];
// params.load.model.H11 = -1.;

// 2D settings
params.dofs2D = ["dz", "rx", "ry"];
params.groups2D = [ "all", "all", "all" ];

// INPUT
Input.modules = [ "input", "nodeInput", "tables" ];
Input.input.type = "FEInput";
Input.input.file = "$(_DIR_)/re-entrant.dat";
Input.nodeInput.type = "PBCGroupInput";
Input.nodeInput.groupSettings.restrictToGroup = "joints";
Input.nodeInput.groupSettings.restrictPartial = 0;
Input.nodeInput.corners = false;
Input.tables.type = "FEInput";
Input.tables.file = "$(_DIR_)/groups.dat";

// ACTUAL_MODEL
model.type = "Matrix";
model.model.type = "Multi";
model.model.models = [ "cosseratRod", "cosseratJoints", "load", "joint", "fixed2D" ];

model.model.cosseratRod.type = "specialCosseratRod";
model.model.cosseratRod.elements = "rods_middle";
model.model.cosseratRod.material_ey = [ 0., 0., 1. ];
model.model.cosseratRod += params.material_rod;

model.model.cosseratJoints.type = "specialCosseratRod";
model.model.cosseratJoints.elements = "rods_joint";
model.model.cosseratJoints.material_ey = [ 0., 0., 1. ];
model.model.cosseratJoints += params.material_joints;

model.model.load = params.load;

model.model.joint.type = "rodJoint";
model.model.joint.elements = "joints";
model.model.joint.lockDofs = [];
model.model.joint.lockDofs += model.model.cosseratRod.dofNamesTrans;
model.model.joint.lockDofs += model.model.cosseratRod.dofNamesRot;

model.model.fixed2D.type = "Dirichlet";
model.model.fixed2D.dispIncr = 0.;
model.model.fixed2D.nodeGroups = params.groups2D;
model.model.fixed2D.dofs = params.dofs2D;

// OUTPUTS
Output.modules = [ "pbcOut", "view" ];

Output.pbcOut.type = "PBCGroupOutput";
Output.pbcOut.dofs = params.pbcDofs;
Output.pbcOut.sampling.file = "$(CASE_NAME).csv";

// Output.view.type = "Graph";
// Output.view.dataSets = [ "plot" ];
// Output.view.plot.xData = "(xmax.disp.dx - xmin.disp.dx) / all.extent.dx";
// Output.view.plot.yData = "xmax.resp.dx / all.extent.dy";
// Output.view.plot.key = "P11 vs H11";
// Output.view.snapshot = "$(_DIR_)/re-entrant_arclen.png";

Output.view.type = "ParaView";
Output.view.output_format = "$(CASE_NAME)/custom-step%d";
Output.view.groups = [ "rods" ];
Output.view.rods.shape = "Line2";
Output.view.rods.disps = params.dofNames;
Output.view.rods.otherDofs = params.rotNames;
Output.view.rods.el_data = [ "strain", "stress", "mat_strain", "mat_stress" ];