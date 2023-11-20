// PROGRAM CONTROL
control.runWhile = "loadScale <= 0.1";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "AdaptiveStep";
Solver.solver.loadIncr = 0.001;
Solver.solver.minIncr = 0.00005;

// SETUP
// material settings

params.material.young = 210e9;
params.material.poisson_ratio = 0.3;
params.material.cross_section = "square";
// pbc settings
params.pbcDofs = [ "dx", "dy" ];

params.load.type = "PeriodicBC";
params.load.dofs = ["dx", "dy"];
params.load.rotDofs = [ "rz" ];
params.load.H11="nan";
params.load.H22=-1.;

// 2D settings
params.dofs2D = ["dz", "rx", "ry"];
params.groups2D = [ "all", "all", "all" ];

// standard configs
include "../../studies/configs/input.pro";
include "../../studies/configs/model.pro";
include "../../studies/configs/output.pro";

// params.scale = 1;
// params.name = "elastic";

Input.input.onelab.BaseWidth = "$(params.scale) * 1e-2";
model.model.lattice.child.material.radius = "$(params.scale) * 1e-3";

Input.input.verbose = true;
Input.input.file="studies/geometries/re-entrant.geo";
Input.input.onelab.Angle='64.471220634490691369245999339962435963006843100907948288171106356';
Input.input.onelab.LengthRatio='0.75';
Input.input.onelab.Elems = '25';
model.model.lattice.child.material.type = "ElastoPlasticRod";
model.model.lattice.child.material.cross_section = "circle";
model.model.lattice.child.material.tolerance = 1e-4;

// model.model.lattice.child.material.yieldCond  = "  abs(dx/ 700/$(params.scale)^2)^2.04 ";
// model.model.lattice.child.material.yieldCond += "+ abs(dy/ 700/$(params.scale)^2)^2.04 "; 
// model.model.lattice.child.material.yieldCond += "+ abs(dz/1470/$(params.scale)^2)^1.76 ";
// model.model.lattice.child.material.yieldCond += "+ abs(rx/0.62/$(params.scale)^3)^2.09 ";
// model.model.lattice.child.material.yieldCond += "+ abs(ry/0.62/$(params.scale)^3)^2.09 ";
// model.model.lattice.child.material.yieldCond += "+ abs(rz/0.56/$(params.scale)^3)^1.73 ";
// model.model.lattice.child.material.yieldCond += "- 1";

// model.model.lattice.child.material.yieldCond  = "  abs(dx/ 700/$(params.scale)^2)^2.04 ";
// model.model.lattice.child.material.yieldCond += "+ abs(dy/ 700/$(params.scale)^2)^2.04 "; 
// model.model.lattice.child.material.yieldCond += "+ abs(dz/1470/$(params.scale)^2)^1.76 ";
// model.model.lattice.child.material.yieldCond += "+ abs(rx/0.62/$(params.scale)^3)^2.09 ";
// model.model.lattice.child.material.yieldCond += "+ abs(ry/0.62/$(params.scale)^3)^2.09 ";
// model.model.lattice.child.material.yieldCond += "+ abs(rz/0.56/$(params.scale)^3)^1.73 ";
// model.model.lattice.child.material.yieldCond += "- (1 + h_0)";
// model.model.lattice.child.material.isotropicCoefficient = "500/$(params.scale)^2";

// model.model.lattice.child.material.yieldCond  = "  abs(dx/( 700-h_dx)/$(params.scale)^2)^2.04 ";
// model.model.lattice.child.material.yieldCond += "+ abs(dy/( 700-h_dy)/$(params.scale)^2)^2.04 "; 
// model.model.lattice.child.material.yieldCond += "+ abs(dz/(1470-h_dz)/$(params.scale)^2)^1.76 ";
// model.model.lattice.child.material.yieldCond += "+ abs(rx/(0.62-h_rx)/$(params.scale)^3)^2.09 ";
// model.model.lattice.child.material.yieldCond += "+ abs(ry/(0.62-h_ry)/$(params.scale)^3)^2.09 ";
// model.model.lattice.child.material.yieldCond += "+ abs(rz/(0.56-h_rz)/$(params.scale)^3)^1.73 ";
// model.model.lattice.child.material.yieldCond += "- 1";
// model.model.lattice.child.material.kinematicTensor = ["19014e+0/$(params.scale)^2","17547e+0/$(params.scale)^2","33121e+0/$(params.scale)^2","16069e-3/$(params.scale)^2","16743e-3/$(params.scale)^2","15552e-3/$(params.scale)^2",
//                                                       "17547e+0/$(params.scale)^2","19014e+0/$(params.scale)^2","33121e+0/$(params.scale)^2","16743e-3/$(params.scale)^2","16069e-3/$(params.scale)^2","15556e-3/$(params.scale)^2",
//                                                       "33121e+0/$(params.scale)^2","33121e+0/$(params.scale)^2","56864e+0/$(params.scale)^2","24578e-3/$(params.scale)^2","24578e-3/$(params.scale)^2","26757e-3/$(params.scale)^2",
//                                                       "16069e-3/$(params.scale)^2","16743e-3/$(params.scale)^2","24578e-3/$(params.scale)^2","15015e-6/$(params.scale)^2","15009e-6/$(params.scale)^2","12715e-6/$(params.scale)^2",
//                                                       "16743e-3/$(params.scale)^2","16069e-3/$(params.scale)^2","24578e-3/$(params.scale)^2","15009e-6/$(params.scale)^2","16015e-6/$(params.scale)^2","12715e-6/$(params.scale)^2",
//                                                       "15552e-3/$(params.scale)^2","15556e-3/$(params.scale)^2","26757e-3/$(params.scale)^2","12715e-6/$(params.scale)^2","12715e-6/$(params.scale)^2","10434e-6/$(params.scale)^2"];



Output.modules = [ "tangent", "sampling"];
// overwriting settings

Output.sampling.dataSets	= [ "(ymax.disp.dy - ymin.disp.dy) / all.extent.dy", "tangent.stiffness[15]" ];
Output.sampling.append	= false;
Output.sampling.header	= "H22, C_44";
Output.paraview.beams.shape = "Line2";
// Output.paraview.beams.el_data += "plast_strain";

Output.sampling.sampleWhen = true;
Output.tangent.sampleWhen = true;
Output.tangent.thickness = model.model.lattice.child.material.radius;

// LOGGING
log.pattern = "*";
log.file = "$(CASE_NAME)_$(params.name).log";
Output.sampling.file = "$(CASE_NAME)_$(params.name).csv";
Output.paraview.output_format = "$(CASE_NAME)/$(params.name)_vis%i";
