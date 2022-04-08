
// LOGGING
log.pattern = "*.info ";//| *.debug
log.file = "$(CASE_NAME)/run.log";

// OUTPUTS
Output.modules = [  "loadextent", "disp", "resp", "paraview" ];

Output.loadextent.type = "GroupOutput";
Output.loadextent.nodeGroups = "free";
Output.loadextent.dofs = model.model.model.cosseratRod.dofNamesTrans;
Output.loadextent.dofs += model.model.model.cosseratRod.dofNamesRot;
Output.loadextent.dimensions = model.model.model.cosseratRod.dofNamesTrans;

Output.disp.type = "Sample";
Output.disp.file = "$(CASE_NAME)/disp.csv";
Output.disp.dataSets = [ "free.disp.dx", "free.disp.dy", "free.disp.dz", "free.disp.rx", "free.disp.ry", "free.disp.rz" ];
Output.disp.separator = ",";

Output.resp.type = "Sample";
Output.resp.file = "$(CASE_NAME)/resp.csv";
Output.resp.dataSets = [ "free.resp.dx", "free.resp.dy", "free.resp.dz", "free.resp.rx", "free.resp.ry", "free.resp.rz" ];
Output.resp.separator = ",";

Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%d";
Output.paraview.groups = [ "rods" ];
Output.paraview.rods.shape = "Line$(params.rod_details.shape.numPoints)";
Output.paraview.rods.disps = model.model.model.cosseratRod.dofNamesTrans;
Output.paraview.rods.otherDofs = model.model.model.cosseratRod.dofNamesRot;
Output.paraview.rods.node_data = ["fint", "fext", "fres"];
Output.paraview.rods.el_data = ["strain", "stress", "mat_stress", "mat_strain"];