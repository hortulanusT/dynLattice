// Lang et al 2011 "Multibody Dynamics Simulation of geometrically exact Cossrat rods Example 1

// PROGRAM_CONTROL
control.runWhile = "t <= 1";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "MilneDevice";
Solver.integrator.deltaTime = 1e-6;
// Solver.integrator.precision = 1e-7;

// settings
params.rod_details.material.type = "ElastoPlasticRod";
params.rod_details.material.cross_section = "circle";
params.rod_details.material.radius = 5e-3;
params.rod_details.material.young = 5e6;
params.rod_details.material.poisson_ratio = 0.5;
params.rod_details.material.shear_correction = 1.;
params.rod_details.material.density = 1.1e3;
params.rod_details.material.yieldCond  = "  abs(dx/(0.700-h_dx)/(0.5)^2)^2.04 ";
params.rod_details.material.yieldCond += "+ abs(dy/(0.700-h_dy)/(0.5)^2)^2.04 "; 
params.rod_details.material.yieldCond += "+ abs(dz/(1.470-h_dz)/(0.5)^2)^1.76 ";
params.rod_details.material.yieldCond += "+ abs(rx/(0000.62-h_rx)/(0.5)^3)^2.09 ";
params.rod_details.material.yieldCond += "+ abs(ry/(0000.62-h_ry)/(0.5)^3)^2.09 ";
params.rod_details.material.yieldCond += "+ abs(rz/(0000.56-h_rz)/(0.5)^3)^1.73 ";
params.rod_details.material.yieldCond += "- 1";
params.rod_details.material.kinematicTensor = ["19014e-3/(0.5)^2","17547e-3/(0.5)^2","33121e-3/(0.5)^2","16069e-6/(0.5)^2","16743e-6/(0.5)^2","15552e-6/(0.5)^2",
                                              "17547e-3/(0.5)^2","19014e-3/(0.5)^2","33121e-3/(0.5)^2","16743e-6/(0.5)^2","16069e-6/(0.5)^2","15556e-6/(0.5)^2",
                                              "33121e-3/(0.5)^2","33121e-3/(0.5)^2","56864e-3/(0.5)^2","24578e-6/(0.5)^2","24578e-6/(0.5)^2","26757e-6/(0.5)^2",
                                              "16069e-6/(0.5)^2","16743e-6/(0.5)^2","24578e-6/(0.5)^2","15015e-9/(0.5)^2","15009e-9/(0.5)^2","12715e-9/(0.5)^2",
                                              "16743e-6/(0.5)^2","16069e-6/(0.5)^2","24578e-6/(0.5)^2","15009e-9/(0.5)^2","16015e-9/(0.5)^2","12715e-9/(0.5)^2",
                                              "15552e-6/(0.5)^2","15556e-6/(0.5)^2","26757e-6/(0.5)^2","12715e-9/(0.5)^2","12715e-9/(0.5)^2","10434e-9/(0.5)^2"];


// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

// more settings
Input.input.onelab.size = 0.1;

model.model.force.type = "Neumann";
model.model.force.loadIncr = 0.;
model.model.force.initLoad = "PI * (5e-3)^2 * $(Input.input.onelab.size) * 1.1e3 * 9.81";
model.model.force.nodeGroups =  [ "all", "free", "fixed" ] ;
model.model.force.factors = [ -1., 0.5, 0.5 ];
model.model.force.dofs = [ "dz", "dz", "dz" ];

model.model.fixed.nodeGroups = [ "fixed", "fixed", "fixed" ];
model.model.fixed.dofs = model.model.rodMesh.child.dofNamesTrans;
model.model.fixed.factors = [ 0., 0., 0. ]; 

model.model.disp.type = "None";

Output.disp.saveWhen = "t % 1e-4 < deltaTime";
Output.modules += "paraview";
Output.paraview.type = "ParaView";
Output.paraview.output_format = "$(CASE_NAME)/visual/step%i";
Output.paraview.groups = [ "beams" ];
Output.paraview.beams.shape = "Line2";
Output.paraview.beams.disps = model.model.rodMesh.child.dofNamesTrans;
Output.paraview.beams.otherDofs = model.model.rodMesh.child.dofNamesRot;
Output.paraview.beams.node_data = ["fint", "fext", "fres"];
Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain", "plast_strain"];
Output.paraview.sampleWhen = "t % 0.01 < deltaTime";

log.pattern = "*";
log.file = "$(CASE_NAME)/run.log";
