#!/usr/bin/python3

# TEST 3 (Simo Paper 3D-Test)

import pandas as pd
import matplotlib.pyplot as plt
from termcolor import colored

energy = pd.read_csv("tests/transient/test3/energy.csv",
                     index_col="time")
disp = pd.read_csv("tests/transient/test3/disp.gz", index_col="time")
ref_disp = pd.read_csv("tests/transient/ref_data/test3_ref.csv",
                       header=[0, 1])

en_rise = (energy["E_tot"].diff() > 0) & (energy["load"] == 0)
max_rise = max(
    (energy["E_tot"].diff().values / energy["E_tot"].values)[en_rise])

if (max(energy.index) > 0.9) & (max_rise < 1e-5):
  ellbow = disp[["dx[1]", "dy[1]", "dz[1]"]]
  tip = disp[["dx[2]", "dy[2]", "dz[2]"]]

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

  plt.savefig("tests/transient/test3/result.pdf")
  print(colored("TRANSIENT TEST 3 RUN THROUGH", "green"))
else:
  print(colored("TRANSIENT TEST 3 FAILED", "red", attrs=["bold"]))
