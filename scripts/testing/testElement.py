#!/usr/bin/python3

import os, subprocess, glob, math, sys
import numpy as np
import pandas as pd
from itertools import product
from termcolor import colored

# MANUAL SETTINGS
sim2D = False
elem_order = [ "lin", "quad", "cub" ] #
elem_dir = [ "x", "y", "z" ] #
load_dir = [ "dx", "dy", "dz", "rx", "ry", "rz" ] # 
load_typ = [  "disp","force" ] #

# cleanup the workshop
try:  
  [os.remove(file) for file in glob.glob("tests/element/*.res")]
  [os.remove(file) for file in glob.glob("tests/element/*.log")]
  [os.remove(file) for file in glob.glob("tests/element/FAILED/*.log")]
  [os.remove(file) for file in glob.glob("tests/element/FAILED/*.res")]
  [os.remove(file) for file in glob.glob("tests/element/DIFF/*.res")]
  [os.remove(file) for file in glob.glob("tests/element/DIFF/*.log")]
  os.remove("tests/element/FAILED/_runs.txt")
except FileNotFoundError:
  pass
try:  
  os.mkdir("tests/element/FAILED")
except FileExistsError:
  pass
try:  
  os.mkdir("tests/element/DIFF")
except FileExistsError:
  pass

# get some settings
nel = sys.argv[1] if len(sys.argv) > 1 else "1"
elem_nodes = [ 2, 3, 4 ]

fixed_groups = {eDir:{} for eDir in elem_dir}
for eDir in elem_dir:
  for lDir in load_dir:
    fixed_groups[eDir][lDir] = []
    for dDir in load_dir:
      if lDir[0] == dDir[0] == "d": # load and displacement translational
        fixed_groups[eDir][lDir].append("fixed" if dDir == lDir or dDir[1] == eDir else "all")
      if lDir[0] == dDir[0] == "r": # load and displacement rotational
        fixed_groups[eDir][lDir].append("fixed" if dDir == lDir else "all")
      if lDir[0] == "d" and dDir[0] == "r": # translational load and rotational displacment
        fixed_groups[eDir][lDir].append("fixed" if dDir[1] != lDir[1] and dDir[1] != eDir else "all")
      if lDir[0] == "r" and dDir[0] == "d": # rotational load and translational displacment
        fixed_groups[eDir][lDir].append("fixed" if dDir[1] != lDir[1] else "all")

y_Dir = {eDir:{} for eDir in ["x","y","z"]}
y_Dir["x"]["x"] = "[0.,1.,0.]"
y_Dir["x"]["y"] = "[0.,0.,1.]"
y_Dir["x"]["z"] = "[0.,1.,0.]"
y_Dir["y"]["x"] = "[0.,0.,1.]"
y_Dir["y"]["y"] = "[0.,0.,1.]"
y_Dir["y"]["z"] = "[1.,0.,0.]"
y_Dir["z"]["x"] = "[0.,1.,0.]"
y_Dir["z"]["y"] = "[1.,0.,0.]"
y_Dir["z"]["z"] = "[0.,1.,0.]"

# prepare some analytical solutions
steps = 100
incr = 1000/steps
forces = [(i+1)*incr for i in range(steps)]
displs = [(i+1)*incr*1e-3 for i in range(steps)]
bending_u = lambda d: 1-(math.sin(d)/d)
bending_v = lambda d: (1-math.cos(d))/d

ideal_disp = {eDir:{} for eDir in ["x","y","z"]}
ideal_resp = {}

# FIXME find some expressions for point force bending
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
test_passed = pd.DataFrame(columns=[lTyp+"_"+lDir for lTyp, lDir in product(load_typ, load_dir)]
                        , index=[eOrder+"_"+eDir for eOrder, eDir in product(elem_order, elem_dir)])

# prepare the testing
subprocess.run(["make"])
print("\n")
print(colored("> > Testing simple analytical " + ("2D " if sim2D else "") + f"scenarios for {nel} element" + ("s" if int(nel)>1 else "") + " < <", "yellow"))

