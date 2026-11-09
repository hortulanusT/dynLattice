///////////////////////////////////
/////// CANTILEVER FREQUENCIES ////
///////////////////////////////////

// Small amplitude vibration of a cantilever after an initial tip velocity.
// tests/testing.mk runs it for two slenderness ratios and each integrator.

// PROGRAM_CONTROL
control.runWhile = "t < 1.";

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "LeapFrog";
Solver.integrator.deltaTime = 1e-4;
Solver.integrator.minDTime = 1e-4;
Solver.integrator.maxDTime = 1e-4;
Solver.integrator.dofs_SO3 = [ "rx", "ry", "rz" ];
Solver.integrator.updateWhen = false;

// SETTINGS
params.rod_details.material.type = "ElasticRod";
params.rod_details.material.cross_section = "circle";
params.rod_details.material.radius = 0.02;
params.rod_details.material.young = 1e4;
params.rod_details.material.poisson_ratio = 0.3;
params.rod_details.material.density = 1.;
// Cowper (1966), solid circle: 6 (1 + nu) / (7 + 6 nu)
params.rod_details.material.shear_correction = 0.8863;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

model.model.models = [ "rodMesh", "fixed", "init" ];
model.model.init.veloGroups = [ "free" ];
model.model.init.veloDofs = [ "dx" ];
model.model.init.veloVals = [ 1e-3 ];
