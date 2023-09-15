#!/usr/bin/python3

# TEST 1 from Smirit et al. (axial stretch under twist, no hardening)
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/plastic/test2a/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/plastic/test2a/resp.csv", delimiter=',')
  ref_data = np.loadtxt("tests/plastic/ref_data/test2a_ref.csv", delimiter=',')

  plt.plot(sim_disp[:, 1])
  plt.axhline(0, alpha=.2, color="k", lw=.5)
  plt.savefig("test.png")

  plt.figure(figsize=(16/3, 6))
  plt.plot(sim_disp[:, 1], sim_resp[:, 1]/10, label="(custom implementation)")
  plt.plot(ref_data[:, 0], ref_data[:, 1], ":", label="(Smriti et al.)")
  plt.legend(loc="lower right")
  plt.xlabel("Axial strain")
  plt.ylabel("Axial load (per yield limit)")
  plt.axhline(0, alpha=.2, color="k", lw=.5)
  plt.axhline(1, alpha=.2, color="k", lw=.5)
  plt.axhline(-1, alpha=.2, color="k", lw=.5)
  plt.axvline(0, alpha=.2, color="k", lw=.5)
  plt.ylim(-1.5, 1.5)
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("PLASTIC TEST 2a RUN THROUGH", "green"))

  plt.tight_layout()
  plt.savefig("tests/plastic/test2a/result.pdf")
else:
  print(colored("PLASTIC TEST 2a FAILED", "red", attrs=["bold"]))