# Iterate over elements
for eDir in elem_dir:
  for eOrder, eNodes in zip(elem_order, elem_nodes):
    # build the .dat file for this element
    print(colored(f"{eOrder.upper():5}ELEMENT - "+eDir.upper(), "cyan"), end=" ")
    subprocess.call(["./scripts/utils/geo_to_dat.py", "tests/element/test.geo", eDir, "1", "-o", str(eNodes-1), "nel", nel], stdout=subprocess.DEVNULL)

    # iterate over loads
    for lDir in load_dir:
      for lTyp in load_typ:
        print("...", end="", flush=True)
        # prepare the list for this load and run it
        running_list = ["./bin/nonlinRod"
            , "-p", f"params.Incr={incr:f}"
            , "tests/element/test.pro"
            , "-p", f"control.runWhile=\"i<{steps}\""
            , "-p", f"model.model.model.cosseratRod.shape.numPoints={eNodes}"
            , "-p", f"model.model.model.cosseratRod.material_ey={y_Dir[eDir][lDir[1]]}" 
            , "-p", f"model.model.model.{lTyp}.nodeGroups=[\"free\"]"
            , "-p", f"model.model.model.{lTyp}.dofs=[\"{lDir}\"]"
            , "-p", f"model.model.model.{lTyp}.factors=[1.]" ]
        if sim2D:
            running_list.append("-p")
            fixing_string = "[" + ",".join(f'"{fix}"' for fix in fixed_groups[eDir][lDir]) + "]"
            running_list.append(f"model.model.model.fixed.nodeGroups={fixing_string}")
        prog_ret = subprocess.run(running_list, stdout=subprocess.DEVNULL).returncode

        if prog_ret: #nonzero return code == failed execution
          test_passed.at[eOrder+"_"+eDir, lTyp+"_"+lDir] = ">< FAIL ><"
          running_list.append("-p")
          running_list.append("Output.modules+=[\"paraview\"]")
          running_list.append("-p")
          running_list.append("Input.input.file=\"$(CASE_NAME).dummy\"")
          failed_runs.append(" ".join(running_list))
          try:
            os.rename("tests/element/test.log", f"tests/element/FAILED/{eOrder}-{eDir}_{lTyp}-{lDir}.log")
            os.rename("tests/element/test-disp.res", f"tests/element/FAILED/{eOrder}-{eDir}_{lTyp}-{lDir}_disp.res")
            os.rename("tests/element/test-load.res", f"tests/element/FAILED/{eOrder}-{eDir}_{lTyp}-{lDir}_load.res")
            os.rename("tests/element/test-resp.res", f"tests/element/FAILED/{eOrder}-{eDir}_{lTyp}-{lDir}_resp.res")
          except FileNotFoundError:
            pass
          continue
        
        # compare it with desired results
        sim_disp = np.loadtxt("tests/element/test-disp.res")
        sim_resp = np.loadtxt("tests/element/test-resp.res")
        disp_comp = np.allclose(sim_disp, ideal_disp[eDir][lDir], atol = 1e-6, rtol=1e-3)
        load_comp = np.allclose(sim_resp, ideal_resp[lDir], atol = 1e-6, rtol=1e-3)

        if disp_comp and load_comp:
          test_passed.at[eOrder+"_"+eDir, lTyp+"_"+lDir] = "analytical"
        elif np.isnan(ideal_disp[eDir][lDir]).all() and lDir[-1]!=eDir and lDir[0]=="d":
          test_passed.at[eOrder+"_"+eDir, lTyp+"_"+lDir] = "???" 
        elif disp_comp:
          test_passed.at[eOrder+"_"+eDir, lTyp+"_"+lDir] = "_loadDiff_"
          load_diff_runs.append(f"{eOrder}-{eDir}_{lTyp}-{lDir}")
        elif load_comp:
          test_passed.at[eOrder+"_"+eDir, lTyp+"_"+lDir] = "_dispDiff_"
          disp_diff_runs.append(f"{eOrder}-{eDir}_{lTyp}-{lDir}")
        else:
          test_passed.at[eOrder+"_"+eDir, lTyp+"_"+lDir] = "_ALL_Diff_"
          load_diff_runs.append(f"{eOrder}-{eDir}_{lTyp}-{lDir}")
          disp_diff_runs.append(f"{eOrder}-{eDir}_{lTyp}-{lDir}")

        if "Diff" in test_passed.at[eOrder+"_"+eDir, lTyp+"_"+lDir] or \
            "???" == test_passed.at[eOrder+"_"+eDir, lTyp+"_"+lDir]:
          os.rename("tests/element/test.log", f"tests/element/DIFF/{eOrder}-{eDir}_{lTyp}-{lDir}.log")
          os.rename("tests/element/test-disp.res", f"tests/element/DIFF/{eOrder}-{eDir}_{lTyp}-{lDir}_disp.res")
          os.rename("tests/element/test-load.res", f"tests/element/DIFF/{eOrder}-{eDir}_{lTyp}-{lDir}_load.res")
          os.rename("tests/element/test-resp.res", f"tests/element/DIFF/{eOrder}-{eDir}_{lTyp}-{lDir}_resp.res")
          np.savetxt(f"tests/element/DIFF/{eOrder}-{eDir}_{lTyp}-{lDir}_disp-ana.res", ideal_disp[eDir][lDir], fmt="%16.8e")
          np.savetxt(f"tests/element/DIFF/{eOrder}-{eDir}_{lTyp}-{lDir}_resp-ana.res", ideal_resp[lDir], fmt="%16.8e")

    if all(test_passed.loc[eOrder+"_"+eDir] == "analytical"):
      print(colored(" PASSED", "green", attrs=["bold"]))
    else:
      print(colored(" FAILED", "red", attrs=["bold"]))

# finishing
if (test_passed == "analytical").all().all():
  print(colored("> > > ALL TESTS PASSED < < <\t:))", "green"))
  [os.remove(file) for file in glob.glob("tests/element/*.res")]
  [os.remove(file) for file in glob.glob("tests/element/*.log")]
  os.remove("tests/element/test.dat")
  os.rmdir("tests/element/FAILED")
  os.rmdir("tests/element/DIFF")
else:
  print(colored("> > > ONE (OR MORE) TESTS FAILED < < <\t:((\n", "red"))
  [os.remove(file) for file in glob.glob("tests/element/*.res")]
  [os.remove(file) for file in glob.glob("tests/element/*.log")]
  os.rename("tests/element/test.dat", "tests/element/test.dummy")
  print("Overview:\n")
  print(test_passed)

  if len(failed_runs):
    failed_runs = [run.replace("\"", "\\\"") for run in failed_runs]
    failed_runs = [run.replace("<", "\\<") for run in failed_runs]
    failed_runs = [run.replace("$", "\\$") for run in failed_runs]
    failed_runs = [run.replace("(", "\\(") for run in failed_runs]
    failed_runs = [run.replace(")", "\\)") for run in failed_runs]
    with open("tests/element/FAILED/_runs.txt", "w") as f:
      f.writelines("\n".join(failed_runs))
  else:
    os.rmdir("tests/element/FAILED")

  if len(load_diff_runs):
    print("\nDifferent Load Response Runs:\n"+";\t".join(load_diff_runs))
  if len(disp_diff_runs):
    print("\nDifferent Displacement Runs:\n"+";\t".join(disp_diff_runs))
  try:
    os.rmdir("tests/element/DIFF")
  except OSError:
    pass

print("\n")
