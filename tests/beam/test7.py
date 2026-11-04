#!/usr/bin/python3

# TEST 7: roll-up by a prescribed total rotation, passes |theta| = 2 pi
import re
import sys
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/beam/test7/disp.csv", delimiter=",")
  steps = 80

  iters = []
  for line in open("tests/beam/test7/run.log"):
    match = re.search(r"iter = (\d+), scaled residual", line)
    if match:
      if int(match.group(1)) == 1:
        iters.append(0)
      iters[-1] = int(match.group(1))

  # the Dirichlet model starts at zero in the first step
  prescribed = np.arange(len(sim_disp)) * 8 * np.pi / steps
  angle = sim_disp[1:, 5]
  circle_x = -(1 - np.cos(angle)) / angle
  circle_y = np.sin(angle) / angle - 1

  fig, ax = plt.subplots(1, 2, figsize=(11, 5))
  ax[0].plot(sim_disp[1:, 0], sim_disp[1:, 1], "x", label="custom implementation")
  ax[0].plot(circle_x, circle_y, label="planar circle")
  ax[0].set_xlabel("tip dx")
  ax[0].set_ylabel("tip dy")
  ax[0].legend()
  ax[1].plot(prescribed, sim_disp[:, 5] - prescribed)
  ax[1].set_xlabel("prescribed rotation")
  ax[1].set_ylabel("rz - prescribed")

  test_passed = (len(sim_disp) == steps
                 and np.max(iters) <= 4
                 and np.allclose(sim_disp[:, 5], prescribed, atol=1e-6)
                 and np.allclose(sim_disp[1:, 0], circle_x, atol=1e-2)
                 and np.allclose(sim_disp[1:, 1], circle_y, atol=1e-2))
except Exception as e:
  print(e)

if test_passed:
  print(colored("STATIC TEST 7 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/beam/test7/result.pdf")
  plt.savefig("tests/beam7_result.png")
else:
  print(colored("STATIC TEST 7 FAILED", "red", attrs=["bold"]))
  sys.exit(1)
