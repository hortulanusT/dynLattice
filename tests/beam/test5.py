#!/usr/bin/python3

# TEST 5 3D
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

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
  plt.xlabel("load [N]")
  plt.ylabel("displacment [m]")
  plt.xlim(left=0, right=3000)
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("STATIC TEST 5 RUN THROUGH", "green"))

  plt.tight_layout()
  plt.savefig("tests/beam/test5/result.pdf")
  plt.savefig("tests/beam5_result.png")
else:
  print(colored("STATIC TEST 5 FAILED", "red", attrs=["bold"]))
