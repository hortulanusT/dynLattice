// LOGGING
log.pattern = "*.info";
log.file="$(CASE_NAME)/run.log";

// OUTPUTS
Output.modules = ["groupOut", "sampling", "paraview"];

Output.groupOut.type = "GroupOutput";
Output.groupOut.nodeGroups = ["ymax", "ymin", "plate1_bot"];
Output.groupOut.dofs = "dy";

Output.sampling.type = "Sample";
Output.sampling.file = "$(CASE_NAME)/data.csv";
Output.sampling.header='            time,          v_ymax,          d_ymax,          F_ymax,          F_ymin,            H_yy,        P_yy_top,        P_yy_bot,           E_pot,          E_diss,           E_kin,            Mass,        F_plate1';
Output.sampling.dataSets=['t', 'ymax.velo.dy', 'ymax.disp.dy', 'ymax.resp.dy', 'ymin.resp.dy', 'ymax.disp.dy/SIZE.Y', 'ymax.resp.dy/(SIZE.X/2.0)/0.00075', 'ymin.resp.dy/SIZE.X/0.00075', 'potentialEnergy', 'dissipatedEnergy','kineticEnergy', 'mass', "plate1_bot.resp.dy"];

Output.sampling.separator = ",";

Output.paraview.type = "ParaView";
Output.paraview.groups = [ "beams","plate1" ];
Output.paraview.beams.disps = ["dx", "dy", "dz"];
Output.paraview.beams.otherDofs = ["rx", "ry", "rz"];
Output.paraview.beams.el_data = [ "mat_strain", "mat_stress", "plast_strain" ];
Output.paraview.beams.node_data = ["fres", "F_contact", 'kineticEnergy', 'mass'];
Output.paraview.beams.shape = "Line2";
Output.paraview.plate1 = Output.paraview.beams;
Output.paraview.plate1.el_data = [ "mat_strain", "mat_stress" ];
Output.paraview.plate1.node_data = ["fres"];
Output.paraview.output_format = "$(CASE_NAME)/vis%i";
Output.paraview.sampleWhen = "(i-1)%100<1";