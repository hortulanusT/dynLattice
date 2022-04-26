// ACTUAL_MODEL
model.type = "Debug";
model.noiseLevel = 0;
model.model.type = "Matrix";
model.model.model.type = "Multi";
model.model.model.models = [ "lattice", "diriFixed", "force", "joint" ];
model.model.model.lattice.type = "Lattice";
model.model.model.lattice.prefix = "beam_";
model.model.model.lattice.child.type = "specialCosseratRod";
model.model.model.lattice.child.dofNamesTrans = ["dx", "dy", "dz"];
model.model.model.lattice.child.dofNamesRot = ["rx", "ry", "rz"];
model.model.model.lattice.child += params.rod_details;

model.model.model.diriFixed.type = "Dirichlet";
model.model.model.diriFixed.maxDisp = 0.;
model.model.model.diriFixed.dispIncr =  0.;
model.model.model.diriFixed.nodeGroups = [ "fixed_left", "fixed_left", "fixed_left" ];
model.model.model.diriFixed.dofs = model.model.model.lattice.child.dofNamesTrans;
model.model.model.diriFixed.factors = [ 0., 0., 0. ]; 
model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.lattice.child.dofNamesTrans;
model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 

model.model.model.force = params.force_model;

model.model.model.joint.type = "rodJoint";
model.model.model.joint.elements = "points";
model.model.model.joint.lockDofs = [];
model.model.model.joint.lockDofs += model.model.model.lattice.child.dofNamesTrans;