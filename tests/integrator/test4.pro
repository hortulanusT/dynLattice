///////////////////////////////////
/////// FREELY SPINNING BODY //////
///////////////////////////////////

// Krysl, Endres (2005), Sect. 4.1: a free rigid body with diag I = (0.9144, 1.098, 1.66) and the initial body angular
// velocity (0.45549, 0.82623, 0.03476), close to the intermediate axis. A stiff rod of one element along y is the body:
// I_x = rho J_x L, I_y = rho J_p L, I_z = rho J_z L; the area is negligible, so that the inertia of the distributed mass,
// A L^3 / 12, plays no role. The end nodes start with the velocities -+ w x (0, 0.5, 0).
// tests/testing.mk runs it for each integrator and step size.

// PROGRAM_CONTROL
control.runWhile = "t < 100.";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "LeapFrog";
Solver.integrator.deltaTime = 1e-3;
Solver.integrator.minDTime = 1e-3;
Solver.integrator.maxDTime = 1e-3;
Solver.integrator.dofs_SO3 = [ "rx", "ry", "rz" ];

// SETTINGS
params.rod_details.material_ey = [ 1., 0., 0. ];
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.young = 2e4;
params.rod_details.material.shear_modulus = 1e4;
params.rod_details.material.density = 1.;
params.rod_details.material.area = 1e-6;
// area_moment = [ I_z, I_x ], polar_moment = I_y, for material_ey = x and the rod along y
params.rod_details.material.area_moment = [ 1.66, 0.9144 ];
params.rod_details.material.polar_moment = 1.098;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.model.models = [ "rodMesh", "init" ];
model.model.init.veloGroups = [ "free", "free", "free", "free", "free", "free", "fixed", "fixed", "fixed", "fixed", "fixed", "fixed" ];
model.model.init.veloDofs = [ "dx", "dy", "dz", "rx", "ry", "rz", "dx", "dy", "dz", "rx", "ry", "rz" ];
model.model.init.veloVals = [ -0.01738, 0., 0.227745, 0.45549, 0.82623, 0.03476, 0.01738, 0., -0.227745, 0.45549, 0.82623, 0.03476 ];

// the angular velocity of the free end, for the body frame
Output.sampling.header = "t,dx,dy,dz,rx,ry,rz,E_pot,E_kin,wx,wy,wz";
Output.sampling.dataSets += [ "free.velo.rx", "free.velo.ry", "free.velo.rz" ];
