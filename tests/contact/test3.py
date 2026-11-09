#!/usr/bin/python3

# TEST 3: two equal free rods meeting tip to tip: the velocities are exchanged, as for equal bars
import sys
from pathlib import Path

import numpy as np
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from termcolor import colored

sys.path.insert(0, str(Path(__file__).parent))
from contact_state import linear_momentum, read_state

ELEMENTS = 10
RHO_A = np.pi * 0.02**2
ELEMENT = 1. / ELEMENTS
# the rebound converges to -1 with the mesh: -0.962, -0.976, -0.982 for 10, 20, 40 elements per rod
REBOUND_TOL = 0.05
ENERGY_TOL = 0.02

# details of the checks; on screen only PASSED, FAILED or RUN THROUGH
LOG = open("tests/contact/test3/result.log", "w")

test_passed = False

try:
  times, pos_a, vel_a, pos_b, vel_b = read_state(
      "tests/contact/test3/state.gz", (0., -1.06, 0.), (0., -0.06, 0.), (0., 0.06, 0.), (0., 1.06, 0.), ELEMENTS)
  energy = np.loadtxt("tests/contact/test3/disp.csv", delimiter=",", ndmin=2)

  gap = pos_b[:, 0, 1] - pos_a[:, -1, 1]
  momentum_a = linear_momentum(vel_a, RHO_A, ELEMENT)[:, 1]
  momentum_b = linear_momentum(vel_b, RHO_A, ELEMENT)[:, 1]
  momentum_error = np.abs(momentum_a + momentum_b).max() / abs(momentum_a[0])
  rebound = momentum_a[-1] / momentum_a[0]
  total_energy = energy[:, 1] + energy[:, 2]
  energy_change = abs(total_energy[-1] / total_energy[0] - 1.)

  print(f"  smallest gap of the tips {gap.min():.4f} (contact distance 0.1), final gap {gap[-1]:.4f}", file=LOG)
  print(f"  momentum of rod A at the end {rebound:+.3f} of its start (exact -1, limit +- {REBOUND_TOL}), "
        f"total momentum "
        f"largest {momentum_error:.2e}", file=LOG)
  print(f"  energy at the end against the start: {energy_change:.2e} (limit {ENERGY_TOL})", file=LOG)

  fig_a, gap_axes = plt.subplots(figsize=(16 / 3, 6))
  gap_axes.plot(times, gap, label="simulation")
  gap_axes.axhline(0.1, color="0.5", ls=":", label="contact distance (2 x joint contact radius)")
  gap_axes.set_xlabel("t")
  gap_axes.set_ylabel("gap of the tips")
  gap_axes.set_title("tips; dotted: contact distance")
  gap_axes.legend(fontsize=7)
  fig_b, momentum_axes = plt.subplots(figsize=(16 / 3, 6))
  momentum_axes.plot(times, momentum_a / momentum_a[0], label="rod A")
  momentum_axes.plot(times, momentum_b / momentum_a[0], label="rod B")
  exchange = "reference after the impact: equal elastic\nrods exchange their velocities (exact)"
  momentum_axes.axhline(-1., color="k", ls="--", label=exchange)
  momentum_axes.axhline(1., color="k", ls="--")
  momentum_axes.set_xlabel("t")
  momentum_axes.set_ylabel("momentum in y / start of rod A")
  momentum_axes.set_title("momentum; dashed: exact after the impact")
  momentum_axes.legend(fontsize=7)
  fig_c, energy_axes = plt.subplots(figsize=(16 / 3, 6))
  # the penalty energy of the contact is not part of E_pot, so that the energy dips while the tips touch
  label = "simulation, without the penalty energy\nof the contact (dips while the tips touch)"
  energy_axes.plot(energy[:, 0], total_energy / total_energy[0], label=label)
  energy_axes.axhline(1., color="k", ls="--", label="reference: energy conserved")
  energy_axes.set_xlabel("t")
  energy_axes.set_ylabel("(E_pot + E_kin) / E_0")
  energy_axes.set_title("energy; dashed: conserved (exact)")
  energy_axes.legend(fontsize=7)

  test_passed = (gap.min() > 0. and gap.min() < 0.1 and gap[-1] > 0.1 and abs(rebound + 1.) <= REBOUND_TOL
                 and momentum_error <= 1e-6 and energy_change <= ENERGY_TOL)
except Exception as e:
  print(e)

if test_passed:
  print(colored("CONTACT TEST 3 PASSED", "green"))

  with PdfPages("tests/contact/test3/result.pdf") as pdf:
    for fig, tag in [(fig_a, "a"), (fig_b, "b"), (fig_c, "c")]:
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/contact3{tag}_result.png")
else:
  print(colored("CONTACT TEST 3 FAILED", "red", attrs=["bold"]) + " (details: tests/contact/test3/result.log)")
  sys.exit(1)
