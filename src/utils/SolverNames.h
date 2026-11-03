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
  static const char *ROT_DOFS; ///< names of the rotation dofs (rotation vectors), see GET_ROT_DOFS

  // actions
  static const char *CHECK_COMMIT;
  static const char *SET_STEP_SIZE;
  static const char *CONTINUE;
  static const char *GET_ROT_DOFS; ///< models with rotation dofs (rods) report their names in ROT_DOFS
};
