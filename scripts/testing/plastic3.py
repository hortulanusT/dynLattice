#!/usr/bin/python3

# TEST 4 from Herrnböck et al.
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/plastic/test3/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/plastic/test3/resp.csv", delimiter=',')
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

  fig, axs = plt.subplots(1, 2, figsize=(32/3, 6))
  for i in range(3):
    axs[0].plot(np.linspace(0, 1, len(sim_disp)),
                sim_resp[:, i], label=f"n{i+1} (custom)")
    axs[0].plot(ref_n[i][:, 0], ref_n[i][:, 1]*1e3,
                ":", label=f"n{i+1} (Herrnböck et al.)")
    axs[1].plot(np.linspace(0, 1, len(sim_disp)),
                sim_resp[:, i+3], label=f"m{i+1} (custom)")
    axs[1].plot(ref_m[i][:, 0], ref_m[i][:, 1],
                ":", label=f"m{i+1} (Herrnböck et al.)")
  axs[0].legend()
  axs[1].legend()
  axs[0].set_ylabel("n [N]")
  axs[1].set_ylabel("m [Nm]")
  axs[0].set_xlabel(r"$\lambda$")
  axs[1].set_xlabel(r"$\lambda$")
  axs[0].set_xlim(0, 1)
  axs[1].set_xlim(0, 1)
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("PLASTIC TEST 3 RUN THROUGH", "green"))

  plt.tight_layout()
  plt.savefig("tests/plastic/test3/result.pdf")
else:
  print(colored("PLASTIC TEST 3 FAILED", "red", attrs=["bold"]))
