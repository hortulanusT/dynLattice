#!/usr/bin/python3

# TEST 4: freely spinning body of Krysl, Endres (2005), Sect. 4.1, against their Fig. 3
import sys
from pathlib import Path

import numpy as np
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.lines import Line2D
from scipy.spatial.transform import Rotation
from termcolor import colored

sys.path.insert(0, str(Path(__file__).parent.parent))
from metrics import interp_on_reference, relative_L2

OUT = "tests/integrator/test4"
END = 100.
# body-frame angular velocity of Krysl, Endres (2005), Fig. 3 (their finest step), read from the vector paths of the figure
REFERENCE = "tests/integrator/ref_data/test4_ref_omega.csv"
OMEGA_TOL = 0.01      # relative L2 of each component, scaled by the largest one (Omega_2), as in plastic test 3
SCHEMES = {"LeapFrog": "leap-frog", "Milne": "MilneDevice: Adams-Bashforth-Moulton",
           "ode23": "EmbeddedRK ode23: Bogacki-Shampine", "ode45": "EmbeddedRK ode45: Dormand-Prince"}
COLOURS = {"LeapFrog": "C0", "Milne": "C1", "ode23": "C2", "ode45": "C3"}
# run: integrator, checked. Not checked, only run through, until ROADMAP C2: EmbeddedRK in 3D (mass frozen in the stages)
RUNS = {"LeapFrog_0": ("LeapFrog", True), "Milne": ("Milne", True), "ode23": ("ode23", False), "ode45": ("ode45", False)}
# LeapFrog at five steps, each half the previous one: order 2 expected; run through until ROADMAP C2, as the integrators
# are first order on rigid rotation
ORDER_RUNS, ORDER = 5, 2.


def read(name):
  rows = np.loadtxt(f"{OUT}/{name}.csv", delimiter=",", skiprows=1, ndmin=2)
  if rows[-1, 0] < END - 2. * (rows[1, 0] - rows[0, 0]):
    raise RuntimeError(f"{name}: the run did not reach t = {END}, see the .log files in {OUT}")
  return rows


def body_angular_velocity(rows):
  """Omega = R^T w: the angular velocity of the free end in the frame of the body, which is the global frame at t = 0."""
  return np.einsum("nji,nj->ni", Rotation.from_rotvec(rows[:, 4:7]).as_matrix(), rows[:, 9:12])


def energy_error(rows, leapfrog):
  """Largest |E(t) / E(0) - 1|; LeapFrog stores the velocity half a step early, see test 1."""
  e_kin, e_pot = rows[:, 8], rows[:, 7]
  if leapfrog:
    e_kin, e_pot = 0.5 * (e_kin[:-1] + e_kin[1:]), e_pot[:-1]
  energy = e_pot + e_kin
  return np.abs(energy / energy[0] - 1.).max()


def status(ok, checked=True):
  return (colored("ok", "green") if ok else colored("off", "red")) if checked else colored("RUN THROUGH", "yellow")


test_passed = True

try:
  data = np.genfromtxt(REFERENCE, delimiter=",", skip_header=2)
  reference = [data[np.isfinite(data[:, 2 * i]), 2 * i:2 * i + 2] for i in range(3)]
  dominant = max([curve[:, 1] for curve in reference], key=np.linalg.norm)

  fig_a, omega_axes = plt.subplots(figsize=(16 / 3, 6))
  for run, (scheme, checked) in RUNS.items():
    rows = read(run)
    omega = body_angular_velocity(rows)
    errors = [relative_L2(interp_on_reference(rows[:, 0], omega[:, i], curve[:, 0]), curve[:, 1], scale=dominant)
              for i, curve in enumerate(reference)]
    ok = max(errors) <= OMEGA_TOL
    test_passed = test_passed and (ok or not checked)
    print(f"  {scheme}, step {rows[1, 0] - rows[0, 0]:g}: Omega against Fig. 3, relative L2 "
          f"{', '.join(f'{e:.4f}' for e in errors)} (limit {OMEGA_TOL})  {status(ok, checked)}")
    if scheme == "LeapFrog":
      for i in range(3):
        omega_axes.plot(rows[:, 0], omega[:, i], color=f"C{i}", label=f"Omega_{i + 1}, leap-frog")
  for i, curve in enumerate(reference):
    omega_axes.plot(curve[:, 0], curve[:, 1], "k--", lw=1., label="reference: Krysl, Endres (2005), Fig. 3" if i == 0 else None)
  omega_axes.set_xlabel("t")
  omega_axes.set_ylabel("angular velocity, body frame")
  omega_axes.set_title("body angular velocity; dashed: Krysl, Endres")
  omega_axes.legend(fontsize=7, loc="lower right")

  # convergence of LeapFrog: rotation at the end against the run at half the step, and the energy error
  runs = [read(f"LeapFrog_{i}") for i in range(ORDER_RUNS)]
  steps = np.array([r[1, 0] - r[0, 0] for r in runs])
  ends = [Rotation.from_rotvec(r[-1, 4:7]) for r in runs]
  diffs = np.array([(ends[i].inv() * ends[i + 1]).magnitude() for i in range(ORDER_RUNS - 1)])
  energies = np.array([energy_error(r, True) for r in runs])
  fig_b, order_axes = plt.subplots(figsize=(16 / 3, 6))
  for what, x, y, marker in [("rotation", steps[:-1], diffs, "o"), ("energy", steps, energies, "s")]:
    measured = np.polyfit(np.log(x), np.log(y), 1)[0]
    local = np.log2(y[:-1] / y[1:])
    print(f"  LeapFrog, {what}: order {measured:.2f} (expected {ORDER}; local {', '.join(f'{v:.2f}' for v in local)})  "
          f"{status(False, False)}")
    order_axes.loglog(x, y, marker=marker, markerfacecolor="none", ls="", color=COLOURS["LeapFrog"])
    order_axes.loglog(x, y[-1] * (x / x[-1])**ORDER, "--", color=COLOURS["LeapFrog"])
  handles = [Line2D([], [], color="k", marker="o", markerfacecolor="none", ls="", label="rotation at t = 100: difference "
                    "to the run at half the step [rad]"),
             Line2D([], [], color="k", marker="s", markerfacecolor="none", ls="", label="energy error, largest |E / E(0) - 1|"),
             Line2D([], [], color="k", ls="--", label="reference: order 2")]
  order_axes.legend(handles=handles, fontsize=7)
  order_axes.set_xlabel("time step")
  order_axes.set_ylabel("error, leap-frog")
  order_axes.set_title("convergence (run through: first order, open)")
except Exception as e:
  print(e)
  test_passed = False

if test_passed:
  print(colored("INTEGRATOR TEST 4 PASSED", "green") + colored(" (RUN THROUGH: EmbeddedRK, order and energy)", "yellow"))

  with PdfPages(f"{OUT}/result.pdf") as pdf:
    for fig, tag in [(fig_a, "a"), (fig_b, "b")]:
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/integrator4{tag}_result.png")
else:
  print(colored("INTEGRATOR TEST 4 FAILED", "red", attrs=["bold"]))
  sys.exit(1)
