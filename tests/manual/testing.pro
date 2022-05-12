// LOGGING
log.pattern = "*";
log.file = "-$(CASE_NAME).log";

// PROGRAM CONTROL
control.runWhile = "i < 50";

// SETUP
params.dofNames = ["dx", "dy", "dz"];
params.rotNames = ["rx", "ry", "rz"];

// INPUT
Input.modules = [ "input", "nodeInput" ];

Input.input.type = "GMSHInput";
Input.input.file = "$(CASE_NAME).geo";
Input.input.verbose = true;
Input.input.store_tangents = true;
Input.input.onelab.RepX = 1.;
Input.input.onelab.RepY = 1.;
Input.input.onelab.TopBar = 1.;
Input.input.onelab.Angle = 120.;

Input.nodeInput.type = "PBCGroupInput";
Input.nodeInput.groupSettings.restrictToGroup = "points";
Input.nodeInput.corners = false;
// Input.nodeInput.nodeGroups = [ "impact" ];
// Input.nodeInput.impact.restrictToGroup = "beams";
// Input.nodeInput.impact.ytype = "max";
// Input.nodeInput.impact.xbounds = [ "ORIGIN.X + SIZE.X/3*1", "ORIGIN.X + SIZE.X/3*2" ];

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.deltaTime = 1e-4;
// Solver.solver.type = "Newmark";
// Solver.solver.solver.type = "Nonlin";
Solver.solver.type = "TimeStep";
Solver.solver.dtAssembly = 1;

// ACTUAL MODEL
model.type = "Matrix";
model.model.type = "Multi";
model.model.models = [ "lattice", "load" ];

model.model.lattice.type = "Lattice";
model.model.lattice.type = "Lattice";
model.model.lattice.prefix = "beam_";
model.model.lattice.child.type = "specialCosseratRod";
model.model.lattice.child.dofNamesTrans = ["dx", "dy", "dz"];
model.model.lattice.child.dofNamesRot = ["rx", "ry", "rz"];
model.model.lattice.child.shape.numPoints = 2;
model.model.lattice.child.young = 210e9;
model.model.lattice.child.poission_ratio = 0.3;
model.model.lattice.child.area = "0.05e-3 ^ 2";
model.model.lattice.child.area_moment = "0.05e-3 ^ 4 / 12";
model.model.lattice.child.density = 7850.;
model.model.lattice.child.dofNamesTrans = params.dofNames;
model.model.lattice.child.dofNamesRot = params.rotNames;

model.model.load.type = "Multi";
model.model.load.models = [ "fixed", "impact" ];
model.model.load.fixed.type = "Dirichlet";
model.model.load.fixed.dispIncr = 0.;
model.model.load.fixed.nodeGroups = [ "zmin", "zmin", "zmin" ];
model.model.load.fixed.dofs = [ "dx", "dy", "dz" ];
model.model.load.fixed.factors = [ 0., 0., 0. ]; 
model.model.load.impact.type = "Dirichlet";
model.model.load.impact.dispIncr = 0.005;
model.model.load.impact.nodeGroups = [ "zmax", "zmax" ];
model.model.load.impact.dofs = [ "dx", "dy" ];
model.model.load.impact.factors = [ 0., -1. ];

// OUTPUTS
Output.modules = [ "pbcOut", "paraview" ];

Output.pbcOut.type = "None";
// Output.pbcOut.type = "PBCGroupOutput";
// Output.pbcOut.dofs = params.pbcDofs;
// Output.pbcOut.sampling.file = "$(CASE_NAME)/PBCOut.csv";

Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/step%d_Explicit";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.disps = params.dofNames;
Output.paraview.beams.otherDofs = params.rotNames;
Output.paraview.beams.el_data = [ "strain", "stress", "mat_strain", "mat_stress" ];
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.shape = "Line2";
