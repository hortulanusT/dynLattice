// LOGGING
log.pattern = "*.info | *.debug"; // 
log.file = "-$(CASE_NAME).log";

// PROGRAM CONTROL
control.runWhile = "(xmax.disp.dx - xmin.disp.dx) / all.extent.dx < 0.75";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Arclen";

// SETUP
params.dofNames = ["dx", "dy", "dz"];
params.rotNames = ["rx", "ry", "rz"];
// material settings
params.material.shape.numPoints = 2;
params.material.young = 210e9;
params.material.poission_ratio = 0.3;
params.material.area = ".05e-3 ^ 2";
params.material.area_moment = ".05e-3 ^ 4 / 12";
params.material.dofNamesTrans = params.dofNames;
params.material.dofNamesRot = params.rotNames;
// pbc settings
params.pbcDofs = [ "dx", "dy" ];
params.load.type = "StdArclen";
params.load.arcFunc = "UNP";
params.load.weightTable = "arclenWeights";
params.load.loadIncr = 1e-3;
params.load.model.type = "PeriodicBC";
params.load.model.mode = "arclen";
params.load.model.dofs = ["dx", "dy"];
params.load.model.rotDofs = [ "rz" ];
params.load.model.grad11 = -1.;
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
model.model.models = [ "cosseratRod", "load", "joint", "fixed2D" ];

model.model.cosseratRod.type = "specialCosseratRod";
model.model.cosseratRod.elements = "rods";
model.model.cosseratRod.material_ey = [ 0., 0., 1. ];
model.model.cosseratRod += params.material;

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

// Output.view.type = "FemView";

// Output.view.type = "ParaView";
// Output.view.output_format = "$(CASE_NAME)/custom-step%d";
// Output.view.groups = [ "rods" ];
// Output.view.rods.shape = "Line$(params.material.shape.numPoints)";
// Output.view.rods.disps = params.dofNames;
// Output.view.rods.otherDofs = params.rotNames;
// Output.view.rods.el_data = [ "strain", "stress", "mat_strain", "mat_stress" ];