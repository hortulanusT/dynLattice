#!/usr/bin/python3

# TEST 6: roll-up to 4 pi with a changing rotation axis, passes |theta| = 2 pi
import re
import sys
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/beam/test6/disp.csv", delimiter=",")
  steps = 80

  # Newton iterations per load step
  iters = []
  for line in open("tests/beam/test6/run.log"):
    match = re.search(r"iter = (\d+), scaled residual", line)
    if match:
      if int(match.group(1)) == 1:
        iters.append(0)
      iters[-1] = int(match.group(1))

  theta = np.linalg.norm(sim_disp[:, 3:], axis=1)
  load_steps = np.arange(1, len(theta) + 1)

  fig, ax = plt.subplots(1, 3, figsize=(16, 5))
  ax[0].plot(load_steps, sim_disp[:, :3], label=["dx", "dy", "dz"])
  ax[0].set_ylabel("tip displacement")
  ax[0].legend()
  ax[1].plot(load_steps, theta)
  ax[1].axhline(2 * np.pi, color="gray", lw=0.8)
  ax[1].set_ylabel("|theta| of the tip")
  ax[2].plot(np.arange(1, len(iters) + 1), iters, "x")
  ax[2].set_ylabel("Newton iterations")
  for a in ax:
    a.set_xlabel("load step")

  # the moment rolls the rod into one and two full circles, the transverse force lifts it out of plane
  test_passed = (len(sim_disp) == steps
                 and np.max(iters) <= 4
                 and np.allclose(sim_disp[[steps // 2 - 1, steps - 1], 0], 0., atol=5e-3)
                 and np.allclose(sim_disp[[steps // 2 - 1, steps - 1], 1], -1., atol=5e-3)
                 and np.max(np.abs(np.diff(theta))) < 0.25
                 and abs(theta[-1] - 4 * np.pi) < 0.1)
except Exception as e:
  print(e)

if test_passed:
  print(colored("STATIC TEST 6 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/beam/test6/result.pdf")
  plt.savefig("tests/beam6_result.png")
else:
  print(colored("STATIC TEST 6 FAILED", "red", attrs=["bold"]))
  sys.exit(1)
