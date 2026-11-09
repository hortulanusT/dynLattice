#!/usr/bin/python3

# TEST 2: axial wave in a bar, exact d'Alembert solution
import sys

import numpy as np
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from termcolor import colored

OUT = "tests/integrator/test2"
STRAIN = 1e-3      # F / (E A)
FORCE = STRAIN * np.pi * 0.01**2
LENGTH = 1.
SPEED = 1.
ELEMENT = 0.01
END = 12.         # three cycles of the wave, 4 L / c each

# the runs, listed in tests/testing.mk; each at about the same accuracy per cost, not at the same step
SCHEMES = {"LeapFrog": "leap-frog", "Milne": "MilneDevice: Adams-Bashforth-Moulton",
           "ode23": "EmbeddedRK ode23: Bogacki-Shampine", "ode45": "EmbeddedRK ode45: Dormand-Prince"}
COLOURS = {"LeapFrog": "C0", "Milne": "C1", "ode23": "C2", "ode45": "C3"}
TIP_TOL = 5e-3     # relative L2 of the tip displacement
FRONT_TOL = 3. * ELEMENT / SPEED   # every front at the fixed end; dispersion makes them arrive early


def exact_tip(t):
  """Triangle wave: the tip moves with STRAIN * c until the reflected wave returns at 2 L / c, then back."""
  phase = np.mod(t * SPEED, 4. * LENGTH)
  return STRAIN * np.where(phase < 2. * LENGTH, phase, 4. * LENGTH - phase)


def exact_reaction(t):
  """Reaction at the fixed end over F: zero until the wave arrives at L / c, doubled by the reflection until the
  unloading wave arrives at 3 L / c, period 4 L / c."""
  phase = np.mod(t * SPEED, 4. * LENGTH)
  return np.where((phase > LENGTH) & (phase < 3. * LENGTH), 2., 0.)


# details of the checks; on screen only PASSED, FAILED or RUN THROUGH
LOG = open("tests/integrator/test2/result.log", "w")

test_passed = True

try:
  fig_a, tip_axes = plt.subplots(figsize=(16 / 3, 6))
  fig_b, reaction_axes = plt.subplots(figsize=(16 / 3, 6))
  for name in SCHEMES:
    rows = np.loadtxt(f"{OUT}/{name}.csv", delimiter=",", skiprows=1, ndmin=2)
    step = rows[1, 0] - rows[0, 0]
    t, tip, reaction = rows[:, 0], rows[:, 2], np.abs(rows[:, 9])

    tip_error = np.linalg.norm(tip - exact_tip(t)) / np.linalg.norm(exact_tip(t))
    # fronts at the fixed end, where the reaction crosses F: the loading wave arrives at (4 k + 1) L / c, the unloading
    # wave at (4 k + 3) L / c
    fronts = []
    for k in range(int(END * SPEED / (4. * LENGTH))):
      for exact, rising in [((4 * k + 1) * LENGTH / SPEED, True), ((4 * k + 3) * LENGTH / SPEED, False)]:
        window = np.abs(t - exact) < 0.5 * LENGTH / SPEED
        crossed = reaction[window] > FORCE if rising else reaction[window] < FORCE
        fronts.append(t[window][np.argmax(crossed)] - exact)
    front_error = np.abs(fronts).max()

    ok = tip_error <= TIP_TOL and front_error <= FRONT_TOL and t[-1] >= END - 2. * step
    test_passed = test_passed and ok
    print(f"  {name}: tip error {tip_error:.2e} (limit {TIP_TOL}), fronts at the fixed end "
          f"{', '.join(f'{x:+.4f}' for x in fronts)} from exact (limit +- {FRONT_TOL})" + ("" if ok else "  <-- off"), file=LOG)
    tip_axes.plot(t, tip / STRAIN, color=COLOURS[name], label=SCHEMES[name])
    reaction_axes.plot(t, reaction / FORCE, color=COLOURS[name], label=SCHEMES[name])

  tip_axes.plot(t, exact_tip(t) / STRAIN, "k--", label="reference: exact (d'Alembert)")
  tip_axes.set_xlabel("t c / L")
  tip_axes.set_ylabel("tip displacement / (L F / (E A))")
  tip_axes.set_title("loaded end; dashed: exact")
  tip_axes.legend(fontsize=7)
  reaction_axes.plot(t, exact_reaction(t), "k--", label="reference: exact (d'Alembert)")
  reaction_axes.set_xlabel("t c / L")
  reaction_axes.set_ylabel("reaction / F")
  reaction_axes.set_title("fixed end; dashed: exact")
  reaction_axes.legend(fontsize=7)
except Exception as e:
  print(e)
  test_passed = False

if test_passed:
  print(colored("INTEGRATOR TEST 2 PASSED", "green"))

  with PdfPages(f"{OUT}/result.pdf") as pdf:
    for fig, tag in [(fig_a, "a"), (fig_b, "b")]:
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/integrator2{tag}_result.png")
else:
  print(colored("INTEGRATOR TEST 2 FAILED", "red", attrs=["bold"]) + " (details: tests/integrator/test2/result.log)")
  sys.exit(1)
