#!/usr/bin/python3

import os, subprocess, glob, shutil
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

splits = 15

try:  
  [os.remove(file) for file in glob.glob("tests/beam/test5.dat")]
  [shutil.rmtree(folder) for folder in glob.glob("tests/beam/test*/")]
except FileNotFoundError:
  pass

subprocess.run(["make"])
print("\n")
print(colored("> > Testing different scenarios from [Simo/Vu-Quoc 1986] < <", "yellow"))
test_passed = []

## TEST 1 ( GENERAL FORCE CONTROL )
print(colored("EX 7.1", "cyan"), end=" ")

alpha = np.linspace(0., 2*np.pi, splits, endpoint=False)
run_success = []

for i in range(splits):
  px = np.sin ( alpha[i] )
  pz = np.cos ( alpha[i] )

  run_success.append(not subprocess.call(["./bin/nonlinRod", "tests/beam/test1.pro", "-p", f"model.model.model.force.factors=[{px},{pz}]"], stdout=subprocess.DEVNULL))
  print(".", end="", flush=True)

if all(run_success):
  try:
    sim_disp = np.loadtxt("tests/beam/test1/disp.res")
    sim_resp = np.loadtxt("tests/beam/test1/resp.res")

    ideal_disp = np.array([[0, -1, 0, 4*np.pi*np.sin(alpha[i]), 0, 4*np.pi*np.cos(alpha[i])] for i in range(splits)])
    ideal_resp = np.array([[0,  0, 0, 8*np.pi*np.sin(alpha[i]), 0, 8*np.pi*np.cos(alpha[i])] for i in range(splits)])

    test_passed.append(np.allclose(sim_disp, ideal_disp) and np.allclose(sim_resp, ideal_resp))
  except IOError:
    test_passed.append(False)
else:
  test_passed.append(False)

if test_passed[-1]:
  print(colored(" PASSED", "green", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## TEST 2 ( QUADRATIC ELEMENTS )
print(colored("EX 7.2", "cyan"), end=" ")
print("...", end="", flush=True)

test_passed.append( not subprocess.call(["./bin/nonlinRod", "tests/beam/test2.pro"], stdout=subprocess.DEVNULL) )

try:
  sim_load = np.loadtxt("tests/beam/test2/load.res")
  sim_resp = np.loadtxt("tests/beam/test2/resp.res")

  test_passed[-1] &= np.allclose(sim_load, sim_resp)
except IOError:
  test_passed[-1] = False


if test_passed[-1]:
  print(colored(" RUN THROUGH", "green", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## TEST 5 ( BEND GEOMETRY )
print(colored("EX 7.5", "cyan"), end=" ")
print("...", end="", flush=True)

subprocess.run(["./scripts/utils/geo_to_dat.py", "tests/beam/test5.geo"], stdout=subprocess.DEVNULL)
test_passed.append(not subprocess.call(["./bin/nonlinRod", "tests/beam/test5.pro"], stdout=subprocess.DEVNULL) )

try:
  sim_load = np.loadtxt("tests/beam/test5/load.res")
  sim_resp = np.loadtxt("tests/beam/test5/resp.res")

  test_passed[-1] &= np.allclose(sim_load, sim_resp)
except IOError:
  test_passed[-1] = False

if test_passed[-1]:
  print(colored(" RUN THROUGH", "green", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## POST PROCESSING
try:
  sim_disp = np.loadtxt("tests/beam/test2/disp.res")
  sim_load = np.loadtxt("tests/beam/test2/load.res")
  sim_resp = np.loadtxt("tests/beam/test2/resp.res")

  plt.plot(sim_load[:,-1], sim_disp[:,2], 'k-', label="vertical")
  plt.plot(sim_load[:,-1], sim_disp[:,1]*-1, 'k-.', label="horizontal")
  plt.legend(loc="upper left")
  plt.xlabel( "Tip Load [N]" )
  plt.xlim(left=0)
  plt.ylabel( "Displacement [m] ")
  plt.grid()

  plt.savefig("tests/beam/test2/results.png")
except:
  pass

## finishing
if all(test_passed):
  print(colored("> > > ALL TESTS PASSED < < <\t:))", "green"))
  [shutil.rmtree(folder) for folder in glob.glob("tests/beam/test*/")]
  [os.remove(file) for file in glob.glob("tests/beam/test*.log")]
  [os.remove(file) for file in glob.glob("tests/beam/test5.dat")]
else:
  print(colored("> > > ONE (OR MORE) TESTS FAILED < < <\t:((", "red"))

print("\n")