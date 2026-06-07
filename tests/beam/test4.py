#!/usr/bin/python3

# TEST 4 buckling
import sys
import numpy as np
from pathlib import Path
from termcolor import colored
from matplotlib import pyplot as plt

sys.path.insert(0, str(Path(__file__).parent.parent))
from metrics import curve_distance_2d

TOL = 0.05

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
           * 1e3, ":", label="vertical (Simo, Vu-Quoc (1986))")
  plt.plot(ref_data_horizontal[:, 0], ref_data_horizontal[:, 1]
           * 1e3, ":", label="horizontal (Simo, Vu-Quoc (1986))")
  plt.legend()
  plt.xlabel("displacement (m)")
  plt.ylabel("load (N)")
  plt.xlim(left=0)

  # Non-monotone post-buckling branch
  force = -1*sim_resp[:, 1]
  sim_vert = np.column_stack([-1*sim_disp[:, 1], force])
  sim_horz = np.column_stack([+1*sim_disp[:, 0], force])
  ref_vert_xy = np.column_stack([ref_data_vertical[:, 0], ref_data_vertical[:, 1]*1.0e3])
  ref_horz_xy = np.column_stack([ref_data_horizontal[:, 0], ref_data_horizontal[:, 1]*1.0e3])
  err_vert = curve_distance_2d(sim_vert, ref_vert_xy)
  err_horz = curve_distance_2d(sim_horz, ref_horz_xy)
  test_passed = (err_vert <= TOL) and (err_horz <= TOL)

except Exception as e:
  print(e)

if test_passed:
  print(colored("STATIC TEST 4 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/beam/test4/result.pdf")
  plt.savefig("tests/beam4_result.png")
else:
  print(colored("STATIC TEST 4 FAILED", "red", attrs=["bold"]))
  sys.exit(1)
