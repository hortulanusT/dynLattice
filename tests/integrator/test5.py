#!/usr/bin/python3

# TEST 5: pure rotation past |theta| = 2 pi, against the analytical solution of Holzinger, Gerstmayr (2021), Eq. (73)
import sys
from pathlib import Path

import numpy as np
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from termcolor import colored

sys.path.insert(0, str(Path(__file__).parent.parent))
from metrics import relative_L2

OUT = "tests/integrator/test5"
END = 100.
RATE = np.pi          # w0 = (0, pi, 0)
SCHEMES = {"LeapFrog": "leap-frog", "Milne": "MilneDevice: Adams-Bashforth-Moulton",
           "ode23": "EmbeddedRK ode23: Bogacki-Shampine", "ode45": "EmbeddedRK ode45: Dormand-Prince"}
COLOURS = {"LeapFrog": "C0", "Milne": "C1", "ode23": "C2", "ode45": "C3"}
THETA_TOL = 1e-8      # relative L2 of each component of the rotation vector, scaled by the reference theta_2
JUMP_TOL = 1e-6       # every step of |theta| is the rotation of one step, pi dt, up to this fraction


def analytical(t):
  """Eq. (73), alpha_2 = pi t: the rotation vector of a pure rotation about axis 2."""
  return np.column_stack([0. * t, RATE * t, 0. * t])


def status(ok):
  return colored("ok", "green") if ok else colored("off", "red")


test_passed = True

try:
  fig_a, theta_axes = plt.subplots(figsize=(16 / 3, 6))
  fig_b, error_axes = plt.subplots(figsize=(16 / 3, 6))
  for name in SCHEMES:
    rows = np.loadtxt(f"{OUT}/{name}.csv", delimiter=",", skiprows=1, ndmin=2)
    t, theta = rows[:, 0], rows[:, 4:7]
    step = t[1] - t[0]
    if t[-1] < END - 2. * step:
      raise RuntimeError(f"{name}: the run did not reach t = {END}, see the .log files in {OUT}")
    exact = analytical(t)
    errors = [relative_L2(theta[:, i], exact[:, i], scale=exact[:, 1]) for i in range(3)]
    jump = np.abs(np.abs(np.diff(np.linalg.norm(theta, axis=1))) / (RATE * step) - 1.).max()
    ok = max(errors) <= THETA_TOL and jump <= JUMP_TOL
    test_passed = test_passed and ok
    print(f"  {name}, step {step:g}: theta against Eq. (73), relative L2 {', '.join(f'{e:.1e}' for e in errors)} "
          f"(limit {THETA_TOL}); |theta| at the end {np.linalg.norm(theta[-1]) / np.pi:.4f} pi, steps of |theta| within "
          f"{jump:.1e} of pi dt (limit {JUMP_TOL})  {status(ok)}")
    theta_axes.plot(t, theta[:, 1] / np.pi, color=COLOURS[name], label=SCHEMES[name])
    error_axes.semilogy(t, np.linalg.norm(theta - exact, axis=1) + 1e-16, color=COLOURS[name], label=SCHEMES[name])

  theta_axes.plot(t, exact[:, 1] / np.pi, "k--", label="reference: Holzinger, Gerstmayr (2021), Eq. (73)")
  theta_axes.set_xlabel("t")
  theta_axes.set_ylabel("rotation vector theta_2 / pi")
  theta_axes.set_title("rotation; dashed: analytical")
  theta_axes.legend(fontsize=7)
  error_axes.set_xlabel("t")
  error_axes.set_ylabel("|theta - theta of Eq. (73)| [rad]")
  error_axes.set_title("deviation from the analytical solution (exact: 0)")
  error_axes.legend(fontsize=7)
except Exception as e:
  print(e)
  test_passed = False

if test_passed:
  print(colored("INTEGRATOR TEST 5 PASSED", "green"))

  with PdfPages(f"{OUT}/result.pdf") as pdf:
    for fig, tag in [(fig_a, "a"), (fig_b, "b")]:
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/integrator5{tag}_result.png")
else:
  print(colored("INTEGRATOR TEST 5 FAILED", "red", attrs=["bold"]))
  sys.exit(1)
