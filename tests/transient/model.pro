// ACTUAL_MODEL
model.type = "Matrix";
model.model.type = "Multi";

model.model.models = [ "rodMesh", "fixed", "force", "disp" ]; 
model.model.rodMesh.type = "Lattice";
model.model.rodMesh.prefix = "beam_";
model.model.rodMesh.child.type = "specialCosseratRod";
model.model.rodMesh.child.dofNamesTrans = ["dx", "dy", "dz"];
model.model.rodMesh.child.dofNamesRot = ["rx", "ry", "rz"];
model.model.rodMesh.child += params.rod_details;

model.model.fixed.type = "Dirichlet";
model.model.fixed.maxDisp = 0.;
model.model.fixed.dispIncr =  0.;
model.model.fixed.nodeGroups = [ "fixed", "fixed", "fixed" ];
model.model.fixed.dofs = model.model.rodMesh.child.dofNamesTrans;
model.model.fixed.factors = [ 0., 0., 0. ]; 
model.model.fixed.nodeGroups += [ "fixed", "fixed", "fixed" ];
model.model.fixed.dofs += model.model.rodMesh.child.dofNamesRot;
model.model.fixed.factors += [ 0., 0., 0. ]; 

model.model.force.type = "Neumann";
model.model.force.initLoad = 0.;
model.model.force.loadIncr = 0.;
model.model.force.nodeGroups =  [ "fixed" ] ;
model.model.force.factors = [ 0. ];
model.model.force.dofs = [ "dx" ];

model.model.disp.type = "Dirichlet";
model.model.disp.initDisp = 0.;
model.model.disp.dispIncr = 0.;
model.model.disp.nodeGroups =  [ "fixed" ] ;
model.model.disp.factors = [ 0. ];
model.model.disp.dofs = [ "dx" ];
