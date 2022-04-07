#!/usr/bin/python3

from cmath import pi
import os, subprocess, glob, shutil
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt

splits = 15

for folder in glob.glob("tests/beam/test*/"):
  try:  
    shutil.rmtree(folder)
  except FileNotFoundError:
    pass

for i in range(6):
  os.mkdir(f"tests/beam/test{i}")

print("\n")
print(colored("> > Testing different scenarios from [Simo/Vu-Quoc 1986] < <", "yellow"))
test_passed = []

## TEST 0 ( Simple Arc Length )
print(colored("EX 7.0", "cyan"), end=" ")
print("...", end="", flush=True)

test_passed.append( not subprocess.call(["./bin/nonlinRod", "tests/beam/test0.pro"
            , "-p", "model.model.model.force=params.force_model_disp;"
            , "-p", "Solver.solver.type=\"Nonlin\";"], stdout=subprocess.DEVNULL) )

os.rename("tests/beam/test0/resp.csv", "tests/beam/test0/resp_ref.csv")
os.rename("tests/beam/test0/disp.csv", "tests/beam/test0/disp_ref.csv")
os.rename("tests/beam/test0/run.log", "tests/beam/test0/run_ref.log")
os.rename("tests/beam/test0/visual", "tests/beam/test0/visual_ref")

test_passed.append( not subprocess.call(["./bin/nonlinRod", "tests/beam/test0.pro"], stdout=subprocess.DEVNULL) )

try:
  sim_resp = np.loadtxt("tests/beam/test0/resp.csv", delimiter=",")
  sim_disp = np.loadtxt("tests/beam/test0/disp.csv", delimiter=",")

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

alpha = np.linspace(0., 2*np.pi, splits, endpoint=False)[::-1]
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
    sim_disp = np.loadtxt("tests/beam/test1/disp.csv", delimiter=",")
    sim_resp = np.loadtxt("tests/beam/test1/resp.csv", delimiter=",")

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
  sim_resp = np.loadtxt("tests/beam/test2/resp.csv", delimiter=",")
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
  sim_resp = np.loadtxt("tests/beam/test3/resp.csv", delimiter=",")
except IOError:
  test_passed[-1] = False

