#!/usr/bin/python3

# TEST 3: bending frequencies of a cantilever against the Timoshenko beam, Han, Benaroya, Wei (1999)
import sys

import numpy as np
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.lines import Line2D
from scipy.optimize import brentq
from termcolor import colored

OUT = "tests/integrator/test3"
LENGTH, YOUNG, NU, DENSITY = 1., 1e4, 0.3, 1.
SHEAR = 6. * (1. + NU) / (7. + 6. * NU)    # Cowper (1966), solid circle
MODES = 2

# name, radius; the runs are listed in tests/testing.mk, each over ten periods of the first mode
SLENDERNESS = [("slender", 0.02), ("stubby", 0.1)]
SCHEMES = {"LeapFrog": "leap-frog", "Milne": "MilneDevice: Adams-Bashforth-Moulton",
           "ode23": "EmbeddedRK ode23: Bogacki-Shampine", "ode45": "EmbeddedRK ode45: Dormand-Prince"}
COLOURS = {"LeapFrog": "C0", "Milne": "C1", "ode23": "C2", "ode45": "C3"}
SCHEME_TOL = (0.002, 0.01)    # every integrator on 16 elements, per mode
MESHES = [16, 32, 64]         # LeapFrog, for the convergence with the mesh
MESH_ORDER, MESH_ORDER_TOL = 2., 0.1    # of mode 2; mode 1 is below the precision of the spectrum on the finer meshes
FINE_TOL = (1e-4, 5e-4)       # on 64 elements, per mode

# Han, Benaroya, Wei (1999), Sect. 3.5, clamped-free Timoshenko beam: a tube of radii 0.16 and 0.15 m and L = 1 m
# (1/s printed as 0.11) with gamma = 2.205, and the exact wave numbers and frequency ratios they give
HAN_AREA, HAN_MOMENT, HAN_LENGTH = np.pi * (0.16**2 - 0.15**2), np.pi * (0.16**4 - 0.15**4) / 4., 1.
HAN_GAMMA = 2.205
HAN_A, HAN_B = [1.843, 4.236, 7.305, 9.813], [1.655, 2.727, 2.575, 0.803]
HAN_RATIOS = [1., 3.991, 8.412, 12.037]
HAN_TOL = 2e-3                # their values have three or four digits


# --- reference: Han, Benaroya, Wei (1999), only evaluated -------------------------------------------------------------
def slenderness(area, moment, length):
  """s = L / k with the radius of gyration k = sqrt(I / A) (nomenclature)."""
  return length / np.sqrt(moment / area)


def han_eq110(b, a, s, gamma):
  """Relation of the wave numbers for a < a_c, Eq. (110), as left side - s^2."""
  g2 = gamma**2
  return (g2 * b**2 + a**2) * (a**2 * g2 + b**2) / ((a**2 - b**2) * (1. + g2)) - s**2


def han_b(a, s, gamma):
  """The wave number b of a, the root of Eq. (110) between 0 and a."""
  return np.array([brentq(han_eq110, 0., x * (1. - 1e-12), args=(x, s, gamma)) for x in np.atleast_1d(a)])


def han_clamped_free(a, s, gamma):
  """Frequency equation of the clamped-free Timoshenko beam for a < a_c, Table 9 (c-f)."""
  b, g2 = han_b(a, s, gamma), gamma**2
  return ((a**2 - b**2) * np.sin(a) * np.sinh(b)
          - a * b * (a**4 + a**4 * g2**2 + 4. * g2 * a**2 * b**2 + b**4 * g2**2 + b**4)
          / ((b**2 + g2 * a**2) * (a**2 + g2 * b**2)) * np.cos(a) * np.cosh(b) - 2. * a * b)


def han_frequency(a, b, gamma):
  """Dimensionless frequency w, with w^2 = B3 of Eq. (109)."""
  g2 = gamma**2
  return np.sqrt(0.25 * ((a**2 + b**2)**2 - ((1. - g2) / (1. + g2))**2 * (a**2 - b**2)**2))


