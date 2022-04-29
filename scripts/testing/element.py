#!/usr/bin/python3
import sys, math, sys
import numpy as np
from termcolor import colored

# MANUAL SETTINGS
load_dir = [ "dx", "dy", "dz", "rx", "ry", "rz" ]
load_typ = [  "disp", "force" ] #

# get some settings
nel = int(sys.argv[2]) if len(sys.argv) > 2 else 1
elem_order = sys.argv[1].split("_")[0]
elem_nodes = 2 if elem_order=="lin" else 3 if elem_order=="quad" else 4 if elem_order=="cub" else -1
elem_dir = sys.argv[1].split("_")[1]

# prepare some analytical solutions
steps = 100
incr = 1000/steps
forces = [(i+1)*incr for i in range(steps)]
displs = [(i+1)*incr*1e-3 for i in range(steps)]
bending_u = lambda d: 1-(math.sin(d)/d)
bending_v = lambda d: (1-math.cos(d))/d

ideal_disp = {eDir:{} for eDir in ["x","y","z"]}
ideal_resp = {}

# TODO find some expressions for point force bending
ideal_disp["x"]["dx"] = np.array([[d, 0, 0, 0, 0, 0] for d in displs])
ideal_disp["x"]["dy"] = np.array([[math.nan, math.nan, math.nan, math.nan, math.nan, math.nan] for _ in displs])
ideal_disp["x"]["dz"] = np.array([[math.nan, math.nan, math.nan, math.nan, math.nan, math.nan] for _ in displs])
ideal_disp["x"]["rx"] = np.array([[0, 0, 0, d, 0, 0] for d in displs])
ideal_disp["x"]["ry"] = np.array([[-bending_u(d), 0, -bending_v(d), 0, d, 0] for d in displs])
ideal_disp["x"]["rz"] = np.array([[-bending_u(d),  bending_v(d), 0, 0, 0, d] for d in displs])

ideal_disp["y"]["dx"] = np.array([[math.nan, math.nan, math.nan, math.nan, math.nan, math.nan] for _ in displs])
ideal_disp["y"]["dy"] = np.array([[0, d, 0, 0, 0, 0] for d in displs])
ideal_disp["y"]["dz"] = np.array([[math.nan, math.nan, math.nan, math.nan, math.nan, math.nan] for _ in displs])
ideal_disp["y"]["rx"] = np.array([[0, -bending_u(d),  bending_v(d), d, 0, 0] for d in displs])
ideal_disp["y"]["ry"] = np.array([[0, 0, 0, 0, d, 0] for d in displs])
ideal_disp["y"]["rz"] = np.array([[-bending_v(d), -bending_u(d), 0, 0, 0, d] for d in displs])

ideal_disp["z"]["dx"] = np.array([[math.nan, math.nan, math.nan, math.nan, math.nan, math.nan] for _ in displs])
ideal_disp["z"]["dy"] = np.array([[math.nan, math.nan, math.nan, math.nan, math.nan, math.nan] for _ in displs])
ideal_disp["z"]["dz"] = np.array([[0, 0, d, 0, 0, 0] for d in displs])
ideal_disp["z"]["rx"] = np.array([[0, -bending_v(d), -bending_u(d), d, 0, 0] for d in displs])
ideal_disp["z"]["ry"] = np.array([[ bending_v(d), 0, -bending_u(d), 0, d, 0] for d in displs])
ideal_disp["z"]["rz"] = np.array([[0, 0, 0, 0, 0, d] for d in displs])

ideal_resp["dx"] = np.array([[f, 0, 0, 0, 0, 0] for f in forces])
ideal_resp["dy"] = np.array([[0, f, 0, 0, 0, 0] for f in forces])
ideal_resp["dz"] = np.array([[0, 0, f, 0, 0, 0] for f in forces])
ideal_resp["rx"] = np.array([[0, 0, 0, f, 0, 0] for f in forces])
ideal_resp["ry"] = np.array([[0, 0, 0, 0, f, 0] for f in forces])
ideal_resp["rz"] = np.array([[0, 0, 0, 0, 0, f] for f in forces])

# prepare some lists for storing the results
failed_runs = []
load_diff_runs = []
disp_diff_runs = []
test_runs = {}

# iterate over loads
for lDir in load_dir:
  for lTyp in load_typ:    
    # compare it with desired results
    try:
      sim_disp = np.loadtxt(f"tests/element/runs/{sys.argv[1]}_{lTyp}_{lDir}-disp.csv")
      sim_resp = np.loadtxt(f"tests/element/runs/{sys.argv[1]}_{lTyp}_{lDir}-resp.csv")
    except:
      test_runs[lTyp+"_"+lDir] = ">< FAIL ><"
      continue

    disp_comp = np.allclose(sim_disp, ideal_disp[elem_dir][lDir], atol = 1e-6, rtol=1e-3)
    load_comp = np.allclose(sim_resp, ideal_resp[lDir], atol = 1e-6, rtol=1e-3)

    if disp_comp and load_comp:
      test_runs[lTyp+"_"+lDir] = "analytical"
    elif np.isnan(ideal_disp[elem_dir][lDir]).all() and lDir[-1]!=elem_dir and lDir[0]=="d":
      test_runs[lTyp+"_"+lDir] = "???" 
    elif disp_comp:
      test_runs[lTyp+"_"+lDir] = "_loadDiff_"
    elif load_comp:
      test_runs[lTyp+"_"+lDir] = "_dispDiff_"
    else:
      test_runs[lTyp+"_"+lDir] = "_ALL_Diff_"

    if "Diff" in test_runs[lTyp+"_"+lDir] or "???" == test_runs[lTyp+"_"+lDir]:
      np.savetxt(f"tests/element/runs/{sys.argv[1]}_{lTyp}_{lDir}-disp-ana.csv", ideal_disp[elem_dir][lDir], fmt="%16.8e")
      np.savetxt(f"tests/element/runs/{sys.argv[1]}_{lTyp}_{lDir}-resp-ana.csv", ideal_resp[lDir], fmt="%16.8e")

# finishing
with open(f"tests/element/result_{sys.argv[1]}.txt", "w") as result_file:
  result_file.writelines([f"{run:>10} : {outcome}\n" for run, outcome in test_runs.items()])

if all(outcome=="analytical" or outcome=="???" for outcome in test_runs.values()):
  print(colored(f" All tests for {sys.argv[1]:8} passed", "green"))
elif not any(outcome=="_loadDiff_" or outcome=="_ALL_Diff_" for outcome in test_runs.values()):
  print(colored(f"Some tests for {sys.argv[1]:8} have different displacements", "yellow"))
else:
  print(colored(f" !!! Tests for {sys.argv[1]:8} failed", "red", attrs=["bold"]))