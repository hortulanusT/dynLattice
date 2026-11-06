// ACTUAL_MODEL
model.type = "Matrix";
model.model.type = "Multi";

model.model.models = [ "rodMesh", "fixed", "force", "init" ];
model.model.rodMesh.type = "Lattice";
model.model.rodMesh.prefix = "beam_";
model.model.rodMesh.child.type = "specialCosseratRod";
model.model.rodMesh.child.dofNamesTrans = ["dx", "dy", "dz"];
model.model.rodMesh.child.dofNamesRot = ["rx", "ry", "rz"];
model.model.rodMesh.child += params.rod_details;

model.model.fixed.type = "Dirichlet";
model.model.fixed.maxDisp = 0.;
model.model.fixed.dispIncr = 0.;
model.model.fixed.nodeGroups = [ "fixed", "fixed", "fixed", "fixed", "fixed", "fixed" ];
model.model.fixed.dofs = [ "dx", "dy", "dz", "rx", "ry", "rz" ];
model.model.fixed.factors = [ 0., 0., 0., 0., 0., 0. ];

model.model.force.type = "Neumann";
model.model.force.initLoad = 0.;
model.model.force.loadIncr = 0.;
model.model.force.nodeGroups = [ "free" ];
model.model.force.dofs = [ "dx" ];
model.model.force.factors = [ 0. ];

model.model.init.type = "InitLoad";
