include "../../studies/programs/nonlin-comp.pro";

Input.input.verbose = true;
Input.input.file="studies/geometries/re-entrant.geo";
model.model.load.model.H11="nan";
model.model.load.model.H22=-1.;
model.model.lattice.child.material.type = "ElastoPlasticRod";
model.model.lattice.child.material.yieldCond = "abs(ry/2.5e-3) + (dz/10)^2*(1+(dx/10)^2) + (dx/10) - 1";
model.model.lattice.child.material.side_length=0.1e-3;
Output.sampling.dataSets	= [ "(ymax.disp.dy - ymin.disp.dy) / all.extent.dy", "tangentModuli.E_x", "tangentModuli.E_y", "tangentModuli.nu_xy",
      "tangentModuli.nu_yx", "tangentModuli.G_xy", "tangentModuli.G_yx" ];
Output.sampling.append	= false;
Output.sampling.header	= "H22,E_x,E_y,nu_xy,nu_yx,G_xy,G_yx";
Output.modules+="paraview";
Output.paraview.beams.shape = "Line2";
Output.paraview.beams.el_data += "plast_strain";

Output.sampling.sampleWhen = "(i-1) % 10 < 1";
Output.tangent.sampleWhen = "(i-1) % 10 < 1";
Output.paraview.sampleWhen = "(i-1) % 10 < 1";

log.pattern = "*";
log.file = "$(CASE_NAME)_plast.log";
Output.paraview.output_format = "$(CASE_NAME)/plast_vis%i";
Output.sampling.file = "$(CASE_NAME)_plast.csv";

// LOGGING