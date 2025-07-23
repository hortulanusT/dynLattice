#!/usr/bin/python3

# TEST 4 buckling
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/beam/test4/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test4/resp.csv", delimiter=',')
  ref_data_vertical = np.loadtxt(
      "tests/beam/ref_data/test4_ref_vertical.csv", delimiter=',', skiprows=1)
  ref_data_horizontal = np.loadtxt(
      "tests/beam/ref_data/test4_ref_horizontal.csv", delimiter=',', skiprows=1)

  plt.figure(figsize=(16/3, 6))
  plt.plot(+1*sim_disp[:, 0], -1*sim_resp[:, 1],
           label="horizontal (custom implementation)")
  plt.plot(-1*sim_disp[:, 1], -1*sim_resp[:, 1],
           label="vertical (custom implementation)")
  plt.plot(ref_data_vertical[:, 0], ref_data_vertical[:, 1]
           * 1e3, ":", label="vertical (Simo/Vu-Quoc)")
  plt.plot(ref_data_horizontal[:, 0], ref_data_horizontal[:, 1]
           * 1e3, ":", label="horizontal (Simo/Vu-Quoc)")
  plt.legend()
  plt.xlabel("displacment [m]")
  plt.ylabel("load [N]")
  plt.xlim(left=0)
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("STATIC TEST 4 RUN THROUGH", "green"))

  plt.tight_layout()
  plt.savefig("tests/beam/test4/result.pdf")
else:
  print(colored("STATIC TEST 4 FAILED", "red", attrs=["bold"]))
