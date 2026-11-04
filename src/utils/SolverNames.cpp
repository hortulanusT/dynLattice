/**
 * @file SolverNames.cpp
 * @author Frans van der Meer
 * @brief file containing the names of the solver parameters and actions
 *
 */
#include "utils/SolverNames.h"

// params
const char *SolverNames::ACCEPT = "Accept";
const char *SolverNames::DISCARD = "Discard";
const char *SolverNames::N_CONTINUES = "NContinues";
const char *SolverNames::STEP_SIZE = "StepSize";
const char *SolverNames::STEP_SIZE_0 = "StepSize0";
const char *SolverNames::TERMINATE = "Terminate";
const char *SolverNames::ROT_DOFS = "RotDofs";
const char *SolverNames::ROT_SHIFT = "RotShift";

// actions
const char *SolverNames::CHECK_COMMIT = "CheckCommit";
const char *SolverNames::SET_STEP_SIZE = "SetStepSize";
const char *SolverNames::CONTINUE = "Continue";
const char *SolverNames::GET_ROT_DOFS = "GetRotDofs";
