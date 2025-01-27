
// LOGGING
log.pattern = "*.info | *.debug";
log.file = "-";

// OUTPUTS
Output.modules = [  "loadextent", "disp", "resp", "paraview" ];

Output.loadextent.type = "GroupOutput";
Output.loadextent.nodeGroups = "free";
Output.loadextent.dofs = model.model.model.lattice.child.dofNamesTrans;
Output.loadextent.dofs += model.model.model.lattice.child.dofNamesRot;
Output.loadextent.dimensions = model.model.model.lattice.child.dofNamesTrans;

Output.disp.type = "Sample";
Output.disp.file = "$(CASE_NAME)/disp.csv";
Output.disp.dataSets = [ "free.disp.dx", "free.disp.dy", "free.disp.dz", "free.disp.rx", "free.disp.ry", "free.disp.rz" ];
Output.disp.separator = ",";

Output.resp.type = "Sample";
Output.resp.file = "$(CASE_NAME)/resp.csv";
Output.resp.dataSets = [ "free.resp.dx", "free.resp.dy", "free.resp.dz", "free.resp.rx", "free.resp.ry", "free.resp.rz" ];
Output.resp.separator = ",";

Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%i";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.disps = model.model.model.lattice.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.model.lattice.child.dofNamesRot;
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain"];
