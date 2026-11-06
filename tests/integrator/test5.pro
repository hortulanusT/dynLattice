///////////////////////////////////
/////// PURE ROTATION PAST 2 PI ///
///////////////////////////////////

// Holzinger, Gerstmayr (2021), Sect. 7.1.2 with epsilon = 0: a free rigid body with J = diag(5.2988, 1.1775, 4.3568),
// at rest in the initial orientation, turning with w0 = (0, pi, 0) about its axis 2, so that the rotation vector passes
// |theta| = 2 pi, 4 pi, ... A stiff rod of one element along y is the body: axis 2 is the rod axis, with
// J_x = rho J_x L, J_y = rho J_p L, J_z = rho J_z L; the area is negligible.
// tests/testing.mk runs it for each integrator.

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
// area_moment = [ J_z, J_x ], polar_moment = J_y, for material_ey = x and the rod along y
params.rod_details.material.area_moment = [ 4.3568, 5.2988 ];
params.rod_details.material.polar_moment = 1.1775;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";

// w0 x (0, +-0.5, 0) = 0: only the rotation dofs move
model.model.models = [ "rodMesh", "init" ];
model.model.init.veloGroups = [ "free", "fixed" ];
model.model.init.veloDofs = [ "ry", "ry" ];
model.model.init.veloVals = [ 3.141592653589793, 3.141592653589793 ];
