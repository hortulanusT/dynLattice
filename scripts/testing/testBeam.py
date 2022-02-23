#!/usr/bin/python3

import os, subprocess, glob, shutil
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

splits = 15

try:  
  [shutil.rmtree(folder) for folder in glob.glob("tests/beam/test*/")]
except FileNotFoundError:
  pass

subprocess.run(["make"])
print("\n")
print(colored("> > Testing different scenarios from [Simo/Vu-Quoc 1986] < <", "yellow"))
test_passed = []

## TEST 2 ( Simple Arc Length )
print(colored("EX 7.0", "cyan"), end=" ")
print("...", end="", flush=True)

test_passed.append( not subprocess.call(["./bin/nonlinRod", "tests/beam/test0.pro"], stdout=subprocess.DEVNULL) )

try:
  sim_resp = np.loadtxt("tests/beam/test0/resp.res", delimiter=",")
  sim_disp = np.loadtxt("tests/beam/test0/disp.res", delimiter=",")

  idx_max = sim_resp[:,1].argmax()
  idx_min = sim_resp[:,1].argmin()

  test_passed[-1] = idx_min < len(sim_disp[:,1])/2 < idx_max and np.abs(np.interp( 0.2, -1*sim_disp[:,1], sim_resp[:,1])) < 0.1
except IOError:
  test_passed[-1] = False


if test_passed[-1]:
  print(colored(" PASSED", "green", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## TEST 1 ( GENERAL FORCE CONTROL )
print(colored("EX 7.1", "cyan"), end=" ")

alpha = np.linspace(0., 2*np.pi, splits, endpoint=False)
run_success = []

for i in range(splits):
  px = np.sin ( alpha[i] )
  pz = np.cos ( alpha[i] )

  run_success.append(not subprocess.call(
    ["./bin/nonlinRod", "tests/beam/test1.pro"
      , "-p", f"model.model.model.force.factors=[{px},{pz}]"
      , "-p", "Output.disp.append=true"
      , "-p", "Output.resp.append=true"]
    , stdout=subprocess.DEVNULL))
  print(".", end="", flush=True)

if all(run_success):
  try:
    sim_disp = np.loadtxt("tests/beam/test1/disp.res", delimiter=",")
    sim_resp = np.loadtxt("tests/beam/test1/resp.res", delimiter=",")

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

# ## TEST 2 ( QUADRATIC ELEMENTS )
print(colored("EX 7.2", "cyan"), end=" ")
print("...", end="", flush=True)

test_passed.append( not subprocess.call(["./bin/nonlinRod", "tests/beam/test2.pro"], stdout=subprocess.DEVNULL) )

try:
  sim_resp = np.loadtxt("tests/beam/test2/resp.res", delimiter=",")
except IOError:
  test_passed[-1] = False


if test_passed[-1]:
  print(colored(" RUN THROUGH", "green", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## TEST 3 ( BEND GEOMETRY )
print(colored("EX 7.3", "cyan"), end=" ")
print("...", end="", flush=True)

test_passed.append(not subprocess.call(["./bin/nonlinRod", "tests/beam/test3.pro"], stdout=subprocess.DEVNULL) )

try:
  sim_resp = np.loadtxt("tests/beam/test3/resp.res", delimiter=",")
except IOError:
  test_passed[-1] = False

if test_passed[-1]:
  print(colored(" RUN THROUGH", "green", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## TEST 4 ( ARC LENGTH CONTROL )
print(colored("EX 7.4", "cyan"), end=" ")
print("...", end="", flush=True)

test_passed.append(not subprocess.call(["./bin/nonlinRod", "tests/beam/test4.pro"], stdout=subprocess.DEVNULL) )

try:
  sim_resp = np.loadtxt("tests/beam/test4/resp.res", delimiter=",")
except IOError:
  test_passed[-1] = False

if test_passed[-1]:
  print(colored(" RUN THROUGH", "green", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## TEST 5 ( BEND GEOMETRY )
print(colored("EX 7.5", "cyan"), end=" ")
print("...", end="", flush=True)

test_passed.append(not subprocess.call(["./bin/nonlinRod", "tests/beam/test5.pro"], stdout=subprocess.DEVNULL) )

try:
  sim_resp = np.loadtxt("tests/beam/test5/resp.res", delimiter=",")
except IOError:
  test_passed[-1] = False

if test_passed[-1]:
  print(colored(" RUN THROUGH", "green", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## POST PROCESSING
try:
  # PLOT TEST 0
  sim_disp = np.loadtxt("tests/beam/test0/disp.res", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test0/resp.res", delimiter=',')

  plt.figure()
  plt.plot(-1*sim_disp[:,1]-0.2, sim_resp[:,1], label="arc-length")
  plt.legend()
  plt.xlabel( "displacment" )
  plt.ylabel( "load" )

  plt.savefig("tests/beam/results0.png")
except:
  pass

try:
  # PLOT TEST 2
  sim_disp = np.loadtxt("tests/beam/test2/disp.res", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test2/resp.res", delimiter=',')

  plt.figure()
  plt.plot(sim_resp[:,-1]/1e3, sim_disp[:,2], label="vertical")
  plt.plot(sim_resp[:,-1]/1e3, -1*sim_disp[:,1], label="horizontal")
  plt.legend(loc="upper left")
  plt.xlabel( "Tip Load [kN]" )
  plt.ylabel( "Displacement [m] ")
  plt.xlim([0, 140])
  plt.axhline(y=0, color="grey")

  plt.savefig("tests/beam/results2.png")
except:
  pass

try:
  # PLOT TEST 3
  sim_disp = np.loadtxt("tests/beam/test3/disp.res", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test3/resp.res", delimiter=',')

  plt.figure()
  plt.plot(-1*sim_disp[:,0], -1*sim_resp[:,1], label="horizontal")
  plt.plot(-1*sim_disp[:,1], -1*sim_resp[:,1], label="vertical")
  plt.legend()
  plt.xlabel( "displacment" )
  plt.ylabel( "load" )

  plt.savefig("tests/beam/results3.png")
except:
  pass

try:
  # PLOT TEST 4
  sim_disp = np.loadtxt("tests/beam/test4/disp.res", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test4/resp.res", delimiter=',')

  plt.figure()
  plt.plot(+1*sim_disp[:,0], -1*sim_resp[:,1], label="horizontal")
  plt.plot(-1*sim_disp[:,1], -1*sim_resp[:,1], label="vertical")
  plt.legend()
  plt.xlabel( "displacment" )
  plt.ylabel( "load" )

  plt.savefig("tests/beam/results4.png")
except:
  pass

try:
  # PLOT TEST 5
  sim_disp = np.loadtxt("tests/beam/test5/disp.res", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test5/resp.res", delimiter=',')

  plt.figure()
  plt.plot(sim_resp[:,2], sim_disp[:,0], label="u_1")
  plt.plot(sim_resp[:,2], sim_disp[:,1], label="u_2")
  plt.plot(sim_resp[:,2], sim_disp[:,2], label="u_3")
  plt.legend()
  plt.xlabel( "load" )
  plt.ylabel( "displacment" )

  plt.savefig("tests/beam/results5.png")
except:
  pass

## finishing
if all(test_passed):
  print(colored("> > > ALL TESTS PASSED < < <\t:))", "green"))
  [os.remove(file) for file in glob.glob("tests/beam/test*.log")]
else:
  print(colored("> > > ONE (OR MORE) TESTS FAILED < < <\t:((", "red"))

print("\n")