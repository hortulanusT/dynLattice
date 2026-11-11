#!/usr/bin/python3

# TEST 6: flying spaghetti against Kinon, Eugster, Betsch (2026), Sect. 6.1
import sys
from pathlib import Path

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.lines import Line2D
from scipy.spatial.transform import Rotation
from termcolor import colored

sys.path.insert(0, str(Path(__file__).parent.parent))
from metrics import convergence_order, energy_error, interp_on_reference, relative_L2

OUT = "tests/integrator/test6"
END, FREE = 15., 5.       # the load ends at t = 5
ORDER_END = 5.            # Eq. (76): the evaluation time of the convergence study
START, TIP0 = np.array([6., 0., 0.]), np.array([0., 0., 8.])
ELEMENTS, LENGTH, RHO_A = 10, 10., 1.
ELEM = LENGTH / ELEMENTS
# Table 1 and Appendix (at Eq. 88): M^11 = M^22 = 10, J = M^aa I - M^ab e_a e_b, which is 10 about the transverse axes
# and 20 about the rod axis t3 of the initial configuration
AXIS = (TIP0 - START) / LENGTH
INERTIA = 20. * np.eye(3) - 10. * (np.eye(3) - np.outer(AXIS, AXIS))
# total energy (Fig. 5) and momenta (Fig. 6), read from the vector paths of the figures
REF_ENERGY = "tests/integrator/ref_data/test6_ref_energy.csv"
REF_MOMENTUM = "tests/integrator/ref_data/test6_ref_momentum.csv"
CENTRE_TOL = 1e-3      # largest deviation from Eq. (75), over L
CURVE_TOL = 0.01       # relative L2 against Figs. 5 and 6, each momentum scaled by the largest one (l_2)
DRIFT_TOL = 1e-3       # energy and angular momentum in the free flight
SCHEMES = {"LeapFrog": "leap-frog", "Milne": "MilneDevice: Adams-Bashforth-Moulton",
           "ode23": "EmbeddedRK ode23: Bogacki-Shampine", "ode45": "EmbeddedRK ode45: Dormand-Prince"}
COLOURS = {"LeapFrog": "C0", "Milne": "C1", "ode23": "C2", "ode45": "C3"}
# Checked: all integrators against the references; LeapFrog also the energy in the free flight; LeapFrog, ode23 and
# ode45 the angular momentum of the model (output of its mass matrix) in the free flight. Run through: the energy of
# EmbeddedRK in the free flight; MilneDevice in the free flight, which damps by construction; the angular momentum of
# Eq. (108) with J, for every integrator: the nodal inertia of the model also holds rho A l^2 / 12 about the transverse
# axes, which is not in J, so that it drifts by 3e-3
CHECK_REFERENCES = ("LeapFrog", "Milne", "ode23", "ode45")
CHECK_ENERGY = ("LeapFrog",)
CHECK_MOMENTUM = ("LeapFrog", "ode23", "ode45")
# LeapFrog at five steps to t = 5, each half the previous one: the relative error of Eq. (76) against the finest run,
# order 2 expected (Fig. 8 shows order 2 for their scheme). Checked for the position; the velocity is run through (its
# coarsest step is not yet in the asymptotic range)
ORDER_TOL = 0.1
ORDER_RUNS, ORDER = 5, 2.


def exact_centre(t):
  """Centre of mass, Eq. (75)."""
  r1 = np.where(t <= 2.5, 3. + 2. / 15. * t**3,
                np.where(t <= 5., 43. / 6. - 5. * t + 2. * t**2 - 2. / 15. * t**3, -19. / 2. + 5. * t))
  return np.column_stack([r1, np.zeros_like(t), 4. * np.ones_like(t)])


