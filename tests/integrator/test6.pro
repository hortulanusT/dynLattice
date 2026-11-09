///////////////////////////////////
/////// FLYING SPAGHETTI //////////
///////////////////////////////////

// Free rod under a force and a moment at its lower end, then free flight (Simo, Vu-Quoc 1988; Simo, Tarnow, Doblare 1995,
// Fig. 2). Parameters of Kinon, Eugster, Betsch (2026), Table 1. tests/testing.mk runs it for each integrator.

// PROGRAM_CONTROL
control.runWhile = "t < 15.";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "LeapFrog";
Solver.integrator.deltaTime = 1e-3;
Solver.integrator.minDTime = 1e-3;
Solver.integrator.maxDTime = 1e-3;
Solver.integrator.dofs_SO3 = [ "rx", "ry", "rz" ];

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.C_11 = 1e4;
params.rod_details.material.C_22 = 1e4;
params.rod_details.material.C_33 = 1e4;
params.rod_details.material.C_44 = 1e3;
params.rod_details.material.C_55 = 1e3;
params.rod_details.material.C_66 = 1e3;
// rho A = 1 and M_rho^11 = M_rho^22 = 10, so that J_rho = diag(10, 10, 20) (Kinon et al., Appendix: J = M^aa I - M^ab e_a e_b)
params.rod_details.material.density = 1.;
params.rod_details.material.area = 1.;
params.rod_details.material.area_moment = [ 10., 10. ];
params.rod_details.material.polar_moment = 20.;

// INPUT
Input.modules = [ "input", "groupInput" ];
Input.input.type = "GMSHInput";
Input.input.file = "$(CASE_NAME).geo";
Input.groupInput.type = "GroupInput";
Input.groupInput.nodeGroups = [ "fixed", "free" ];
Input.groupInput.fixed.ztype = "min";
Input.groupInput.free.ztype = "max";

// include model and i/o files
include "model.pro";
include "output.pro";

model.model.models = [ "rodMesh", "force" ];
model.model.force.type = "LoadScale";
model.model.force.scaleFunc = "if (t <= 2.5, 80. * t, if (t <= 5., 400. - 80. * t, 0.))";
model.model.force.model.type = "Neumann";
// at the lower end, (6, 0, 0): the node group 'fixed' of the shared inputs, nothing holds it here
model.model.force.model.nodeGroups = [ "fixed", "fixed", "fixed" ];
model.model.force.model.dofs = [ "dx", "ry", "rz" ];
model.model.force.model.factors = [ 0.1, 1., 0.5 ];

// all nodes, for the momenta
Output.modules += [ "state" ];
Output.state.type = "CSVOutput";
Output.state.file = "$(CASE_NAME)/state.gz";
Output.state.vectors = [ "state = disp", "state1 = velo" ];
Output.state.saveWhen = "t % 0.1 < deltaTime";

// the loaded end, s = 0, for the order (Kinon et al., Eq. 76)
Output.sampling.header = "t,dx,dy,dz,rx,ry,rz,E_pot,E_kin,x0,y0,z0,vx0,vy0,vz0";
Output.sampling.dataSets += [ "fixed.disp.dx", "fixed.disp.dy", "fixed.disp.dz", "fixed.velo.dx", "fixed.velo.dy", "fixed.velo.dz" ];
