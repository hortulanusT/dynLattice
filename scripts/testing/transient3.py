#!/usr/bin/python3

# TEST 3 (Simo Paper 3D-Test)

import os
import pandas as pd
import matplotlib.pyplot as plt
from termcolor import colored
from matplotlib.backends.backend_pdf import PdfPages

energy = pd.read_csv("tests/transient/test3/energy.csv",
                     index_col="time")
disp = pd.read_csv("tests/transient/test3/disp.gz", index_col="time")
ref_disp = pd.read_csv("tests/transient/ref_data/test3_ref.csv",
                       header=[0, 1])

en_rise = (energy["E_tot"].diff() > 0) & (energy["load"] == 0)
max_rise = max(
    (energy["E_tot"].diff().values / energy["E_tot"].values)[en_rise])

ellbow = disp[["dx[1]", "dy[1]", "dz[1]"]]
tip = disp[["dx[2]", "dy[2]", "dz[2]"]]

with PdfPages("tests/transient/test3/result.pdf") as file:
  plt.plot(ellbow["dz[1]"], label="ellbow")
  plt.plot(ref_disp["Ellbow"]["X"],
           ref_disp["Ellbow"]["Y"],
           "--",
           label="ellbow (Simo et al. 1988)")
  plt.plot(tip["dz[2]"], label="tip")
  plt.plot(ref_disp["Tip"]["X"],
           ref_disp["Tip"]["Y"],
           "--",
           label="tip (Simo et al. 1988)")
  plt.xlim(0, 30)
  plt.ylim(-10, 10)
  plt.legend()
  plt.tight_layout()

  file.savefig()
  plt.clf()

  plt.plot(energy["E_tot"], label="total")
  plt.plot(energy["E_kin"], label="kinetic")
  plt.plot(energy["E_pot"], label="potential")
  plt.axvline(2, c="k", alpha=.5)

  plt.xlim(0, 30)
  plt.ylim(bottom=0)
  plt.legend()
  plt.tight_layout()
  file.savefig()

if (max(energy.index) > 0.9) & (max_rise < 1e-5):
  print(colored("TRANSIENT TEST 3 RUN THROUGH", "green"))

  all_data = pd.merge(ellbow, tip, on="time")[::10]
  all_data.columns = ["ellbow_x", "ellbow_y",
                      "ellbow_z", "tip_x", "tip_y", "tip_z"]
  all_data.to_csv("tests/transient/test3/all_data.csv")

  ref_disp.columns = ['_'.join(col).strip() for col in ref_disp.columns.values]
  ref_disp.to_csv("tests/transient/test3/ref_disp.csv", index=False)

  for i, step in enumerate([0, 2.45080000e+04, 4.90070000e+04, 7.35040000e+04, 9.80020000e+04, 1.22495000e+05, 1.46993000e+05]):
    os.system(
        f"scripts/translate_view.py tests/transient/test3/visualDisp.msh {int(step)} 1 1")
    os.system(
        f"mv tests/transient/test3/visual_{int(step)}.tikz tests/transient/test3/transient_{int(i*5)}s.tikz")
else:
  print(max(energy.index))
  print(max_rise)
  print(colored("TRANSIENT TEST 3 FAILED", "red", attrs=["bold"]))
