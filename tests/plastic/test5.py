#!/usr/bin/python3

# TEST 5: cyclic bending with coarse load steps and the plastic update with sub-increments (substepYield), against the
# exact solution of the stress resultant model; test 4 with 25 instead of 1000 steps per phase
import sys

import numpy as np
from matplotlib import pyplot as plt
from termcolor import colored

LENGTH = 0.01
YIELD_MOMENT = 0.62              # yield surface of test 3, bending
HARDENING_MOMENT = 16015e-6      # hardening tensor of test 3, bending entry (Herrnboeck et al. 2023, Eq. 44)

ENERGY_TOL = 0.01      # plastic work and stored energy against the loop area
EXACT_TOL = 0.01       # whole cycle against the exact solution of the resultant model
STEPS_PER_PHASE = 25   # load steps for each 8 % of strain, as in tests/plastic/test5.pro


def exact_moments(curvatures, stiffness):
  """Bending moment of the stress resultant model with linear hardening along the history: with a single nonzero
  component the yield surface is |m| = m_y + H xi, and the radial return is exact for any step."""
  plastic, alpha = 0., 0.
  moments = []
  for curvature in curvatures:
    moment = stiffness * (curvature - plastic)
    excess = abs(moment) - (YIELD_MOMENT + HARDENING_MOMENT * alpha)
    if excess > 0.:
      increment = excess / (stiffness + HARDENING_MOMENT)
      plastic += increment * np.sign(moment)
      alpha += increment
      moment = stiffness * (curvature - plastic)
    moments.append(moment)
  return np.array(moments)


# details of the checks; on screen only PASSED, FAILED or RUN THROUGH
LOG = open("tests/plastic/test5/result.log", "w")

test_passed = False

try:
  disp = np.loadtxt("tests/plastic/test5/disp.csv", delimiter=",")
  resp = np.loadtxt("tests/plastic/test5/resp.csv", delimiter=",")
  energy = np.loadtxt("tests/plastic/test5/energy.csv", delimiter=",")

  rotation, moment = disp[:, 5], resp[:, 5]
  # sign of the resultant: the same as the rotation in the first, elastic step
  moment = moment * np.sign(moment[0] * rotation[0])
  curvature = rotation / LENGTH
  stiffness = moment[0] / rotation[0] * LENGTH
  exact = exact_moments(curvature, stiffness)

  # the phases of the input: first loading, reversal to the lowest curvature, reloading; one row per step from step 1
  if len(moment) != 5 * STEPS_PER_PHASE + 1:
    raise RuntimeError(f"{len(moment)} rows, expected {5 * STEPS_PER_PHASE + 1}: see tests/plastic/test5.pro")
  first, lowest, last = STEPS_PER_PHASE - 1, 3 * STEPS_PER_PHASE - 1, len(moment) - 1
  # along the whole cycle, against the largest moment (on the elastic branches near zero the ratio is meaningless)
  exact_error = np.max(np.abs(moment - exact)) / np.max(np.abs(exact))

  # work of the moment against the stored energy and the dissipation
  work = np.concatenate([[0.], np.cumsum(0.5 * (moment[1:] + moment[:-1]) * np.diff(rotation))])
  balance = np.abs(work[1:] - (energy[1:, 0] + energy[1:, 1])).max() / np.abs(work).max()

  for name, i in [("end of the first loading", first), ("after reversal, at the lowest curvature", lowest),
                  ("after reloading", last)]:
    print(f"  {name}: moment {moment[i]:.4f}, exact {exact[i]:.4f} ({(moment[i] / exact[i] - 1.) * 100:+.2f} %)", file=LOG)
  print(f"  whole cycle against the exact solution: largest difference over the largest moment "
        f"{exact_error * 100:.2f} % (limit {EXACT_TOL * 100:.0f} %)", file=LOG)
  print(f"  work against stored energy + dissipation: {balance * 100:.3f} % (limit {ENERGY_TOL * 100:.0f} %)", file=LOG)

  plt.figure(figsize=(16 / 3, 6))
  plt.plot(curvature, moment, "o-", ms=3, label="simulation, 25 steps per phase, with sub-increments")
  plt.plot(curvature, exact, "k--", label="reference: exact solution of the stress resultant model")
  plt.xlabel("curvature [1/m]")
  plt.ylabel("bending moment [N m]")
  plt.title("cyclic bending, coarse steps; dashed: exact resultant model")
  plt.legend(fontsize=7)

  test_passed = exact_error <= EXACT_TOL and balance <= ENERGY_TOL
except Exception as e:
  print(e)

if test_passed:
  print(colored("PLASTIC TEST 5 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/plastic/test5/result.pdf")
  plt.savefig("tests/plastic5_result.png")
else:
  print(colored("PLASTIC TEST 5 FAILED", "red", attrs=["bold"]) + " (details: tests/plastic/test5/result.log)")
  sys.exit(1)
