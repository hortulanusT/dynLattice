/**
 * @file SolverNames.h
 * @author Frans van der Meer
 * @brief file containing the names of the solver parameters and actions
 *
 */
#pragma once

struct SolverNames
{
  // params
  static const char *ACCEPT;
  static const char *DISCARD;
  static const char *N_CONTINUES;
  static const char *STEP_SIZE;
  static const char *STEP_SIZE_0;
  static const char *TERMINATE;

  // actions
  static const char *CHECK_COMMIT;
  static const char *SET_STEP_SIZE;
  static const char *CONTINUE;
};
