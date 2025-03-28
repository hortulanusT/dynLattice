#!/usr/bin/python3

# two beams crossed in an X shape

import os
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/contact/test1/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/contact/test1/resp.csv", delimiter=',')

  ref_disp = np.array([0, 0, 1e-9, 1.72e1, 3.43e1, 5.13e1, 6.84e1])

  plt.figure(figsize=(16/3, 6))

  plt.plot(np.sqrt(sim_resp[:, 0]**2 + sim_resp[:, 1]
           ** 2 + sim_resp[:, 2]**2), label="F_N (sim)")
  plt.plot(ref_disp, label="F_N (ref)", linestyle="--")

  plt.legend()
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("CONTACT TEST 1 RUN THROUGH", "green"))

  plt.tight_layout()
  plt.savefig("tests/contact/test1/result.pdf")

  for i in range(7):
    os.system(
        f"scripts/translate_view.py tests/contact/test1/visualDisp.msh {i+1} 1 1")
    os.system(
        f"mv tests/contact/test1/visual_{int(i+1)}.tikz tests/contact/test1/contact_{int(i)}.tikz")
else:
  print(colored("CONTACT TEST 1 FAILED", "red", attrs=["bold"]))
