///////////////////////////////////
/////// SMRITI ET AL.  5.1 (no hard)  ///////
///////////////////////////////////

// LOGGING
log.pattern = "*.info | *.debug"; //

// PROGRAM_CONTROL
control.runWhile = "i<501";

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";

// SETTINGS
params.rod_details.material.type = "ElastoPlasticRod";
params.rod_details.material.young = 1e6;
params.rod_details.material.poisson_ratio = .4;
params.rod_details.material.shear_correction = "(6*1.4)/(7+9*0.4)";
params.rod_details.material.cross_section = "circle";
params.rod_details.material.radius = 0.05;
params.rod_details.material.yieldCond  = "  abs(dx+h_dx) ";
params.rod_details.material.yieldCond += "+ abs(dy+h_dy) "; 
params.rod_details.material.yieldCond += "+ abs(dz+h_dz) "; 
params.rod_details.material.yieldCond += "+ abs((rx+h_rx)/0.12) "; 
params.rod_details.material.yieldCond += "+ abs((ry+h_ry)/0.12) "; 
params.rod_details.material.yieldCond += "+ abs((rz+h_rz)/0.12) "; 
params.rod_details.material.yieldCond += "- 10 * (1+h_0)";
params.rod_details.material.isotropicCoefficient = 0.;
params.rod_details.material.kinematicTensor = [0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.];

params.force_model.type = "Multi";
params.force_model.models = [ "twist", "stretch"];

params.force_model.twist.type = "Dirichlet";
params.force_model.twist.maxDisp = 0.18;
params.force_model.twist.initDisp = 0.18;
params.force_model.twist.dispIncr = 0.;
params.force_model.twist.nodeGroups = "free";
params.force_model.twist.dofs = "ry";
params.force_model.twist.factors = 1.; 

params.force_model.stretch.type = "LoadScale";
params.force_model.stretch.scaleFunc =  "   if(i>  1, 0.031*(i-  1),0) - if(i>101, 0.031*(i-101),0)";
params.force_model.stretch.scaleFunc += " + if(i>101,-0.059*(i-101),0) - if(i>201,-0.059*(i-201),0)";
params.force_model.stretch.scaleFunc += " + if(i>201, 0.071*(i-201),0) - if(i>301, 0.071*(i-301),0)";
params.force_model.stretch.scaleFunc += " + if(i>301,-0.084*(i-301),0) - if(i>401,-0.084*(i-401),0)";
params.force_model.stretch.scaleFunc += " + if(i>401, 0.056*(i-401),0) - if(i>501, 0.056*(i-501),0)";
params.force_model.stretch.model.type = "Dirichlet";
params.force_model.stretch.model.nodeGroups =  [ "free" ] ;
params.force_model.stretch.model.factors = [ 1e-3 ];
params.force_model.stretch.model.dofs = [ "dy" ];


// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.model.model.diriFixed.nodeGroups += [ "fixed_right", "fixed_right", "fixed_right" ];
model.model.model.diriFixed.dofs += model.model.model.lattice.child.dofNamesRot;
model.model.model.diriFixed.factors += [ 0., 0., 0. ];

model.model.model.diriFixed.nodeGroups += [ "free", "free", "free", "free" ];
model.model.model.diriFixed.dofs += ["dx","dz","rx","rz"];
model.model.model.diriFixed.factors += [ 0., 0., 0., 0. ];

Output.paraview.sampleWhen = "i%10<1";
Output.paraview.beams.shape = "Line2";
Output.paraview.beams.el_data += "plast_strain";

log.file = "$(CASE_NAME)/run.log";
