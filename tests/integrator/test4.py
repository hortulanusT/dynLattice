#!/usr/bin/python3

# TEST 4: freely spinning body of Krysl, Endres (2005), Sect. 4.1, against their Fig. 3
import sys
from pathlib import Path

import numpy as np
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from scipy.spatial.transform import Rotation
from termcolor import colored

sys.path.insert(0, str(Path(__file__).parent.parent))
from metrics import convergence_order, interp_on_reference, relative_L2

OUT = "tests/integrator/test4"
END = 100.
# body-frame angular velocity of Krysl, Endres (2005), Fig. 3 (their finest step), read from the vector paths
REFERENCE = "tests/integrator/ref_data/test4_ref_omega.csv"
OMEGA_TOL = 0.01      # relative L2 of each component, scaled by the largest one (Omega_2), as in plastic test 3
# run: checked
RUNS = {"LeapFrog_0": True, "Milne": True, "ode23": True, "ode45": True}
# LeapFrog at five steps, each half the previous one: order 2 expected.
# The runs write their output every 0.01 (tests/testing.mk), so the steps are given here.
ORDER_STEPS, ORDER, ORDER_TOL = 1e-3 * 0.5**np.arange(5), 2., 0.1


def read(name):
  rows = np.loadtxt(f"{OUT}/{name}.csv", delimiter=",", skiprows=1, ndmin=2)
  if rows[-1, 0] < END - 1e-6:
    raise RuntimeError(f"{name}: the run did not reach t = {END}, see the .log files in {OUT}")
  return rows


def body_angular_velocity(rows):
  """Omega = R^T w: the angular velocity of the free end in the frame of the body (the global frame at t = 0)."""
  return Rotation.from_rotvec(rows[:, 4:7]).inv().apply(rows[:, 9:12])


# details of the checks; on screen only PASSED, FAILED or RUN THROUGH
LOG = open(f"{OUT}/result.log", "w")

test_passed = True

try:
  data = np.genfromtxt(REFERENCE, delimiter=",", skip_header=2)
  reference = [data[np.isfinite(data[:, 2 * i]), 2 * i:2 * i + 2] for i in range(3)]
  dominant = max([curve[:, 1] for curve in reference], key=np.linalg.norm)

  fig_a, omega_axes = plt.subplots(figsize=(16 / 3, 6))
  for run, checked in RUNS.items():
    rows = read(run)
    omega = body_angular_velocity(rows)
    errors = [relative_L2(interp_on_reference(rows[:, 0], omega[:, i], curve[:, 0]), curve[:, 1], scale=dominant)
              for i, curve in enumerate(reference)]
    ok = max(errors) <= OMEGA_TOL
    test_passed = test_passed and (ok or not checked)
    status = ("ok" if ok else "off") if checked else "RUN THROUGH"
    print(f"  {run}: Omega against Fig. 3, relative L2 {np.round(errors, 4)} "
          f"(limit {OMEGA_TOL})  {status}", file=LOG)
    if run == "LeapFrog_0":
      leapfrog = rows
      for i in range(3):
        omega_axes.plot(rows[:, 0], omega[:, i], color=f"C{i}", label=f"Omega_{i + 1}, leap-frog")
  for i, curve in enumerate(reference):
    label = "reference: Krysl, Endres (2005), Fig. 3" if i == 0 else None
    omega_axes.plot(curve[:, 0], curve[:, 1], "k--", lw=1., label=label)
  omega_axes.set_xlabel("t")
  omega_axes.set_ylabel("angular velocity, body frame")
  omega_axes.set_title("body angular velocity; dashed: Krysl, Endres")
  omega_axes.legend(fontsize=7, loc="lower right")

  # convergence of LeapFrog: the rotation at the end against the run at half the step
  runs = [leapfrog] + [read(f"LeapFrog_{i}") for i in range(1, len(ORDER_STEPS))]
  ends = [Rotation.from_rotvec(r[-1, 4:7]) for r in runs]
  diffs = np.array([(ends[i].inv() * ends[i + 1]).magnitude() for i in range(len(runs) - 1)])
  steps = ORDER_STEPS[:-1]
  measured, local = convergence_order(steps, diffs)
  ok = abs(measured - ORDER) <= ORDER_TOL
  test_passed = test_passed and ok
  print(f"  LeapFrog, rotation: order {measured:.2f} (expected {ORDER} +- {ORDER_TOL}; local {np.round(local, 2)})  "
        f"{'ok' if ok else 'off'}", file=LOG)
  fig_b, order_axes = plt.subplots(figsize=(16 / 3, 6))
  order_axes.loglog(steps, diffs, marker="o", markerfacecolor="none", ls="", color="C0",
                    label="rotation at t = 100: difference to the run at half the step [rad]")
  order_axes.loglog(steps, diffs[-1] * (steps / steps[-1])**ORDER, "k--", label="reference: order 2")
  order_axes.legend(fontsize=7)
  order_axes.set_xlabel("time step")
  order_axes.set_ylabel("error, leap-frog")
  order_axes.set_title("convergence of leap-frog")
except Exception as e:
  print(e)
  test_passed = False

if test_passed:
  print(colored("INTEGRATOR TEST 4 PASSED", "green"))

  with PdfPages(f"{OUT}/result.pdf") as pdf:
    for fig, tag in [(fig_a, "a"), (fig_b, "b")]:
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/integrator4{tag}_result.png")
else:
  print(colored("INTEGRATOR TEST 4 FAILED", "red", attrs=["bold"]) + f" (details: {OUT}/result.log)")
  sys.exit(1)
