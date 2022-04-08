// LOGGING
log.pattern = "*.info | *.debug";
log.file = "$(CASE_NAME).log";

// PROGRAM_CONTROL
control.runWhile = false;

// SOLVER
Solver.modules = [ "solver" ];
Solver.solver.type = "Nonlin";
Solver.solver.precision = 1e-6;

// include model and i/o files
params.Incr = 10.;
include "input.pro";
include "model.pro";
include "output.pro";