if test_passed[-1]:
  print(colored(" ??????", "yellow", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## TEST 4 ( ARC LENGTH CONTROL )
print(colored("EX 7.4", "cyan"), end=" ")
print("...", end="", flush=True)

test_passed.append(not subprocess.call(["./bin/nonlinRod", "tests/beam/test4.pro"], stdout=subprocess.DEVNULL) )

try:
  sim_resp = np.loadtxt("tests/beam/test4/resp.csv", delimiter=",")
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
  sim_resp = np.loadtxt("tests/beam/test5/resp.csv", delimiter=",")
except IOError:
  test_passed[-1] = False

if test_passed[-1]:
  print(colored(" RUN THROUGH", "green", attrs=["bold"]))
else:
  print(colored(" FAILED", "red", attrs=["bold"]))

## POST PROCESSING
try:
  # PLOT TEST 0
  sim_disp = np.loadtxt("tests/beam/test0/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test0/resp.csv", delimiter=',')
  ref_disp = np.loadtxt("tests/beam/test0/disp_ref.csv", delimiter=',')
  ref_resp = np.loadtxt("tests/beam/test0/resp_ref.csv", delimiter=',')

  plt.figure(figsize=(16/3, 3))
  plt.plot(-1*ref_disp[:,1]-0.2, ref_resp[:,1], label="displacment")
  plt.plot(-1*sim_disp[:,1]-0.2, sim_resp[:,1], "--", label="arc-length")
  plt.legend()
  plt.xlabel( "displacment [m]" )
  plt.ylabel( "load [N]" )

  plt.tight_layout()
  plt.savefig("tests/beam/test0/result_7_0.pdf")
except:
  pass

try:
  # PLOT TEST 1
  sim_log = open("tests/beam/test1/run.log").readlines()
  sim_conv = [];
  sim_conv.append(8*pi)
  res_scale = 1.
  for line in sim_log:
    if "module `Solver.solver' : residual scale factor =" in line:
      res_scale = float(line.split('=')[1])
    if f"module `Solver.solver' : iter = {len(sim_conv)}, scaled residual =" in line:
      sim_conv.append(float(line.split('=')[2]) * res_scale)
  ref_conv = [8*pi, .425e2, .441e-13]

  plt.figure(figsize=(16/3, 6))
  plt.semilogy(sim_conv, 'x', label="custom_implementation")
  plt.semilogy(ref_conv, '+', label="Simo/Vu-Quoc")
  plt.legend()
  plt.xlabel( "iteration" )
  plt.ylabel( "normed residual" )
  plt.xticks( range(len(sim_conv)) )

  plt.tight_layout()
  plt.savefig("tests/beam/test1/result_7_1.pdf")
except:
  pass

try:
  # PLOT TEST 2
  sim_disp = np.loadtxt("tests/beam/test2/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test2/resp.csv", delimiter=',')
  ref_data_vertical = np.loadtxt("tests/beam/ref_data/test2_ref.csv", delimiter=',', skiprows=2, usecols=(0,1))
  ref_data_horizontal = np.loadtxt("tests/beam/ref_data/test2_ref.csv", delimiter=',', skiprows=2, usecols=(2,3), max_rows=96)

  plt.figure(figsize=(16/3, 6))
  plt.plot(sim_resp[:,-1]/1e3, sim_disp[:,2], label="vertical (custom implementation)")
  plt.plot(sim_resp[:,-1]/1e3, -1*sim_disp[:,1], label="horizontal (custom implementation)")
  plt.plot(ref_data_vertical[:,0], ref_data_vertical[:,1], ":", label="vertical (Simo/Vu-Quoc)")
  plt.plot(ref_data_horizontal[:,0], ref_data_horizontal[:,1], ":", label="horizontal (Simo/Vu-Quoc)")
  plt.legend()
  plt.xlabel( "Tip Load [kN]" )
  plt.ylabel( "Displacement [m] ")
  plt.xlim([0, 140])
  plt.axhline( y=0, color="grey" )
  plt.xlim( left=0 )

  plt.tight_layout()
  plt.savefig("tests/beam/test2/result_7_2.pdf")
except:
  pass

try:
  # PLOT TEST 3
  sim_disp = np.loadtxt("tests/beam/test3/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test3/resp.csv", delimiter=',')

  plt.figure(figsize=(16/3, 6))
  plt.plot(-1*sim_disp[:,0], -1*sim_resp[:,1], label="horizontal")
  plt.plot(-1*sim_disp[:,1], -1*sim_resp[:,1], label="vertical")
  plt.legend()
  plt.xlabel( "displacment [N]" )
  plt.ylabel( "load [N]" )
  plt.xlim( left=0 )

  plt.tight_layout()
  plt.savefig("tests/beam/test3/result_7_3.pdf")
except:
  pass

try:
  # PLOT TEST 4
  sim_disp = np.loadtxt("tests/beam/test4/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test4/resp.csv", delimiter=',')
  ref_data_vertical = np.loadtxt("tests/beam/ref_data/test4_ref_vertical.csv", delimiter=',', skiprows=1)
  ref_data_horizontal = np.loadtxt("tests/beam/ref_data/test4_ref_horizontal.csv", delimiter=',', skiprows=1)

  plt.figure(figsize=(16/3, 6))
  plt.plot(+1*sim_disp[:,0], -1*sim_resp[:,1], label="horizontal (custom implementation)")
  plt.plot(-1*sim_disp[:,1], -1*sim_resp[:,1], label="vertical (custom implementation)")
  plt.plot(ref_data_vertical[:,0], ref_data_vertical[:,1]*1e3, ":", label="vertical (Simo/Vu-Quoc)")
  plt.plot(ref_data_horizontal[:,0], ref_data_horizontal[:,1]*1e3, ":", label="horizontal (Simo/Vu-Quoc)")
  plt.legend()
  plt.xlabel( "displacment [m]" )
  plt.ylabel( "load [N]" )
  plt.xlim( left=0 )

  plt.tight_layout()
  plt.savefig("tests/beam/test4/result_7_4.pdf")
except:
  pass

try:
  # PLOT TEST 5
  sim_disp = np.loadtxt("tests/beam/test5/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/beam/test5/resp.csv", delimiter=',')
  ref_data_u1 = np.loadtxt("tests/beam/ref_data/test5_ref.csv", delimiter=',', skiprows=2, usecols=(0,1))
  ref_data_u2 = np.loadtxt("tests/beam/ref_data/test5_ref.csv", delimiter=',', skiprows=2, usecols=(2,3), max_rows=48)
  ref_data_u3 = np.loadtxt("tests/beam/ref_data/test5_ref.csv", delimiter=',', skiprows=2, usecols=(4,5), max_rows=51)

  plt.figure(figsize=(12, 4))
  plt.plot(sim_resp[:,2], sim_disp[:,0], label="u_1 (custom_implementation)")
  plt.plot(sim_resp[:,2], sim_disp[:,1], label="u_2 (custom_implementation)")
  plt.plot(sim_resp[:,2], sim_disp[:,2], label="u_3 (custom_implementation)")
  plt.plot(ref_data_u1[:,0], ref_data_u1[:,1], ":", label="u_1 (Simo/Vu-Quoc)")
  plt.plot(ref_data_u2[:,0], ref_data_u2[:,1], ":", label="u_2 (Simo/Vu-Quoc)")
  plt.plot(ref_data_u3[:,0], ref_data_u3[:,1], ":", label="u_3 (Simo/Vu-Quoc)")
  plt.legend( loc = "upper left")
  plt.xlabel( "load [N]" )
  plt.ylabel( "displacment [m]" )
  plt.xlim( left=0, right=3000 )

  plt.tight_layout()
  plt.savefig("tests/beam/test5/result_7_5.pdf")
except:
  pass

## finishing
if all(test_passed):
  print(colored("> > > ALL TESTS PASSED < < <\t:))", "green"))
else:
  print(colored("> > > ONE (OR MORE) TESTS FAILED < < <\t:((", "red"))

print("\n")