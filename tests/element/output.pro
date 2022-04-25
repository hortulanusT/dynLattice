// OUTPUTS
Output.modules = [  "loadextent", "disp", "load", "resp" ];

Output.loadextent.type = "GroupOutput";
Output.loadextent.nodeGroups = [ "free" ];
Output.loadextent.dofs = model.model.model.cosseratRod.dofNamesTrans;
Output.loadextent.dofs += model.model.model.cosseratRod.dofNamesRot;
Output.loadextent.dimensions = model.model.model.cosseratRod.dofNamesTrans;

Output.disp.type = "Sample";
Output.disp.file = "$(CASE_NAME)-disp.res";
Output.disp.dataSets = [ "free.disp.dx", "free.disp.dy", "free.disp.dz", "free.disp.rx", "free.disp.ry", "free.disp.rz" ];

Output.load.type = "Sample";
Output.load.file = "$(CASE_NAME)-load.res";
Output.load.dataSets = [ "free.load.dx", "free.load.dy", "free.load.dz", "free.load.rx", "free.load.ry", "free.load.rz" ];

Output.resp.type = "Sample";
Output.resp.file = "$(CASE_NAME)-resp.res";
Output.resp.dataSets = [ "free.resp.dx", "free.resp.dy", "free.resp.dz", "free.resp.rx", "free.resp.ry", "free.resp.rz" ];
