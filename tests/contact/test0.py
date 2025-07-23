#!/usr/bin/python3

# two beams crossed in an X shape

import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/contact/test0/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/contact/test0/resp.csv", delimiter=',')

  plt.figure(figsize=(16/3, 6))

  plt.plot(np.sqrt(sim_resp[:, 0]**2 + sim_resp[:, 1]
           ** 2 + sim_resp[:, 2]**2), label="F_N (sim)")

  plt.legend()
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("CONTACT TEST 0 RUN THROUGH", "green"))

  plt.tight_layout()
  plt.savefig("tests/contact/test0/result.pdf")
else:
  print(colored("CONTACT TEST 0 FAILED", "red", attrs=["bold"]))
