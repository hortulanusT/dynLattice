// ACTUAL_MODEL
model.type = "Debug";
model.noiseLevel = 2;
model.model.type = "Matrix";
model.model.model.type = "Multi";
model.model.model.models = [ "cosseratRod", "diriFixed", "force", "joint" ]; 
model.model.model.cosseratRod.type = "specialCosseratRod";
model.model.model.cosseratRod.elements = "rods";
model.model.model.cosseratRod.dofNamesTrans = ["dx", "dy", "dz"];
model.model.model.cosseratRod.dofNamesRot = ["rx", "ry", "rz"];

model.model.model.diriFixed.type = "Dirichlet";
model.model.model.diriFixed.maxDisp = 0.;
model.model.model.diriFixed.dispIncr =  0.;
model.model.model.diriFixed.nodeGroups = [ "fixed", "fixed", "fixed" ];
model.model.model.diriFixed.dofs = model.model.model.cosseratRod.dofNamesTrans;
model.model.model.diriFixed.factors = [ 0., 0., 0. ]; 
model.model.model.diriFixed.nodeGroups += [ "fixed", "fixed", "fixed" ];
model.model.model.diriFixed.dofs += model.model.model.cosseratRod.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ]; 

model.model.model.joint.type = "rodJoint";
model.model.model.joint.elements = "joints";
model.model.model.joint.lockDofs = [];
model.model.model.joint.lockDofs += model.model.model.cosseratRod.dofNamesTrans;
model.model.model.joint.lockDofs += model.model.model.cosseratRod.dofNamesRot;