def han_roots(s, gamma, count):
  """The first wave numbers (a, b) below the critical a_c = s sqrt(1 / gamma^2 + 1), Eq. (79)."""
  a_c = s * np.sqrt(1. / gamma**2 + 1.)
  grid = np.linspace(1e-3, a_c * (1. - 1e-6), 4000)
  values = han_clamped_free(grid, s, gamma)
  found = [brentq(han_clamped_free, grid[i], grid[i + 1], args=(s, gamma))
           for i in np.nonzero(values[:-1] * values[1:] < 0.)[0]][:count]
  a = np.array(found)
  return a, han_b(a, s, gamma)


def timoshenko(radius, count):
  """Frequencies of the cantilever of this test: gamma^2 = 2 (1 + nu) / k' (Eq. 81), and the dimensionless frequency
  scaled with sqrt(E I / (rho A L^4)) (Eq. 83)."""
  area, moment = np.pi * radius**2, np.pi * radius**4 / 4.
  gamma = np.sqrt(2. * (1. + NU) / SHEAR)
  a, b = han_roots(slenderness(area, moment, LENGTH), gamma, count)
  return han_frequency(a, b, gamma) * np.sqrt(YOUNG * moment / (DENSITY * area * LENGTH**4))


# --- evaluation ---------------------------------------------------------------------------------------------------------
def measured_frequencies(t, signal, count):
  """Frequencies of the strongest separated peaks of the windowed spectrum, refined by a parabola through the log."""
  samples = signal - signal.mean()
  padded = 8 * len(samples)
  spectrum = np.abs(np.fft.rfft(samples * np.hanning(len(samples)), padded))
  freqs = np.fft.rfftfreq(padded, t[1] - t[0]) * 2. * np.pi
  log = np.log(spectrum + 1e-300)
  peaks = [i for i in range(1, len(log) - 1) if log[i] > log[i - 1] and log[i] > log[i + 1]]
  peaks = sorted(sorted(peaks, key=lambda i: -spectrum[i])[:count])
  result = []
  for i in peaks:
    shift = 0.5 * (log[i - 1] - log[i + 1]) / (log[i - 1] - 2. * log[i] + log[i + 1])
    result.append(freqs[i] + shift * (freqs[1] - freqs[0]))
  return np.array(result), freqs, spectrum


def read(name, exact):
  rows = np.loadtxt(f"{OUT}/{name}.csv", delimiter=",", skiprows=1, ndmin=2)
  if rows[-1, 0] < 0.99 * 10. * 2. * np.pi / exact[0]:
    raise RuntimeError(f"{name}: the run is shorter than ten periods, see the .log files in {OUT}")
  return rows


def status(ok):
  return "ok" if ok else "off"


# details of the checks; on screen only PASSED, FAILED or RUN THROUGH
LOG = open("tests/integrator/test3/result.log", "w")

test_passed = True

