///////////////////////////////////
/////// HERRNBÖCK ET AL. 5.4 ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //

// PROGRAM_CONTROL
control.runWhile = "force.factor <= 1";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.material.type = "ElastoPlasticRod";
params.rod_details.material.young = "9*164.210e9*80.193e9/(3*164.210e9+80.193e9)";
params.rod_details.material.shear_modulus = 80.193e9;
params.rod_details.material.cross_section = "circle";
params.rod_details.material.radius = 1e-3;
params.rod_details.material.yieldCond  = "  abs(dx/( 700-h_dx))^2.04 ";
params.rod_details.material.yieldCond += "+ abs(dy/( 700-h_dy))^2.04 "; 
params.rod_details.material.yieldCond += "+ abs(dz/(1470-h_dz))^1.76 ";
params.rod_details.material.yieldCond += "+ abs(rx/(0.62-h_rx))^2.09 ";
params.rod_details.material.yieldCond += "+ abs(ry/(0.62-h_ry))^2.09 ";
params.rod_details.material.yieldCond += "+ abs(rz/(0.56-h_rz))^1.73 ";
params.rod_details.material.yieldCond += "- 1";
params.rod_details.material.yieldDeriv  = ["2.04 * abs(dx/( 700-h_dx))^1.04 * if(dx/( 700-h_dx)>0, 1, if(dx/( 700-h_dx)<0, -1, 0)) / ( 700-h_dx)"];
params.rod_details.material.yieldDeriv += ["2.04 * abs(dy/( 700-h_dy))^1.04 * if(dy/( 700-h_dy)>0, 1, if(dy/( 700-h_dy)<0, -1, 0)) / ( 700-h_dy)"];
params.rod_details.material.yieldDeriv += ["1.76 * abs(dz/(1470-h_dz))^0.76 * if(dz/(1470-h_dz)>0, 1, if(dz/(1470-h_dz)<0, -1, 0)) / (1470-h_dz)"];
params.rod_details.material.yieldDeriv += ["2.09 * abs(rx/(0.62-h_rx))^1.09 * if(rx/(0.62-h_rx)>0, 1, if(rx/(0.62-h_rx)<0, -1, 0)) / (0.62-h_rx)"];
params.rod_details.material.yieldDeriv += ["2.09 * abs(ry/(0.62-h_ry))^1.09 * if(ry/(0.62-h_ry)>0, 1, if(ry/(0.62-h_ry)<0, -1, 0)) / (0.62-h_ry)"];
params.rod_details.material.yieldDeriv += ["1.73 * abs(rz/(0.56-h_rz))^0.73 * if(rz/(0.56-h_rz)>0, 1, if(rz/(0.56-h_rz)<0, -1, 0)) / (0.56-h_rz)"];
params.rod_details.material.yieldDeriv += ["2.04 * abs(dx/( 700-h_dx))^1.04 * if(dx/( 700-h_dx)>0, 1, if(dx/( 700-h_dx)<0, -1, 0)) * dx/(( 700-h_dx)^2)"];
params.rod_details.material.yieldDeriv += ["2.04 * abs(dy/( 700-h_dy))^1.04 * if(dy/( 700-h_dy)>0, 1, if(dy/( 700-h_dy)<0, -1, 0)) * dy/(( 700-h_dy)^2)"];
params.rod_details.material.yieldDeriv += ["1.76 * abs(dz/(1470-h_dz))^0.76 * if(dz/(1470-h_dz)>0, 1, if(dz/(1470-h_dz)<0, -1, 0)) * dz/((1470-h_dz)^2)"];
params.rod_details.material.yieldDeriv += ["2.09 * abs(rx/(0.62-h_rx))^1.09 * if(rx/(0.62-h_rx)>0, 1, if(rx/(0.62-h_rx)<0, -1, 0)) * rx/((0.62-h_rx)^2)"];
params.rod_details.material.yieldDeriv += ["2.09 * abs(ry/(0.62-h_ry))^1.09 * if(ry/(0.62-h_ry)>0, 1, if(ry/(0.62-h_ry)<0, -1, 0)) * ry/((0.62-h_ry)^2)"];
params.rod_details.material.yieldDeriv += ["1.73 * abs(rz/(0.56-h_rz))^0.73 * if(rz/(0.56-h_rz)>0, 1, if(rz/(0.56-h_rz)<0, -1, 0)) * rz/((0.56-h_rz)^2)"];

params.rod_details.material.kinematicTensor = [19014e+0, 17547e+0, 33121e+0, 16069e-3, 16743e-3, 15552e-3,
                                               17547e+0, 19014e+0, 33121e+0, 16743e-3, 16069e-3, 15556e-3,
                                               33121e+0, 33121e+0, 56864e+0, 24578e-3, 24578e-3, 26757e-3,
                                               16069e-3, 16743e-3, 24578e-3, 15015e-6, 15009e-6, 12715e-6,
                                               16743e-3, 16069e-3, 24578e-3, 15009e-6, 16015e-6, 12715e-6,
                                               15552e-3, 15556e-3, 26757e-3, 12715e-6, 12715e-6, 10434e-6];

params.force_model.type = "Dirichlet";
params.force_model.maxDisp = 1.;
params.force_model.initDisp = 0.;
params.force_model.dispIncr = 0.01;
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
model.model.model.diriFixed.dofs += ["dz", "rx", "ry", "rz"];
model.model.model.diriFixed.factors += [ 0., 0., 0., 0. ];

Output.disp.dataSets += "force.factor";
Output.resp.dataSets += "force.factor";

// Output.paraview.type = "None";
Output.paraview.sampleWhen = "force.factor%0.01<$(params.force_model.dispIncr)/2";
Output.paraview.beams.shape = "Line2";
Output.paraview.beams.el_data += "plast_strain";
