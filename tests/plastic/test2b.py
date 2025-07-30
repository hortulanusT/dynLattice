#!/usr/bin/python3

# TEST 2b from Smirit et al. (axial stretch kinematic hardening)
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/plastic/test2b/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/plastic/test2b/resp.csv", delimiter=',')
  ref_data = np.loadtxt("tests/plastic/ref_data/test2b_ref.csv", delimiter=',')

  plt.figure(figsize=(16/3, 6))
  plt.plot(sim_disp[:, 1], sim_resp[:, 1]/10, label="custom implementation")
  plt.plot(ref_data[:, 0], ref_data[:, 1], ":", label="Smriti et al. (2021)")
  plt.legend(loc="lower right")
  plt.xlabel("Axial strain")
  plt.ylabel("Axial load (per yield limit)")
  plt.axhline(0, alpha=.2, color="k", lw=.5)
  plt.axhline(1, alpha=.2, color="k", lw=.5)
  plt.axhline(-1, alpha=.2, color="k", lw=.5)
  plt.axvline(0, alpha=.2, color="k", lw=.5)
  plt.xlim(-6e-3, 6e-3)
  plt.ylim(-1.5, 1.5)

  plt.title("b) kinematic hardening")
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("PLASTIC TEST 2b RUN THROUGH", "green"))

  plt.tight_layout()
  plt.savefig("tests/plastic/test2b/result.pdf")
  plt.savefig("tests/plastic2b_result.png")
else:
  print(colored("PLASTIC TEST 2b FAILED", "red", attrs=["bold"]))
