#!/usr/bin/python3

# TEST 5.4.1 from Herrnböck et al.
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).parent.parent))
from metrics import interp_on_reference, relative_L2

TOL = 0.12

test_passed = False

try:
  sim_disp = np.loadtxt("tests/plastic/test3/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/plastic/test3/resp.csv", delimiter=',')
  # NOTE: the reference CSVs have the header n1,,n2,,n3 (m1,,m2,,m3), but the
  # column pairs (2,3) and (4,5) are deliberately read swapped (component 2 from
  # (4,5), component 3 from (2,3)): the small components pair with the small
  # ones (simulated n2 is below 1e-3, the file's n3 is small; simulated n3
  # reaches 6.9, the file's n2 is large).
  ref_n = [None]*3
  ref_n[0] = np.loadtxt(
      "tests/plastic/ref_data/test3_ref_force.csv", delimiter=',', skiprows=2, usecols=(0, 1))
  ref_n[0] = ref_n[0][ref_n[0][:, 0].argsort()]
  ref_n[1] = np.loadtxt(
      "tests/plastic/ref_data/test3_ref_force.csv", delimiter=',', skiprows=2, usecols=(4, 5), max_rows=32)
  ref_n[1] = ref_n[1][ref_n[1][:, 0].argsort()]
  ref_n[2] = np.loadtxt(
      "tests/plastic/ref_data/test3_ref_force.csv", delimiter=',', skiprows=2, usecols=(2, 3), max_rows=27)
  ref_n[2] = ref_n[2][ref_n[2][:, 0].argsort()]
  ref_m = [None]*3
  ref_m[0] = np.loadtxt(
      "tests/plastic/ref_data/test3_ref_torque.csv", delimiter=',', skiprows=2, usecols=(0, 1), max_rows=32)
  ref_m[0] = ref_m[0][ref_m[0][:, 0].argsort()]
  ref_m[1] = np.loadtxt(
      "tests/plastic/ref_data/test3_ref_torque.csv", delimiter=',', skiprows=2, usecols=(4, 5), max_rows=33)
  ref_m[1] = ref_m[1][ref_m[1][:, 0].argsort()]
  ref_m[2] = np.loadtxt(
      "tests/plastic/ref_data/test3_ref_torque.csv", delimiter=',', skiprows=2, usecols=(2, 3))
  ref_m[2] = ref_m[2][ref_m[2][:, 0].argsort()]

  # An independent relative error per component is useless here, as some
  # reference components are close to zero. Instead the error of each component
  # is normalised by the norm of the dominant reference component of its group.
  lam = sim_resp[:, 6] if sim_resp.shape[1] == 7 else np.linspace(
      0, 1, len(sim_resp))
  errs = []
  for group, ref, off, fac in (("n", ref_n, 0, 1.0), ("m", ref_m, 3, 1e-3)):
    y_ref = [r[:, 1]*fac for r in ref]
    dominant = max(y_ref, key=np.linalg.norm)
    for i in range(3):
      sim_i = interp_on_reference(lam, sim_resp[:, off+i], ref[i][:, 0])
      errs.append(relative_L2(sim_i, y_ref[i], scale=dominant))

  fig, axs = plt.subplots(1, 2, figsize=(32/3, 6))

  scale = sim_resp[:, 6] if sim_resp.shape[1] == 7 else np.linspace(
      0, 1, len(sim_resp))

  for i in range(3):
    axs[0].plot(scale,
                sim_resp[:, i], label=f"n{i+1} (custom implementation)")
    axs[0].plot(ref_n[i][:, 0], ref_n[i][:, 1],
                ":", label=f"n{i+1} (Herrnböck et al. (2023))")
    axs[1].plot(scale,
                sim_resp[:, i+3], label=f"m{i+1} (custom implementation)")
    axs[1].plot(ref_m[i][:, 0], ref_m[i][:, 1]*1e-3,
                ":", label=f"m{i+1} (Herrnböck et al. (2023))")
  axs[0].legend()
  axs[1].legend()
  axs[0].set_ylabel("forces (N)")
  axs[1].set_ylabel("moments (Nm)")
  axs[0].set_xlabel(r"$\lambda$")
  axs[1].set_xlabel(r"$\lambda$")
  axs[0].set_ylim(-5, 25)
  axs[1].set_ylim(-1, 2)
  axs[0].set_xlim(0, 1)
  axs[1].set_xlim(0, 1)
except Exception as e:
  print(e)
else:
  test_passed = all(np.isfinite(e) and e <= TOL for e in errs)

if test_passed:
  print(colored("PLASTIC TEST 3 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/plastic/test3/result.pdf")
  plt.savefig("tests/plastic3_result.png")
else:
  print(colored("PLASTIC TEST 3 FAILED", "red", attrs=["bold"]))
  sys.exit(1)
