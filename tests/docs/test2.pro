// PROGRAM_CONTROL
control.runWhile='ymax.disp.dy/SIZE.Y>-0.5 && ymax.velo.dy <= 0.';

// SOLVER
Solver.modules = [ "integrator" ];
Solver.integrator.type = "MilneDevice";
Solver.integrator.deltaTime = 2e-9;
Solver.integrator.dofs_SO3 = ["rx", "ry", "rz"];
Solver.integrator.lengthScale = 3.7500000000000003e-05;
Solver.integrator.precision = .3e-4;

// include model and i/o files
include "input.pro";
include "model.pro";
include "output.pro";
