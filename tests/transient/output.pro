// OUTPUTS
Output.modules = [  "loadextent", "disp" ];

Output.loadextent.type = "GroupOutput";
Output.loadextent.nodeGroups = [ "fixed", "free" ];
Output.loadextent.dofs = model.model.model.rodMesh.child.dofNamesTrans;
Output.loadextent.dofs += model.model.model.rodMesh.child.dofNamesRot;
Output.loadextent.dimensions = model.model.model.rodMesh.child.dofNamesTrans;

Output.disp.type = "StateOutput";
Output.disp.file = "$(CASE_NAME)/disp.csv";
Output.disp.dofs = "dz";

// LOGGING
log.pattern = "*.info";
log.file = "-";