///////////////////////////////////
/////// CONVERGENCE ORDER /////////
///////////////////////////////////

// Vibration of a cantilever with an initial tip velocity and a dead tip force, fixed time step.
// tests/testing.mk runs it for each integrator and step size.

// PROGRAM_CONTROL
control.runWhile = "t < 1. - 5e-5";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "LeapFrog";
Solver.integrator.deltaTime = 1e-4;
Solver.integrator.minDTime = 1e-4;
Solver.integrator.maxDTime = 1e-4;
Solver.integrator.dofs_SO3 = [ "rx", "ry", "rz" ];

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.cross_section = "circle";
params.rod_details.material.radius = 0.05;
params.rod_details.material.young = 1e4;
params.rod_details.material.poisson_ratio = 0.3;
params.rod_details.material.density = 1.;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

// dead tip force from t = 0: the first acceleration is not zero, which the start of an integrator has to get right
model.model.force.initLoad = 0.02;
model.model.force.factors = [ 1. ];

model.model.init.veloGroups = [ "free", "free" ];
model.model.init.veloDofs = [ "dx", "dz" ];
model.model.init.veloVals = [ 1., 0.5 ];
