#!/usr/bin/python3

from math import radians
import sys, subprocess, datetime, shutil, pathlib
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
np.seterr(divide='ignore')
now = datetime.datetime.now()

thickness_rods = 0.75e-3
thickness_joint = np.linspace(0.75e-3, 2.25e-3, num=7).round(6)
width = 8.25e-3
alpha = 55
l_ratio = (3 / np.sin(radians(55))) / 8.25 # 0.44391803227689314
depth = 1

origin = "studies"
target = "re-entrant_radiusSanne"

study = pathlib.Path(origin).joinpath(target)
input = study.joinpath("input")
output = study.joinpath("output")
results = study.joinpath("results")

subprocess.run(["make", "opt"])

if len(sys.argv)<=1 or sys.argv[1] == "sim":
  print("SIMULATING")

  for folder in [output, results]:   
    try:
      shutil.rmtree(folder)
    except FileNotFoundError:
      pass
    finally:
      folder.mkdir()
      folder.joinpath(".gitkeep").touch()

  with open(f"{study}/run.info", "w") as f:
    f.writelines([
      "Simulation of RUC in the linear regime for testing purposes\n",
      "conducted on " + now.strftime("%Y-%d-%m %H:%M:%S") + "\n",
      "with git commit " + subprocess.check_output(["git", "rev-parse", "HEAD"]).decode(sys.stdout.encoding).strip() + "\n",
      "data in the csv files describes H - the (material) displacment gradient and P - the 1st Piola Kirchhoff tensor"
    ])


  for thick in thickness_joint:      
    area_rods = thickness_rods * depth
    area_moment_rods = area_rods * thickness_rods * thickness_rods / 12
    area_joint = thick * depth
    area_moment_joint = area_joint * thick * thick / 12

    for dir in [ "11", "12", "21", "22" ]:
      subprocess.run(["./bin/nonlinRod-opt"
        , "-p", f"params.material_rod.area={area_rods}"
        , "-p", f"params.material_rod.area_moment={area_moment_rods}"
        , "-p", f"params.material_joints.area={area_joint}"
        , "-p", f"params.material_joints.area_moment={area_moment_joint}"
        , "-p", f"params.load.model.dispGrad{dir}=-1."
        , f"{study}.pro"
        , "-p", f"log.file=\"{output}/report_H{dir}_joint{thick}.log\""
        , "-p", f"Output.paraview.output_format=\"{output}/visual_H{dir}/joint{thick}_step%d\""
        , "-p", f"Output.pbcOut.sampling.file=\"{output}/results_H{dir}_joint{thick}.csv\""
        ]
      , stdout=subprocess.DEVNULL)


if len(sys.argv)<=1 or sys.argv[1] == "plot":
  print("PLOTTING")

  pdColl = {}
  pdColl["11"] = {}
  pdColl["12"] = {}
  pdColl["21"] = {}
  pdColl["22"] = {}
  for dir in [ "11", "12", "21", "22" ]:
    for thick in thickness_joint:
      pdColl[dir][thick] = pd.read_csv(f"{output}/results_H{dir}_joint{thick}.csv")
      pdColl[dir][thick]["strain"] = pdColl[dir][thick][f"H{dir}"] * -1
      pdColl[dir][thick].set_index("strain", inplace=True)
    pdColl[dir] = pd.concat(pdColl[dir].values(), keys=pdColl[dir].keys())
  pdColl = pd.concat(pdColl.values(), keys=pdColl.keys(), names=["load_dir", "joint_thick", "lambda"])

  pdColl["E_x"]  = pdColl["P11"] / pdColl["H11"] / depth	
  pdColl["nu_x"] = pdColl["H22"] / pdColl["H11"] * -1
  pdColl["E_y"]  = pdColl["P22"] / pdColl["H22"] / depth
  pdColl["nu_y"] = pdColl["H11"] / pdColl["H22"] * -1 
  pdColl["G_xy"] = pdColl["P12"] / pdColl["H12"] / depth
  pdColl["G_yx"] = pdColl["P21"] / pdColl["H21"] / depth

  fig, axs = plt.subplots(3, 2, sharex=True, figsize=(12,12))

  axs[0,0].set_ylabel(r"$E_x\ [Pa]$")
  axs[0,1].set_ylabel(r"$E_y\ [Pa]$")
  axs[1,0].set_ylabel(r"$\nu_{xy}$")
  axs[1,1].set_ylabel(r"$\nu_{yx}$")
  axs[2,0].set_ylabel(r"$G_{xy}\ [Pa]$")
  axs[2,1].set_ylabel(r"$G_{yx}\ [Pa]$")

  for thick in thickness_joint:
    axs[0,0].plot(pdColl.loc[("11", thick)]["E_x"], label=f"{thick*1e3} mm")
    axs[0,1].plot(pdColl.loc[("22", thick)]["E_y"], label=f"{thick*1e3} mm")
    axs[1,0].plot(pdColl.loc[("11", thick)]["nu_x"], label=f"{thick*1e3} mm")
    axs[1,1].plot(pdColl.loc[("22", thick)]["nu_y"], label=f"{thick*1e3} mm")
    axs[2,0].plot(pdColl.loc[("12", thick)]["G_xy"], label=f"{thick*1e3} mm")
    axs[2,1].plot(pdColl.loc[("21", thick)]["G_yx"], label=f"{thick*1e3} mm")
  
  for j in range(2):
    for i in range(3):
      axs[i,j].legend() 
      axs[i,j].grid()
    axs[2,j].set_xlabel("strain")

  fig.tight_layout()
  fig.savefig(f"{results}/radiusComp.pdf")