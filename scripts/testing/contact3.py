#!/usr/bin/python3

# ---
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/contact/test3/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/contact/test3/resp.csv", delimiter=',')

  plt.figure(figsize=(16/3, 6))
  plt.plot(sim_disp[:, 1], sim_resp[:, 1], label="(custom)")
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("CONTACT TEST 3 RUN THROUGH", "green"))

  plt.tight_layout()
  plt.savefig("tests/contact/test3/result.pdf")
else:
  print(colored("CONTACT TEST 3 FAILED", "red", attrs=["bold"]))
