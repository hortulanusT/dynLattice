#!/usr/bin/python3

# TEST 1 from Simo/Vu-Quoc
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

test_passed = False

try:
  sim_disp = np.loadtxt("tests/beam/test1/disp.csv", delimiter=",")
  sim_resp = np.loadtxt("tests/beam/test1/resp.csv", delimiter=",")

  sim_log = open("tests/beam/test1/run.log").readlines()
  sim_conv = [];
  sim_conv.append(8*np.pi)
  res_scale = 1.
  for line in sim_log:
    if "module `Solver.solver' : residual scale factor =" in line:
      res_scale = float(line.split('=')[1])
    if f"module `Solver.solver' : iter = {len(sim_conv)}, scaled residual =" in line:
      sim_conv.append(float(line.split('=')[2]) * res_scale)
  ref_conv = [8*np.pi, .425e2, .441e-13]

  plt.figure(figsize=(16/3, 6))
  plt.semilogy(sim_conv, 'x', label="custom_implementation")
  plt.semilogy(ref_conv, '+', label="Simo/Vu-Quoc")
  plt.legend()
  plt.xlabel( "iteration" )
  plt.ylabel( "normed residual" )
  plt.xticks( range(len(sim_conv)) )

  ideal_disp = np.array([0, -1, 0, 0, 0, 4*np.pi])
  ideal_resp = np.array([0,  0, 0, 0, 0, 8*np.pi])

  test_passed = np.allclose(sim_disp, ideal_disp) and np.allclose(sim_resp, ideal_resp)
except:
  pass

if test_passed:
  print(colored("STATIC TEST 1 PASSED", "green"))

  plt.tight_layout()
  plt.savefig("tests/beam/test1/result.pdf")
else:
  print(colored("STATIC TEST 1 FAILED", "red", attrs=["bold"]))