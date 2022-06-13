// OUTPUTS
Output.modules = [  "loadextent", "disp" ];

Output.loadextent.type = "GroupOutput";
Output.loadextent.nodeGroups = [ "fixed", "free" ];
Output.loadextent.dofs = model.model.model.rodMesh.child.dofNamesTrans;
Output.loadextent.dofs += model.model.model.rodMesh.child.dofNamesRot;
Output.loadextent.dimensions = model.model.model.rodMesh.child.dofNamesTrans;

Output.disp.type = "StateOutput";

// LOGGING
log.pattern = "*.info";
log.file = "-";