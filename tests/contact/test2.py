#!/usr/bin/python3

# TEST 2: two free rods crossing: they must not pass, and must conserve momentum and energy through the impact
import sys
from pathlib import Path

import numpy as np
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from termcolor import colored

sys.path.insert(0, str(Path(__file__).parent))
from contact_state import linear_momentum, read_state

ELEMENTS = 20
RHO_A = np.pi * 0.02**2
ELEMENT = 2. / ELEMENTS
ENERGY_TOL = 0.01

# details of the checks; on screen only PASSED, FAILED or RUN THROUGH
LOG = open("tests/contact/test2/result.log", "w")

test_passed = False

try:
  times, pos_a, vel_a, pos_b, vel_b = read_state(
      "tests/contact/test2/state.gz", (-1., 0., 0.06), (1., 0., 0.06), (0., -1., -0.06), (0., 1., -0.06), ELEMENTS)
  energy = np.loadtxt("tests/contact/test2/disp.csv", delimiter=",", skiprows=0, ndmin=2)

  # height difference of the centres, where the rods cross
  gap = pos_a[:, ELEMENTS // 2, 2] - pos_b[:, ELEMENTS // 2, 2]
  momentum_a, momentum_b = linear_momentum(vel_a, RHO_A, ELEMENT), linear_momentum(vel_b, RHO_A, ELEMENT)
  total = momentum_a + momentum_b
  start = momentum_a[0, 2]
  momentum_error = np.abs(total).max() / abs(start)
  rebound = momentum_a[-1, 2] / start
  total_energy = energy[:, 1] + energy[:, 2]
  energy_change = abs(total_energy[-1] / total_energy[0] - 1.)

  print(f"  smallest gap of the centres {gap.min():.4f} (contact distance 0.1), final gap {gap[-1]:.4f}", file=LOG)
  print(f"  total momentum, largest {momentum_error:.2e} of the momentum of one rod; momentum of rod A at the end "
        f"{rebound:+.3f} of its start", file=LOG)
  print(f"  energy at the end against the start: {energy_change:.2e} (limit {ENERGY_TOL})", file=LOG)

  fig_a, gap_axes = plt.subplots(figsize=(16 / 3, 6))
  gap_axes.plot(times, gap, label="simulation")
  gap_axes.axhline(0.1, color="0.5", ls=":", label="contact distance (2 x contact radius)")
  gap_axes.set_xlabel("t")
  gap_axes.set_ylabel("height difference of the centres at the crossing")
  gap_axes.set_title("crossing; dotted: contact distance")
  gap_axes.legend(fontsize=7)
  fig_b, momentum_axes = plt.subplots(figsize=(16 / 3, 6))
  momentum_axes.plot(times, momentum_a[:, 2] / start, label="rod A")
  momentum_axes.plot(times, momentum_b[:, 2] / start, label="rod B")
  momentum_axes.plot(times, total[:, 2] / start, label="total")
  momentum_axes.axhline(0., color="k", ls="--", label="reference: total conserved (zero)")
  momentum_axes.set_xlabel("t")
  momentum_axes.set_ylabel("momentum in z / start of rod A")
  momentum_axes.set_title("momentum; dashed: conserved (exact)")
  momentum_axes.legend(fontsize=7)
  fig_c, energy_axes = plt.subplots(figsize=(16 / 3, 6))
  # the penalty energy of the contact is not in E_pot yet (ROADMAP C3), so that the energy dips while the rods touch
  energy_axes.plot(energy[:, 0], total_energy / total_energy[0], label="simulation, without the penalty energy\nof the contact (dips while the rods touch)")
  energy_axes.axhline(1., color="k", ls="--", label="reference: energy conserved")
  energy_axes.set_xlabel("t")
  energy_axes.set_ylabel("(E_pot + E_kin) / E_0")
  energy_axes.set_title("energy; dashed: conserved (exact)")
  energy_axes.legend(fontsize=7)

  # the rods are flexible, so most of the momentum of rod A goes into bending: the rebound itself is not judged
  test_passed = gap.min() > 0. and gap.min() < 0.1 and gap[-1] > 0.1 and momentum_error <= 1e-6 and energy_change <= ENERGY_TOL
except Exception as e:
  print(e)

if test_passed:
  print(colored("CONTACT TEST 2 PASSED", "green"))

  with PdfPages("tests/contact/test2/result.pdf") as pdf:
    for fig, tag in [(fig_a, "a"), (fig_b, "b"), (fig_c, "c")]:
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/contact2{tag}_result.png")
else:
  print(colored("CONTACT TEST 2 FAILED", "red", attrs=["bold"]) + " (details: tests/contact/test2/result.log)")
  sys.exit(1)