try:
  # the reference against the example of Han et al.
  a, b = han_roots(slenderness(HAN_AREA, HAN_MOMENT, HAN_LENGTH), HAN_GAMMA, 4)
  ratios = han_frequency(a, b, HAN_GAMMA) / han_frequency(a[0], b[0], HAN_GAMMA)
  deviation = max(np.abs(a - HAN_A).max(), np.abs(b - HAN_B).max(), np.abs(ratios - HAN_RATIOS).max())
  ok = deviation <= HAN_TOL
  test_passed = test_passed and ok
  print(f"  reference against Han et al. (1999), Sect. 3.5: a {np.round(a, 3)}, b {np.round(b, 3)}, ratios "
        f"{np.round(ratios, 3)}; largest deviation {deviation:.1e} (limit {HAN_TOL})  {status(ok)}", file=LOG)

  figures, mesh_errors = [], {}
  for label, radius in SLENDERNESS:
    exact = timoshenko(radius, MODES)
    fig, axes = plt.subplots(figsize=(16 / 3, 6))
    figures.append(fig)
    for name in SCHEMES:
      rows = read(f"{label}_{name}", exact)
      found, freqs, spectrum = measured_frequencies(rows[:, 0], rows[:, 1], MODES)
      errors = found / exact - 1.
      ok = bool(np.all(np.abs(errors) <= SCHEME_TOL))
      test_passed = test_passed and ok
      print(f"  {label} {name}, 16 elements: {np.round(found, 4)} rad/s, Timoshenko {np.round(exact, 4)}, error "
            f"{np.round(errors * 100., 3)} % (limit {np.array(SCHEME_TOL) * 100.} %)  {status(ok)}", file=LOG)
      axes.plot(freqs, spectrum / spectrum.max(), color=COLOURS[name], label=SCHEMES[name])
    for i, w in enumerate(exact):
      axes.axvline(w, color="k", ls="--", lw=1.,
                   label="reference: Timoshenko beam, Han et al. (1999),\nshear coefficient Cowper (1966)" if i == 0 else None)
    axes.set_xlim(0., 1.3 * exact[-1])
    axes.set_ylim(0., 1.05)
    axes.set_xlabel("frequency [rad/s]")
    axes.set_ylabel("spectrum of the tip displacement / its peak")
    axes.set_title(f"{label}, L / r = {LENGTH / radius:.0f}, 16 elements; dashed: Timoshenko")
    axes.legend(fontsize=7)

    # convergence with the mesh, LeapFrog
    errors = []
    for n in MESHES:
      rows = read(f"{label}_LeapFrog" + ("" if n == 16 else f"_{n}"), exact)
      errors.append(measured_frequencies(rows[:, 0], rows[:, 1], MODES)[0] / exact - 1.)
    errors = np.abs(np.array(errors))
    order = np.polyfit(np.log(1. / np.array(MESHES)), np.log(errors[:, 1]), 1)[0]
    ok = abs(order - MESH_ORDER) <= MESH_ORDER_TOL and bool(np.all(errors[-1] <= FINE_TOL))
    test_passed = test_passed and ok
    print(f"  {label} LeapFrog, {' / '.join(map(str, MESHES))} elements: error mode 1 "
          f"{' / '.join(f'{e * 100:.4f}' for e in errors[:, 0])} %, mode 2 {' / '.join(f'{e * 100:.4f}' for e in errors[:, 1])} %; "
          f"order of mode 2 {order:.2f} (expected {MESH_ORDER} +- {MESH_ORDER_TOL}), 64 elements within "
          f"{np.array(FINE_TOL) * 100.} %  {status(ok)}", file=LOG)
    mesh_errors[label] = errors

  fig, axes = plt.subplots(figsize=(16 / 3, 6))
  figures.append(fig)
  sizes = LENGTH / np.array(MESHES)
  markers = {"slender": "o", "stubby": "s"}
  for label, errors in mesh_errors.items():
    for mode, colour in [(0, "C4"), (1, "C5")]:
      axes.loglog(sizes, errors[:, mode], marker=markers[label], markerfacecolor="none", ls="", color=colour)
    for mode, colour in [(0, "C4"), (1, "C5")]:
      axes.loglog(sizes, errors[0, mode] * (sizes / sizes[0])**MESH_ORDER, "--", color=colour)
  handles = [Line2D([], [], color="C4", label="mode 1"), Line2D([], [], color="C5", label="mode 2")]
  handles += [Line2D([], [], color="k", marker=m, markerfacecolor="none", ls="", label=l) for l, m in markers.items()]
  handles += [Line2D([], [], color="k", ls="--", label="reference: order 2")]
  axes.legend(handles=handles, fontsize=7)
  axes.set_xlabel("element length / L")
  axes.set_ylabel("|frequency / Timoshenko - 1|, LeapFrog")
  axes.set_title("convergence with the mesh; dashed: order 2")
except Exception as e:
  print(e)
  test_passed = False

if test_passed:
  print(colored("INTEGRATOR TEST 3 PASSED", "green"))

  with PdfPages(f"{OUT}/result.pdf") as pdf:
    for fig, tag in zip(figures, "abc"):
      fig.tight_layout()
      pdf.savefig(fig)
      fig.savefig(f"tests/integrator3{tag}_result.png")
else:
  print(colored("INTEGRATOR TEST 3 FAILED", "red", attrs=["bold"]) + " (details: tests/integrator/test3/result.log)")
  sys.exit(1)
