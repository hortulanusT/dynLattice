///////////////////////////////////
/////// AXIAL WAVE (D'ALEMBERT) ///
///////////////////////////////////

// Bar fixed at one end with a constant axial force at the other, from rest. Wave speed c = sqrt(E/rho) = 1.
// tests/testing.mk runs it for each integrator.

// PROGRAM_CONTROL
control.runWhile = "t < 12.";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "LeapFrog";
Solver.integrator.deltaTime = 1e-3;
Solver.integrator.minDTime = 1e-3;
Solver.integrator.maxDTime = 1e-3;
Solver.integrator.dofs_SO3 = [ "rx", "ry", "rz" ];
Solver.integrator.updateWhen = false;

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.cross_section = "circle";
params.rod_details.material.radius = 0.01;
params.rod_details.material.young = 1.;
params.rod_details.material.poisson_ratio = 0.;
params.rod_details.material.density = 1.;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.model.models = [ "rodMesh", "fixed", "force" ];
// a small force: F / (E A) = 1e-3 strain
model.model.force.initLoad = "1e-3 * PI * 0.01^2";
model.model.force.dofs = [ "dy" ];
model.model.force.factors = [ 1. ];

Output.sampling.header = "t,dx,dy,dz,rx,ry,rz,E_pot,E_kin,reaction";
Output.sampling.dataSets += [ "fixed.resp.dy" ];
