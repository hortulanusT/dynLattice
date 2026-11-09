#!/usr/bin/python3

# TEST 1: convergence order of the explicit integrators, of the motion and of the energy, free vibration of a cantilever
import sys
from pathlib import Path

import numpy as np
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.lines import Line2D
from termcolor import colored

sys.path.insert(0, str(Path(__file__).parent.parent))
from metrics import convergence_order, energy_error

OUT = "tests/integrator/test1"
FORCE = 0.02       # dead tip force in x, its potential is -F dx
RUNS = 5           # step sizes per case, each half the previous one (tests/testing.mk)

# integrator: name in the figure, order by construction and tolerance, end time.
# MilneDevice at a fixed step is the first-order Adams-Bashforth-Moulton pair; it damps a mode w like
# exp(-w^2 dt t / 2), so it runs only to 0.02, where that is small for all modes of this model.
SCHEMES = {"LeapFrog": ("leap-frog", 2.0, 0.1, 2.),
           "Milne": ("MilneDevice: Adams-Bashforth-Moulton", 1.0, 0.1, 0.02),
           "ode23": ("EmbeddedRK ode23: Bogacki-Shampine", 3.0, 0.3, 2.),
           "ode45": ("EmbeddedRK ode45: Dormand-Prince", 5.0, 0.3, 2.)}
COLOURS = {"LeapFrog": "C0", "Milne": "C1", "ode23": "C2", "ode45": "C3"}
MARKERS = {"planar": "o", "3D": "s"}
# case: (check the motion, check the energy). Not checked, only run through: EmbeddedRK in 3D, which is first order
# as its mass matrix stays frozen during the stages; the energy in 3D, whose error stops at about 2e-4 for every
# integrator, whatever the step.
CASES = {("LeapFrog", "planar"): (True, True), ("LeapFrog", "3D"): (True, False),
         ("Milne", "planar"): (True, True), ("Milne", "3D"): (True, False),
         ("ode23", "planar"): (True, True), ("ode23", "3D"): (False, False),
         ("ode45", "planar"): (True, True), ("ode45", "3D"): (False, False)}


def evaluate(scheme, dim, motion_axes, energy_axes):
  label, order, tol, end = SCHEMES[scheme]
  check_motion, check_energy = CASES[(scheme, dim)]
  runs = [np.loadtxt(f"{OUT}/{scheme}_{dim}_{i}.csv", delimiter=",", skiprows=1, ndmin=2) for i in range(RUNS)]
  steps = np.array([run[1, 0] - run[0, 0] for run in runs])
  if any(run[-1, 0] < end - step for run, step in zip(runs, steps)):
    raise RuntimeError(f"{scheme} {dim}: a run did not reach t = {end}, see the .log files in {OUT}")

  # motion: self-convergence of the tip at the end time, the difference to the run at half the step
  ends = [run[-1, 1:7] for run in runs]
  diffs = np.array([np.linalg.norm(ends[i] - ends[i + 1]) for i in range(RUNS - 1)])
  motion, motion_local = convergence_order(steps[:-1], diffs)
  # energy: the exact solution keeps it, so the error must vanish with the order of the scheme; the potential of the
  # dead tip force is -F dx; LeapFrog stores the velocity half a step before the displacement
  energies = np.array([energy_error(run[:, 7] - FORCE * run[:, 1], run[:, 8], half_step=(scheme == "LeapFrog"))
                       for run in runs])
  energy, energy_local = convergence_order(steps, energies)

  results = []
  for what, measured, local, checked in [("motion", motion, motion_local, check_motion),
                                         ("energy", energy, energy_local, check_energy)]:
    ok = abs(measured - order) <= tol
    status = ("ok" if ok else "off") if checked else "RUN THROUGH"
    print(f"  {scheme}, {dim}, {what}: order {measured:.2f} (expected {order} +- {tol}; local "
          f"{', '.join(f'{x:.2f}' for x in local)})  {status}", file=LOG)
    results.append(ok or not checked)

  style = dict(color=COLOURS[scheme], marker=MARKERS[dim], markerfacecolor="none", ls="")
  motion_axes.loglog(steps[:-1], diffs, **style)
  energy_axes.loglog(steps, energies, **style)
  if dim == "planar":
    # reference: the order of the scheme, through the finest planar point
    motion_axes.loglog(steps[:-1], diffs[-1] * (steps[:-1] / steps[-2])**order, "--", color=COLOURS[scheme])
    energy_axes.loglog(steps, energies[-1] * (steps / steps[-1])**order, "--", color=COLOURS[scheme])
  return all(results)


def legend(axes):
  handles = [Line2D([], [], color=COLOURS[s], label=SCHEMES[s][0]) for s in SCHEMES]
  handles += [Line2D([], [], color="k", marker=m, markerfacecolor="none", ls="", label=d) for d, m in MARKERS.items()]
  handles += [Line2D([], [], color="k", ls="--", label="reference: order 2, 1, 3, 5")]
  axes.legend(handles=handles, fontsize=7)


# details of the checks; on screen only PASSED, FAILED or RUN THROUGH
LOG = open(f"{OUT}/result.log", "w")

test_passed = True

try:
  fig_a, motion_axes = plt.subplots(figsize=(16 / 3, 6))
  fig_b, energy_axes = plt.subplots(figsize=(16 / 3, 6))
  for scheme, dim in CASES:
    test_passed = evaluate(scheme, dim, motion_axes, energy_axes) and test_passed

  motion_axes.set_xlabel("time step")
  motion_axes.set_ylabel("tip at the end: difference to the run at half the step")
  motion_axes.set_title("motion; dashed: order of each scheme")
  legend(motion_axes)
  energy_axes.set_xlabel("time step")
  energy_axes.set_ylabel("energy error, largest |E(t) / E(0) - 1|")
  energy_axes.set_title("energy (exact: conserved); dashed: order")
  legend(energy_axes)
except Exception as e:
  print(e)
  test_passed = False

if test_passed:
  print(colored("INTEGRATOR TEST 1 PASSED", "green")
        + colored(" (RUN THROUGH: EmbeddedRK in 3D, energy in 3D)", "yellow"))

  with PdfPages(f"{OUT}/result.pdf") as pdf:
    for fig, tag in [(fig_a, "a"), (fig_b, "b")]:
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/integrator1{tag}_result.png")
else:
  print(colored("INTEGRATOR TEST 1 FAILED", "red", attrs=["bold"]) + f" (details: {OUT}/result.log)")
  sys.exit(1)