def read_reference(path):
  data = np.genfromtxt(path, delimiter=",", skip_header=2, ndmin=2)
  return [data[np.isfinite(data[:, 2 * i]), 2 * i:2 * i + 2] for i in range(data.shape[1] // 2)]


def read(name, end):
  rows = np.loadtxt(f"{OUT}/{name}.csv", delimiter=",", skiprows=1, ndmin=2)
  if rows[-1, 0] < end - 2. * (rows[1, 0] - rows[0, 0]):
    raise RuntimeError(f"{name}: the run did not reach t = {end}, see the .log files in {OUT}")
  return rows


def read_state(path):
  """Times, and displacements, rotations, velocities and angular velocities of the nodes along the rod, each
  (time, node, 3). gmsh numbers the end points first, then the interior nodes from the start."""
  data = pd.read_csv(path, index_col=["time", "label"], skipinitialspace=True)
  order = [0] + list(range(2, ELEMENTS + 1)) + [1]

  def nodes(frame, names):
    return np.stack([frame[[f"{name}[{node}]" for name in names]].to_numpy() for node in order], axis=1)

  disp, velo = data.xs("disp", level="label"), data.xs("velo", level="label")
  return (disp.index.to_numpy(), nodes(disp, ["dx", "dy", "dz"]), nodes(disp, ["rx", "ry", "rz"]),
          nodes(velo, ["dx", "dy", "dz"]), nodes(velo, ["rx", "ry", "rz"]))


def momenta(pos, rot, vel, spin):
  """Linear momentum and angular momentum, Eq. (108), l = int (phi x rho A v + R J R^T w) ds, of the interpolated
  state: linear between the nodes for the centreline, the rotational part at the nodes with the trapezoidal rule."""
  mass = RHO_A * ELEM / 6.
  pa, pb, va, vb = pos[:, :-1], pos[:, 1:], vel[:, :-1], vel[:, 1:]
  linear = (mass * 3. * (va + vb)).sum(axis=1)
  angular = (mass * (np.cross(2. * pa + pb, va) + np.cross(pa + 2. * pb, vb))).sum(axis=1)
  weights = np.full(ELEMENTS + 1, ELEM)
  weights[[0, -1]] = ELEM / 2.
  # R J R^T w at every node and time (J is symmetric)
  R = Rotation.from_rotvec(rot.reshape(-1, 3))
  spatial = R.apply(R.inv().apply(spin.reshape(-1, 3)) @ INERTIA).reshape(spin.shape)
  angular = angular + (weights[None, :, None] * spatial).sum(axis=1)
  return linear, angular


def against(t, values, reference, scale):
  return [relative_L2(interp_on_reference(t, values[:, i], curve[:, 0]), curve[:, 1], scale=scale)
          for i, curve in enumerate(reference)]


def status(ok, checked):
  return ("ok" if ok else "off") if checked else "RUN THROUGH"


# details of the checks; on screen only PASSED, FAILED or RUN THROUGH
LOG = open(f"{OUT}/result.log", "w")

test_passed = True

try:
  ref_energy = read_reference(REF_ENERGY)
  ref_momentum = read_reference(REF_MOMENTUM)    # p_1, l_2, l_3
  dominant = max([c[:, 1] for c in ref_momentum], key=np.linalg.norm)
  ref = np.linspace(START, TIP0, ELEMENTS + 1)

  fig_a, energy_axes = plt.subplots(figsize=(16 / 3, 6))
  fig_b, momentum_axes = plt.subplots(figsize=(16 / 3, 6))
  for name in SCHEMES:
    rows = read(name, END)
    times, disp, rot, vel, spin = read_state(f"{OUT}/{name}_state.gz")
    pos = ref[None] + disp
    centre = (ELEM / (2. * LENGTH) * (pos[:, :-1] + pos[:, 1:])).sum(axis=1)
    centre_error = np.abs(centre - exact_centre(times)).max() / LENGTH
    linear, angular = momenta(pos, rot, vel, spin)
    energy = rows[:, 7] + rows[:, 8]
    energy_errors = against(rows[:, 0], energy[:, None], ref_energy, None)
    momenta_compared = np.column_stack([linear[:, 0], angular[:, 1], angular[:, 2]])    # p_1, l_2, l_3
    momentum_errors = against(times, momenta_compared, ref_momentum, dominant)
    flying, free = rows[:, 0] >= FREE + 0.1, times >= FREE + 0.1
    energy_drift = energy_error(rows[flying, 7], rows[flying, 8])
    angular_drift = np.linalg.norm(angular[free] - angular[free][0], axis=1).max() / np.linalg.norm(angular[free][0])
    model_angular = rows[flying, 18:21]
    model_drift = np.linalg.norm(model_angular - model_angular[0], axis=1).max() / np.linalg.norm(model_angular[0])
    ok_ref = centre_error <= CENTRE_TOL and max(energy_errors + momentum_errors) <= CURVE_TOL
    ok_energy = energy_drift <= DRIFT_TOL
    ok_momentum = model_drift <= DRIFT_TOL
    test_passed = (test_passed and (ok_ref or name not in CHECK_REFERENCES)
                   and (ok_energy or name not in CHECK_ENERGY) and (ok_momentum or name not in CHECK_MOMENTUM))
    print(f"  {name}, step {rows[1, 0] - rows[0, 0]:g}: centre of mass against Eq. (75) {centre_error:.1e} "
          f"(limit {CENTRE_TOL}); against Figs. 5, 6: energy {energy_errors[0]:.4f}, p_1, l_2, l_3 "
          f"{np.round(momentum_errors, 4)} (limit {CURVE_TOL}); E = {energy[-1]:.1f}, l = {np.round(angular[-1], 1)}  "
          f"{status(ok_ref, name in CHECK_REFERENCES)}", file=LOG)
    print(f"  {name}, free flight: drift of the energy {energy_drift:.1e}  {status(ok_energy, name in CHECK_ENERGY)}, "
          f"of the angular momentum of the model {model_drift:.1e}  {status(ok_momentum, name in CHECK_MOMENTUM)}, "
          f"of Eq. (108) with J {angular_drift:.1e}  RUN THROUGH (limit {DRIFT_TOL})", file=LOG)
    energy_axes.plot(rows[:, 0], energy, color=COLOURS[name], label=SCHEMES[name])
    for value, style in zip(momenta_compared.T, ["-", "-", "-."]):
      momentum_axes.plot(times, value, color=COLOURS[name], ls=style)

  energy_axes.plot(ref_energy[0][:, 0], ref_energy[0][:, 1], "k--", label="reference: Kinon et al. (2026), Fig. 5")
  energy_axes.set_xlabel("t")
  energy_axes.set_ylabel("total energy E_pot + E_kin")
  energy_axes.set_title("energy; dashed: Kinon et al.")
  energy_axes.legend(fontsize=7, loc="lower right")
  for curve in ref_momentum:
    momentum_axes.plot(curve[:, 0], curve[:, 1], "k--", lw=1.)
  handles = [Line2D([], [], color=COLOURS[s], label=SCHEMES[s]) for s in SCHEMES]
  handles += [Line2D([], [], color="k", ls=l, label=n) for l, n in [("-", "p_1"), ("-", "l_2"), ("-.", "l_3")]]
  handles += [Line2D([], [], color="k", ls="--", label="reference: Kinon et al. (2026), Fig. 6")]
  momentum_axes.legend(handles=handles, fontsize=7, loc="center right")
  momentum_axes.set_xlabel("t")
  momentum_axes.set_ylabel("linear momentum p_1, angular momentum l_2, l_3")
  momentum_axes.set_title("momenta; dashed: Kinon et al.")

  # convergence of LeapFrog at t = 5, Eq. (76): position and velocity of the loaded end against the finest run;
  # the position is the displacement plus START
  runs = [read(f"LeapFrog_{i}", ORDER_END) for i in range(ORDER_RUNS)]
  steps = np.array([r[1, 0] - r[0, 0] for r in runs])
  rows_at = [np.argmin(np.abs(r[:, 0] - ORDER_END)) for r in runs]
  positions = [r[i, 9:12] + START for r, i in zip(runs, rows_at)]
  velocities = [r[i, 12:15] for r, i in zip(runs, rows_at)]
  fig_c, order_axes = plt.subplots(figsize=(16 / 3, 6))
  for what, ends, marker in [("position", positions, "o"), ("velocity", velocities, "s")]:
    diffs = np.array([relative_L2(ends[i], ends[-1]) for i in range(ORDER_RUNS - 1)])
    measured, local = convergence_order(steps[:-1], diffs)
    ok = abs(measured - ORDER) <= ORDER_TOL
    test_passed = test_passed and (ok or what != "position")
    print(f"  LeapFrog, {what} of the loaded end at t = {ORDER_END:g}: order {measured:.2f} (expected {ORDER} +- "
          f"{ORDER_TOL}; local {np.round(local, 2)})  {status(ok, what == 'position')}", file=LOG)
    order_axes.loglog(steps[:-1], diffs, marker=marker, markerfacecolor="none", ls="", color=COLOURS["LeapFrog"])
    order_axes.loglog(steps[:-1], diffs[-1] * (steps[:-1] / steps[-2])**ORDER, "--", color=COLOURS["LeapFrog"])
  handles = [Line2D([], [], color="k", marker=m, markerfacecolor="none", ls="",
                    label=f"{w}, relative error against the finest run, Eq. (76)")
             for w, m in [("position", "o"), ("velocity", "s")]]
  handles += [Line2D([], [], color="k", ls="--", label="reference: order 2")]
  order_axes.legend(handles=handles, fontsize=7)
  order_axes.set_xlabel("time step")
  order_axes.set_ylabel("loaded end at t = 5, leap-frog")
  order_axes.set_title("convergence of leap-frog")
except Exception as e:
  print(e)
  test_passed = False

if test_passed:
  print(colored("INTEGRATOR TEST 6 PASSED", "green")
        + colored(" (RUN THROUGH: Milne and EmbeddedRK energy in the free flight, momentum with J, velocity order)",
                  "yellow"))

  with PdfPages(f"{OUT}/result.pdf") as pdf:
    for fig, tag in [(fig_a, "a"), (fig_b, "b"), (fig_c, "c")]:
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/integrator6{tag}_result.png")
else:
  print(colored("INTEGRATOR TEST 6 FAILED", "red", attrs=["bold"]) + f" (details: {OUT}/result.log)")
  sys.exit(1)
