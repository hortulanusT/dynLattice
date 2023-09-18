///////////////////////////////////
/////// HERRNBÖCK ET AL. 5.4 ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //

// PROGRAM_CONTROL
control.runWhile = "i<1001";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.material.type = "ElastoPlasticRod";
params.rod_details.material.young = "9*164210e9*80193e9/(3*164210e9+80193e9)";
params.rod_details.material.shear_modulus = 80193e9;
params.rod_details.material.cross_section = "circle";
params.rod_details.material.radius = 1e-3;
params.scale=1.;
params.rod_details.material.yieldCond  = "  abs(dx/( 700-h_dx)/$(params.scale)^2)^2.04 ";
params.rod_details.material.yieldCond += "+ abs(dy/( 700-h_dy)/$(params.scale)^2)^2.04 "; 
params.rod_details.material.yieldCond += "+ abs(dz/(1470-h_dz)/$(params.scale)^2)^1.76 ";
params.rod_details.material.yieldCond += "+ abs(rx/(0.62-h_rx)/$(params.scale)^3)^2.09 ";
params.rod_details.material.yieldCond += "+ abs(ry/(0.62-h_ry)/$(params.scale)^3)^2.09 ";
params.rod_details.material.yieldCond += "+ abs(rz/(0.56-h_rz)/$(params.scale)^3)^1.73 ";
params.rod_details.material.yieldCond += "- 1";
params.rod_details.material.kinematicTensor = ["19014e+0/$(params.scale)^2","17547e+0/$(params.scale)^2","33121e+0/$(params.scale)^2","16069e-3/$(params.scale)^2","16743e-3/$(params.scale)^2","15552e-3/$(params.scale)^2",
                                               "17547e+0/$(params.scale)^2","19014e+0/$(params.scale)^2","33121e+0/$(params.scale)^2","16743e-3/$(params.scale)^2","16069e-3/$(params.scale)^2","15556e-3/$(params.scale)^2",
                                               "33121e+0/$(params.scale)^2","33121e+0/$(params.scale)^2","56864e+0/$(params.scale)^2","24578e-3/$(params.scale)^2","24578e-3/$(params.scale)^2","26757e-3/$(params.scale)^2",
                                               "16069e-3/$(params.scale)^2","16743e-3/$(params.scale)^2","24578e-3/$(params.scale)^2","15015e-6/$(params.scale)^2","15009e-6/$(params.scale)^2","12715e-6/$(params.scale)^2",
                                               "16743e-3/$(params.scale)^2","16069e-3/$(params.scale)^2","24578e-3/$(params.scale)^2","15009e-6/$(params.scale)^2","16015e-6/$(params.scale)^2","12715e-6/$(params.scale)^2",
                                               "15552e-3/$(params.scale)^2","15556e-3/$(params.scale)^2","26757e-3/$(params.scale)^2","12715e-6/$(params.scale)^2","12715e-6/$(params.scale)^2","10434e-6/$(params.scale)^2"];

params.force_model.type = "Dirichlet";
params.force_model.maxDisp = 1.;
params.force_model.initDisp = 0.;
params.force_model.dispIncr = 1e-3;
params.force_model.nodeGroups = ["fixed_right", "free"];
params.force_model.dofs = ["ry", "dx"];
params.force_model.factors = ["1.*PI/2.", "0.04"]; 

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += [ "rx", "rz" ];
model.model.model.diriFixed.factors += [ 0., 0. ];

model.model.model.diriFixed.nodeGroups += [ "free", "free", "free", "free" ];
model.model.model.diriFixed.dofs += ["dz","rx","ry","rz"];
model.model.model.diriFixed.factors += [ 0., 0., 0., 0. ];

Output.paraview.sampleWhen = "i%10<1";
Output.paraview.beams.shape = "Line2";
Output.paraview.beams.el_data += "plast_strain";
