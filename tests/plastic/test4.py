#!/usr/bin/python3

# TEST 4: cyclic bending of a section, against the exact solution of the stress resultant model;
# the fibre integral of J2 plasticity with linear hardening is reported only
import sys

import numpy as np
from matplotlib import pyplot as plt
from termcolor import colored

KAPPA, MU = 164.210e9, 80.193e9
YOUNG = 9. * KAPPA * MU / (3. * KAPPA + MU)
YIELD, HARDENING = 450e6, 20e9   # Herrnboeck et al. (2021), (2023)
RADIUS, LENGTH = 1e-3, 0.01
YIELD_MOMENT = 0.62              # yield surface of test 3, bending
HARDENING_MOMENT = 16015e-6      # hardening tensor of test 3, bending entry (Herrnboeck et al. 2023, Eq. 44)

ENERGY_TOL = 0.01      # plastic work and stored energy against the loop area
EXACT_TOL = 0.01       # whole cycle against the exact solution of the resultant model
YIELD_TOL = 0.05       # moment where the response leaves the elastic line


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


def fibre_moments(curvatures):
  """Bending moment of a circular section: uniaxial J2 fibres with linear isotropic hardening, along the history."""
  points, weights = np.polynomial.legendre.leggauss(64)
  y = RADIUS * points
  width = 2. * np.sqrt(RADIUS**2 - y**2) * RADIUS * weights
  plastic, alpha = np.zeros_like(y), np.zeros_like(y)
  moments = []
  for curvature in curvatures:
    strain = curvature * y
    trial = YOUNG * (strain - plastic)
    excess = np.abs(trial) - (YIELD + HARDENING * alpha)
    increment = np.maximum(excess, 0.) / (YOUNG + HARDENING)
    plastic = plastic + increment * np.sign(trial)
    alpha = alpha + increment
    stress = YOUNG * (strain - plastic)
    moments.append(np.sum(stress * y * width))
  return np.array(moments)


# details of the checks; on screen only PASSED, FAILED or RUN THROUGH
LOG = open("tests/plastic/test4/result.log", "w")

test_passed = False

try:
  disp = np.loadtxt("tests/plastic/test4/disp.csv", delimiter=",")
  resp = np.loadtxt("tests/plastic/test4/resp.csv", delimiter=",")
  energy = np.loadtxt("tests/plastic/test4/energy.csv", delimiter=",")

  rotation, moment = disp[:, 5], resp[:, 5]
  # sign of the resultant: the same as the rotation in the first, elastic steps
  moment = moment * np.sign(moment[0] * rotation[0])
  curvature = rotation / LENGTH
  stiffness = moment[0] / rotation[0] * LENGTH
  exact = exact_moments(curvature, stiffness)
  fibre = fibre_moments(curvature)

  # the phases of the input: first loading, reversal to the lowest curvature, reloading
  steps = (len(moment) - 1) // 5
  phases = {"end of the first loading": steps - 1, "after reversal, at the lowest curvature": 3 * steps - 1,
            "after reloading": len(moment) - 1}
  # along the whole cycle, against the largest moment (on the elastic branches near zero the ratio is meaningless)
  exact_error = np.max(np.abs(moment - exact)) / np.max(np.abs(exact))

  # work of the moment against the stored energy and the dissipation
  work = np.concatenate([[0.], np.cumsum(0.5 * (moment[1:] + moment[:-1]) * np.diff(rotation))])
  balance = np.abs(work[1:] - (energy[1:, 0] + energy[1:, 1])).max() / np.abs(work).max()

  # moment where the first loading leaves the elastic line by 1 %
  first = phases["end of the first loading"]
  leaves = int(np.argmax(np.abs(moment[:first]) < 0.99 * np.abs(stiffness * curvature[:first])))
  yield_error = abs(abs(moment[leaves]) / YIELD_MOMENT - 1.)

  for name, i in phases.items():
    print(f"  {name}: moment {moment[i]:.4f}, exact {exact[i]:.4f} ({(moment[i] / exact[i] - 1.) * 100:+.2f} %), "
          f"fibre J2 {fibre[i]:.4f} (exact against fibre {(exact[i] / fibre[i] - 1.) * 100:+.2f} %, reported only)", file=LOG)
  print(f"  whole cycle against the exact solution: largest difference over the largest moment {exact_error * 100:.2f} % (limit {EXACT_TOL * 100:.0f} %)", file=LOG)
  print(f"  leaves the elastic line at {abs(moment[leaves]):.3f} N m (table {YIELD_MOMENT}, limit {YIELD_TOL * 100:.0f} %)", file=LOG)
  print(f"  work against stored energy + dissipation: {balance * 100:.3f} % (limit {ENERGY_TOL * 100:.0f} %)", file=LOG)

  plt.figure(figsize=(16 / 3, 6))
  plt.plot(curvature, moment, label="simulation")
  plt.plot(curvature, exact, "k--", label="reference: exact solution of the stress resultant model\n"
           "(yield surface and hardening: Herrnböck et al. 2023, Eqs. 39, 44)")
  plt.plot(curvature, fibre, ":", label="for comparison, not judged: section integral of J2 fibres\n"
           "(sigma_y = 450 MPa, H = 20 GPa, micro model of Herrnböck et al. 2021)")
  plt.xlabel("curvature [1/m]")
  plt.ylabel("bending moment [N m]")
  plt.title("cyclic bending; dashed: exact resultant model")
  plt.legend(fontsize=7)

  test_passed = len(moment) >= 5000 and exact_error <= EXACT_TOL and balance <= ENERGY_TOL and yield_error <= YIELD_TOL
except Exception as e:
  print(e)

if test_passed:
  print(colored("PLASTIC TEST 4 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/plastic/test4/result.pdf")
  plt.savefig("tests/plastic4_result.png")
else:
  print(colored("PLASTIC TEST 4 FAILED", "red", attrs=["bold"]) + " (details: tests/plastic/test4/result.log)")
  sys.exit(1)
