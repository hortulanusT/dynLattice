#!/usr/bin/python3

# TEST 0 aka Simple arc length test
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/beam/test0/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test0/resp.csv", delimiter=',')
  ref_disp = np.loadtxt("tests/beam/test0_ref/disp.csv", delimiter=',')
  ref_resp = np.loadtxt("tests/beam/test0_ref/resp.csv", delimiter=',')

  plt.figure(figsize=(16/3, 3))
  plt.plot(-1*ref_disp[:,1]-0.2, ref_resp[:,1], label="displacment")
  plt.plot(-1*sim_disp[:,1]-0.2, sim_resp[:,1], "--", label="arc-length")
  plt.legend()
  plt.xlabel( "displacment [m]" )
  plt.ylabel( "load [N]" )

  idx_max = sim_resp[:,1].argmax()
  idx_min = sim_resp[:,1].argmin()

  test_passed = idx_min < len(sim_disp[:,1])/2 < idx_max and np.abs(np.interp( 0.2, -1*sim_disp[:,1], sim_resp[:,1])) < 0.1
except:
  pass

if test_passed:
  print(colored("TEST 0 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/beam/test0/result.pdf")
else:
  print(colored("TEST 0 FAILED", "red", attrs=["bold"]))