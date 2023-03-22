// OUTPUTS
Output.modules = [  "loadextent", "disp" ];

Output.loadextent.type = "GroupOutput";
Output.loadextent.nodeGroups = [ "fixed", "free" ];
Output.loadextent.dofs = model.model.rodMesh.child.dofNamesTrans;
Output.loadextent.dofs += model.model.rodMesh.child.dofNamesRot;
Output.loadextent.dimensions = model.model.rodMesh.child.dofNamesTrans;

Output.disp.type = "CSVOutput";
Output.disp.file = "$(CASE_NAME)/disp.gz";
Output.disp.vectors = [ "state = disp" ];

// LOGGING
log.pattern = "*";
log.file = "-";
