#!/usr/bin/python3

# TEST 5 3D
import sys
import numpy as np
from pathlib import Path
from termcolor import colored
from matplotlib import pyplot as plt

sys.path.insert(0, str(Path(__file__).parent.parent))
from metrics import interp_on_reference, relative_L2

TOL = 0.05

test_passed = False

try:
  sim_disp = np.loadtxt("tests/beam/test5/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test5/resp.csv", delimiter=',')
  ref_data_u1 = np.loadtxt(
      "tests/beam/ref_data/test5_ref.csv", delimiter=',', skiprows=2, usecols=(0, 1))
  ref_data_u2 = np.loadtxt("tests/beam/ref_data/test5_ref.csv",
                           delimiter=',', skiprows=2, usecols=(2, 3), max_rows=48)
  ref_data_u3 = np.loadtxt("tests/beam/ref_data/test5_ref.csv",
                           delimiter=',', skiprows=2, usecols=(4, 5), max_rows=51)

  plt.figure(figsize=(12, 4))
  plt.plot(sim_resp[:, 2], sim_disp[:, 0], label="u_1 (custom_implementation)")
  plt.plot(sim_resp[:, 2], sim_disp[:, 1], label="u_2 (custom_implementation)")
  plt.plot(sim_resp[:, 2], sim_disp[:, 2], label="u_3 (custom_implementation)")
  plt.plot(ref_data_u1[:, 0], ref_data_u1[:, 1],
           ":", label="u_1 (Simo, Vu-Quoc (1986))")
  plt.plot(ref_data_u2[:, 0], ref_data_u2[:, 1],
           ":", label="u_2 (Simo, Vu-Quoc (1986))")
  plt.plot(ref_data_u3[:, 0], ref_data_u3[:, 1],
           ":", label="u_3 (Simo, Vu-Quoc (1986))")
  plt.legend(loc="upper left")
  plt.xlabel("load (N)")
  plt.ylabel("displacement (m)")
  plt.xlim(left=0, right=3000)

  # Separate check for each tip displacement component 
  load = sim_resp[:, 2]
  u1 = interp_on_reference(load, sim_disp[:, 0], ref_data_u1[:, 0])
  u2 = interp_on_reference(load, sim_disp[:, 1], ref_data_u2[:, 0])
  u3 = interp_on_reference(load, sim_disp[:, 2], ref_data_u3[:, 0])
  err_u1 = relative_L2(u1, ref_data_u1[:, 1])
  err_u2 = relative_L2(u2, ref_data_u2[:, 1])
  err_u3 = relative_L2(u3, ref_data_u3[:, 1])
  test_passed = all(e <= TOL for e in [err_u1, err_u2, err_u3])

except Exception as e:
  print(e)

if test_passed:
  print(colored("STATIC TEST 5 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/beam/test5/result.pdf")
  plt.savefig("tests/beam5_result.png")
else:
  print(colored("STATIC TEST 5 FAILED", "red", attrs=["bold"]))
  sys.exit(1)
