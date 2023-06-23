include "../../studies/programs/nonlin-comp.pro";

control.runWhile = "(ymax.disp.dy - ymin.disp.dy) / all.extent.dy >= -.9";
Input.input.verbose = true;
Input.input.file="studies/geometries/re-entrant.geo";
Input.input.onelab.Angle='64.471220634490691369245999339962435963006843100907948288171106356';
Input.input.onelab.LengthRatio='0.75';
Input.input.onelab.Thickness='3.4506172839506174e-05';
// Input.input.onelab.RepX='5';
// Input.input.onelab.RepY='5';
model.model.load.model.H11="nan";
model.model.load.model.H22=-1.;
model.model.lattice.child.material.type = "ElastoPlasticRod";
model.model.lattice.child.material.side_length = 0.1e-3;
model.model.lattice.child.material.yieldCond = "abs(dx/2.05)^2.68 + abs(dy/2.05)^2.68 + abs(dz/4.675)^1.75 + abs(rx/0.0001175)^1.93 + abs(ry/0.0001175)^1.93 + abs(rz/0.00009375)^1.70 - 1";
// model.model.lattice.child.material.isotropicHardeningFactor = 1.;
// model.model.lattice.child.material.yieldCond = "abs(dx/2.05)^2.68 + abs(dy/2.05)^2.68 + abs(dz/4.675)^1.75 + abs(rx/0.0001175)^1.93 + abs(ry/0.0001175)^1.93 + abs(rz/0.00009375)^1.70 - (1+a)";
// model.model.lattice.child.material.kinematicHardeningFactors	= [ 47.535, 43.8675, 82.8025, 40.1725, 41.8575, 38.88,
//                                                                   43.8675, 47.535, 82.8025, 41.8575, 40.1725, 38.89,
//                                                                   82.8025, 82.8025, 142.16, 61.445, 61.445, 66.8925,
//                                                                   2.00863, 2.09288, 3.07225, 2.00188, 1.87613, 1.58938,
//                                                                   2.09288, 2.00863, 3.07225, 1.87613, 2.00188, 1.58938, 
//                                                                   1.944, 1.9445, 3.34463, 1.58938, 1.58938, 1.30425];
// model.model.lattice.child.material.yieldCond = "abs(dx/(2.05-b_dx))^2.68 + abs(dy/(2.05-b_dy))^2.68 + abs(dz/(4.675-b_dz))^1.75 + abs(rx/(0.0001175-b_rx))^1.93 + abs(ry/(0.0001175-b_ry))^1.93 + abs(rz/(0.00009375-b_rz))^1.70 - 1";

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