// ACTUAL_MODEL
model.type = "Debug";
model.noiseLevel = 0;
model.model.type = "Matrix";
model.model.model.type = "Multi";
model.model.model.models = [ "rodMesh", "fixed", "force", "disp" ]; 
model.model.model.rodMesh.type = "Lattice";
model.model.model.rodMesh.prefix = "beam_";
model.model.model.rodMesh.child.type = "specialCosseratRod";
model.model.model.rodMesh.child.material.type = "ElasticMat";
model.model.model.rodMesh.child.material.young = 1000.;
model.model.model.rodMesh.child.material.shear_modulus = 1000.;
model.model.model.rodMesh.child.material.area = 1.;
model.model.model.rodMesh.child.material.area_moment = 1.;
model.model.model.rodMesh.child.material.shear_correction = 1.;
model.model.model.rodMesh.child.material.polar_moment = 1.;
model.model.model.rodMesh.child.dofNamesTrans = ["dx", "dy", "dz"];
model.model.model.rodMesh.child.dofNamesRot = ["rx", "ry", "rz"];
model.model.model.rodMesh.child.shape.numPoints = 0;

model.model.model.fixed.type = "Dirichlet";
model.model.model.fixed.maxDisp = 0.;
model.model.model.fixed.dispIncr =  0.;
model.model.model.fixed.nodeGroups = [ "fixed", "fixed", "fixed" ];
model.model.model.fixed.dofs = model.model.model.rodMesh.child.dofNamesTrans;
model.model.model.fixed.factors = [ 0., 0., 0. ]; 
model.model.model.fixed.nodeGroups += [ "fixed", "fixed", "fixed" ];
model.model.model.fixed.dofs += model.model.model.rodMesh.child.dofNamesRot;
model.model.model.fixed.factors += [ 0., 0., 0. ]; 

model.model.model.force.type = "Neumann";
model.model.model.force.initLoad = 0.;
model.model.model.force.loadIncr = params.Incr;
model.model.model.force.nodeGroups =  [ "fixed" ] ;
model.model.model.force.factors = [ 0. ];
model.model.model.force.dofs = [ "dx" ];

model.model.model.disp.type = "Dirichlet";
model.model.model.disp.initDisp = "$(params.Incr) * 1e-3";
model.model.model.disp.dispIncr = "$(params.Incr) * 1e-3";
model.model.model.disp.nodeGroups =  [ "fixed" ] ;
model.model.model.disp.factors = [ 0. ];
model.model.model.disp.dofs = [ "dx" ];
