// LOGGING
log.pattern = "*";
log.file = "$(CASE_NAME).log";

// PROGRAM CONTROL
control.runWhile = "t < 0.1";
control.fgMode = true;

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
Solver.solver.deltaTime = 5e-6;
// Solver.solver.type = "Newmark";
// Solver.solver.solver.type = "Nonlin";
Solver.solver.type = "EulerForward";

// ACTUAL MODEL
model.type = "Matrix";
model.matrix2.type = "Lumped";
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
model.model.lattice.child.cross_section = "square";
model.model.lattice.child.side_length = 0.05;
model.model.lattice.child.density = 7850.;
model.model.lattice.child.dofNamesTrans = params.dofNames;
model.model.lattice.child.dofNamesRot = params.rotNames;

model.model.load.type = "Multi";
model.model.load.models = [ "fixed", "impact", "initial" ];
model.model.load.fixed.type = "Dirichlet";
model.model.load.fixed.dispIncr = 0.;
model.model.load.fixed.nodeGroups = [ "all", "zmin", "zmin", "zmin", "all", "all" ];
model.model.load.fixed.dofs = [ "dx", "dy", "dz", "rx", "ry", "rz" ];
model.model.load.fixed.factors = [ 0., 0., 0., 0., 0., 0. ]; 
model.model.load.impact.type = "LoadScale";
model.model.load.impact.scaleFunc = "-0.1 * (80*PI)^2 * sin(80*PI * t)";
model.model.load.impact.model.type = "Dirichlet";
model.model.load.impact.model.nodeGroups = [ "zmax" ];
model.model.load.impact.model.dofs = [ "dy" ];
model.model.load.impact.model.factors = [ 1. ]; 
model.model.load.initial.type = "InitLoad";
model.model.load.initial.veloGroups = "zmax";
model.model.load.initial.veloDofs = "dy";
model.model.load.initial.veloVals = "0.1 * (80*PI)";


// OUTPUTS
Output.modules = [ "pbcOut", "paraview", "view" ];

Output.pbcOut.type = "GroupOutput";
Output.pbcOut.nodeGroups = "zmax";
Output.pbcOut.dofs = "dy";

Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/step%d_$(Solver.solver.type)";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.disps = params.dofNames;
Output.paraview.beams.otherDofs = params.rotNames;
Output.paraview.beams.el_data = [ "strain", "stress", "mat_strain", "mat_stress" ];
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.shape = "Line2";
Output.paraview.sampleWhen = "i % 100 < 1";

Output.view.type = "Graph";
Output.view.dataSets = "disp_tip";
Output.view.disp_tip.yData = "zmax.disp.dy";
Output.view.disp_tip.xData = "t";
