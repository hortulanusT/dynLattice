
// LOGGING
log.pattern = "*.info | *.debug";
log.file = "-";

// OUTPUTS
Output.modules = [ "loadextent", "disp", "resp", "paraview" ];

Output.loadextent.type = "GroupOutput";
Output.loadextent.nodeGroups = "fixed_left";
Output.loadextent.dofs = model.model.model.lattice.child.dofNamesTrans;
Output.loadextent.dofs += model.model.model.lattice.child.dofNamesRot;
Output.loadextent.dimensions = model.model.model.lattice.child.dofNamesTrans;

Output.disp.type = "Sample";
Output.disp.file = "$(CASE_NAME)/disp.csv";
Output.disp.dataSets = [ "fixed_left.disp.dx", "fixed_left.disp.dy", "fixed_left.disp.dz", "fixed_left.disp.rx", "fixed_left.disp.ry", "fixed_left.disp.rz" ];
Output.disp.separator = ",";

Output.resp.type = "Sample";
Output.resp.file = "$(CASE_NAME)/resp.csv";
Output.resp.dataSets = [ "fixed_left.resp.dx", "fixed_left.resp.dy", "fixed_left.resp.dz", "fixed_left.resp.rx", "fixed_left.resp.ry", "fixed_left.resp.rz" ];
Output.resp.separator = ",";

Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%d";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.disps = model.model.model.lattice.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.model.lattice.child.dofNamesRot;
Output.paraview.beams.node_data = ["fint", "fext", "fres", "F_contact"];
Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain"];
