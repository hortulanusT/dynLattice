#!/usr/bin/python3

# TEST 3 arc buckling
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/beam/test3/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test3/resp.csv", delimiter=',')

  plt.figure(figsize=(16/3, 6))
  plt.plot(-1*sim_disp[:,0], -1*sim_resp[:,1], label="horizontal")
  plt.plot(-1*sim_disp[:,1], -1*sim_resp[:,1], label="vertical")
  plt.legend()
  plt.xlabel( "displacment [N]" )
  plt.ylabel( "load [N]" )
  plt.xlim( left=0 )
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("STATIC TEST 3 ?????", "yellow"))

  plt.tight_layout()
  plt.savefig("tests/beam/test3/result.pdf")
else:
  print(colored("STATIC TEST 3 FAILED", "red", attrs=["bold"]))