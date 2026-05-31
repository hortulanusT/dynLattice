#!/usr/bin/python3

# TEST 2 aka Follower Load
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
  sim_disp = np.loadtxt("tests/beam/test2/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test2/resp.csv", delimiter=',')
  ref_data_vertical = np.loadtxt(
      "tests/beam/ref_data/test2_ref.csv", delimiter=',', skiprows=2, usecols=(0, 1))
  ref_data_horizontal = np.loadtxt(
      "tests/beam/ref_data/test2_ref.csv", delimiter=',', skiprows=2, usecols=(2, 3), max_rows=96)

  plt.figure(figsize=(16/3, 6))
  plt.plot(sim_resp[:, -1]/1e3, sim_disp[:, 2],
           label="vertical (custom implementation)")
  plt.plot(sim_resp[:, -1]/1e3, -1*sim_disp[:, 1],
           label="horizontal (custom implementation)")
  plt.plot(ref_data_vertical[:, 0], ref_data_vertical[:,
           1], ":", label="vertical (Simo, Vu-Quoc (1986))")
  plt.plot(ref_data_horizontal[:, 0], ref_data_horizontal[:,
           1], ":", label="horizontal (Simo, Vu-Quoc (1986))")
  plt.legend()
  plt.xlabel("tip load (kN)")
  plt.ylabel("displacement (m)")
  plt.xlim([0, 140])
  plt.axhline(y=0, color="grey")
  plt.xlim(left=0)

  load_kN = sim_resp[:, -1] / 1.0e3
  tip_vert = interp_on_reference(load_kN, sim_disp[:, 2], ref_data_vertical[:, 0])
  tip_horz = interp_on_reference(load_kN, -1*sim_disp[:, 1], ref_data_horizontal[:, 0])
  err_vert = relative_L2(tip_vert, ref_data_vertical[:, 1])
  err_horz = relative_L2(tip_horz, ref_data_horizontal[:, 1])
  test_passed = (err_vert <= TOL) and (err_horz <= TOL)

except Exception as e:
  print(e)

if test_passed:
  print(colored("STATIC TEST 2 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/beam/test2/result.pdf")
  plt.savefig("tests/beam2_result.png")
else:
  print(colored("STATIC TEST 2 FAILED", "red", attrs=["bold"]))
  sys.exit(1)
