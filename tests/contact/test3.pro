///////////////////////////////////
/////// FREE RODS, TIP TO TIP /////
///////////////////////////////////

// Two equal free elastic rods on one line meet tip to tip through the joint contact: as for equal bars
// (Saint-Venant), their velocities are exchanged after the waves have travelled there and back.

// PROGRAM_CONTROL
control.runWhile = "t < 0.1";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "LeapFrog";
Solver.integrator.deltaTime = 2e-5;
Solver.integrator.minDTime = 2e-5;
Solver.integrator.maxDTime = 2e-5;
Solver.integrator.dofs_SO3 = [ "rx", "ry", "rz" ];
Solver.integrator.updateWhen = false;

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.cross_section = "circle";
params.rod_details.material.radius = 0.02;
params.rod_details.material.young = 1e4;
params.rod_details.material.poisson_ratio = 0.3;
params.rod_details.material.density = 1.;

// INPUT
Input.modules = [ "input", "groupInput" ];
Input.input.type = "GMSHInput";
Input.input.file = "$(CASE_NAME).geo";
Input.groupInput.type = "GroupInput";
Input.groupInput.nodeGroups = [ "rodA", "rodB" ];
Input.groupInput.rodA.ybounds = [ -2., 0. ];
Input.groupInput.rodB.ybounds = [ 0., 2. ];

// ACTUAL_MODEL
model.type = "Matrix";
model.model.type = "Multi";
model.model.models = [ "lattice", "init" ];
model.model.lattice.type = "Lattice";
model.model.lattice.prefix = "beam_";
model.model.lattice.child.type = "specialCosseratRod";
model.model.lattice.child.dofNamesTrans = ["dx", "dy", "dz"];
model.model.lattice.child.dofNamesRot = ["rx", "ry", "rz"];
model.model.lattice.child += params.rod_details;
model.model.lattice.jointContact.type = "JointContact";
model.model.lattice.jointContact.penalty = 1000.;
model.model.lattice.jointContact.radius = 0.05;

model.model.init.type = "InitLoad";
model.model.init.veloGroups = [ "rodA", "rodB" ];
model.model.init.veloDofs = [ "dy", "dy" ];
model.model.init.veloVals = [ 1., -1. ];

// OUTPUTS
Output.modules = [ "loadextent", "disp", "resp", "state", "paraview" ];
Output.loadextent.type = "GroupOutput";
Output.loadextent.nodeGroups = [ "rodA", "rodB" ];
Output.loadextent.dofs = model.model.lattice.child.dofNamesTrans;
Output.loadextent.dimensions = model.model.lattice.child.dofNamesTrans;
Output.disp.type = "Sample";
Output.disp.file = "$(CASE_NAME)/disp.csv";
Output.disp.separator = ",";
Output.disp.dataSets = [ "t", "potentialEnergy", "kineticEnergy" ];
Output.resp.type = "Sample";
Output.resp.file = "$(CASE_NAME)/resp.csv";
Output.resp.separator = ",";
Output.resp.dataSets = [ "t", "rodA.resp.dy", "rodB.resp.dy" ];
Output.resp.sampleWhen = "i % 100 < 1";
Output.state.type = "CSVOutput";
Output.state.file = "$(CASE_NAME)/state.gz";
Output.state.vectors = [ "state = disp", "state1 = velo" ];
Output.state.saveWhen = "t % 0.001 < deltaTime";
Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%d";
Output.paraview.sampleWhen = "t % 0.001 < deltaTime";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.shape = "Line2";
Output.paraview.beams.disps = model.model.lattice.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.lattice.child.dofNamesRot;
Output.paraview.beams.node_data = [ "fint", "fext", "fres" ];
Output.paraview.beams.el_data = [ "strain", "stress" ];

// LOGGING
log.pattern = "*.warn";
log.file = "-";
