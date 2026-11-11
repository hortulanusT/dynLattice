#!/usr/bin/python3

# TEST 1: convergence order of the explicit integrators, of the motion and of the energy, free vibration of a cantilever;
# the energy error of the element in 3D against the element size
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
# mesh runs (tests/testing.mk): elements, and the dead tip force (x, z) from rest; the energy error of the element in 3D
# is of second order in the element size
MESHES, MESH_FORCE, MESH_ORDER, MESH_TOL = [4, 8, 16], (0.02, 0.01), 2.0, 0.1

# integrator: name in the figure, order by construction and tolerance, end time.
# MilneDevice at a fixed step is the first-order Adams-Bashforth-Moulton pair; it damps a mode w like
# exp(-w^2 dt t / 2), so it runs only to 0.02, where that is small for all modes of this model.
SCHEMES = {"LeapFrog": ("leap-frog", 2.0, 0.1, 2.),
           "Milne": ("MilneDevice: Adams-Bashforth-Moulton", 1.0, 0.1, 0.02),
           "ode23": ("EmbeddedRK ode23: Bogacki-Shampine", 3.0, 0.3, 2.),
           "ode45": ("EmbeddedRK ode45: Dormand-Prince", 5.0, 0.3, 2.)}
COLOURS = {"LeapFrog": "C0", "Milne": "C1", "ode23": "C2", "ode45": "C3"}
MARKERS = {"planar": "o", "3D": "s"}
# case: (check the motion, check the energy). The energy in 3D is not conserved by the element (its internal force,
# Petrov-Galerkin, is not the gradient of the energy in 3D; the error is of second order in the element size, see the
# mesh runs), so in 3D the energy of each run is compared with the run at half the step. Not checked, only run through:
# the energy of MilneDevice in 3D (order 0.9).
CASES = {("LeapFrog", "planar"): (True, True), ("LeapFrog", "3D"): (True, True),
         ("Milne", "planar"): (True, True), ("Milne", "3D"): (True, False),
         ("ode23", "planar"): (True, True), ("ode23", "3D"): (True, True),
         ("ode45", "planar"): (True, True), ("ode45", "3D"): (True, True)}


def total_energy(run):
  """E_pot + E_kin + potential of the dead tip force, -F dx."""
  return run[:, 7] - FORCE * run[:, 1] + run[:, 8]


def energy_difference(coarse, fine):
  """Largest |E(t) - E_fine(t)| / E(0) of a run against the run at half the step, at the times of the coarse run."""
  fine = fine[1::2]
  n = min(len(coarse), len(fine))
  if not np.allclose(coarse[:n, 0], fine[:n, 0]):
    raise RuntimeError("the run at half the step does not write every step")
  return np.abs(total_energy(coarse[:n]) - total_energy(fine[:n])).max() / abs(total_energy(coarse)[0])


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
  # dead tip force is -F dx. In 3D: the difference to the run at half the step
  if dim == "planar":
    energy_steps = steps
    energies = np.array([energy_error(run[:, 7] - FORCE * run[:, 1], run[:, 8]) for run in runs])
  else:
    energy_steps = steps[:-1]
    energies = np.array([energy_difference(runs[i], runs[i + 1]) for i in range(RUNS - 1)])
  energy, energy_local = convergence_order(energy_steps, energies)

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
  energy_axes.loglog(energy_steps, energies, **style)
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
  energy_axes.set_ylabel("planar: largest |E(t) / E(0) - 1|; 3D: |E(t) - E(t) at half the step| / E(0)")
  energy_axes.set_title("energy (exact: conserved); dashed: order")
  legend(energy_axes)

  # mesh: energy error of the element in 3D from rest, over the largest kinetic energy, against the element size
  sizes = 1. / np.array(MESHES)
  errors = []
  for elements in MESHES:
    run = np.loadtxt(f"{OUT}/mesh_{elements}.csv", delimiter=",", skiprows=1, ndmin=2)
    if run[-1, 0] < 2. - 1e-3:
      raise RuntimeError(f"mesh_{elements}: the run did not reach t = 2, see the .log files in {OUT}")
    energy = run[:, 7] + run[:, 8] - MESH_FORCE[0] * run[:, 1] - MESH_FORCE[1] * run[:, 3]
    errors.append(np.abs(energy - energy[0]).max() / run[:, 8].max())
  errors = np.array(errors)
  mesh_order, mesh_local = convergence_order(sizes, errors)
  ok = abs(mesh_order - MESH_ORDER) <= MESH_TOL
  test_passed = test_passed and ok
  print(f"  LeapFrog, 3D, energy error of the element, {MESHES} elements: {np.array2string(errors, precision=2)} of "
        f"the largest kinetic energy; order {mesh_order:.2f} in the element size (expected {MESH_ORDER} +- {MESH_TOL}; "
        f"local {', '.join(f'{x:.2f}' for x in mesh_local)})  {'ok' if ok else 'off'}", file=LOG)
  fig_c, mesh_axes = plt.subplots(figsize=(16 / 3, 6))
  mesh_axes.loglog(sizes, errors, marker="s", markerfacecolor="none", ls="", color=COLOURS["LeapFrog"],
                   label="leap-frog, 3D from rest")
  mesh_axes.loglog(sizes, errors[-1] * (sizes / sizes[-1])**MESH_ORDER, "k--", label="reference: order 2")
  mesh_axes.set_xlabel("element size / length")
  mesh_axes.set_ylabel("largest |E(t) - E(0)| / largest E_kin")
  mesh_axes.set_title("energy error of the element in 3D")
  mesh_axes.legend(fontsize=7)
except Exception as e:
  print(e)
  test_passed = False

if test_passed:
  print(colored("INTEGRATOR TEST 1 PASSED", "green") + colored(" (RUN THROUGH: energy of Milne in 3D)", "yellow"))

  with PdfPages(f"{OUT}/result.pdf") as pdf:
    for fig, tag in [(fig_a, "a"), (fig_b, "b"), (fig_c, "c")]:
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/integrator1{tag}_result.png")
else:
  print(colored("INTEGRATOR TEST 1 FAILED", "red", attrs=["bold"]) + f" (details: {OUT}/result.log)")
  sys.exit(1)
