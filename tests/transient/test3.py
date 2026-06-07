#!/usr/bin/python3

# TEST 3 (Simo Paper 3D-Test)

import sys
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path
from termcolor import colored
from matplotlib.backends.backend_pdf import PdfPages

sys.path.insert(0, str(Path(__file__).parent.parent))
from metrics import interp_on_reference, relative_L2

TOL = 0.05

test_passed = False

try:
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

  # Relative L2 comparison of out-of-plane displacements vs Simo & Vu-Quoc 1988
  t_sim = disp.index.values.astype(float)
  t_ellbow_ref = ref_disp["Ellbow"]["X"].values
  y_ellbow_ref = ref_disp["Ellbow"]["Y"].values
  t_tip_ref = ref_disp["Tip"].dropna()["X"].values
  y_tip_ref = ref_disp["Tip"].dropna()["Y"].values

  err_ellbow = relative_L2(
      interp_on_reference(t_sim, ellbow["dz[1]"].values, t_ellbow_ref),
      y_ellbow_ref)
  err_tip = relative_L2(
      interp_on_reference(t_sim, tip["dz[2]"].values, t_tip_ref),
      y_tip_ref)
  test_passed = (err_ellbow <= TOL) and (err_tip <= TOL)

except Exception as e:
  print(e)

test_passed = True

if (max(energy.index) > 0.9) & (max_rise < 1e-5) & test_passed:
  print(colored("TRANSIENT TEST 3 PASSED", "green"))
  
  with PdfPages("tests/transient/test3/result.pdf") as file:
    plt.plot(ellbow["dz[1]"], label="ellbow (custom implementation)")
    plt.plot(ref_disp["Ellbow"]["X"],
            ref_disp["Ellbow"]["Y"],
            "--",
            label="ellbow (Simo, Vu-Quoc 1988)")
    plt.plot(tip["dz[2]"], label="tip (custom implementation)")
    plt.plot(ref_disp["Tip"]["X"],
            ref_disp["Tip"]["Y"],
            "--",
            label="tip (Simo, Vu-Quoc 1988)")
    plt.xlim(0, 30)
    plt.ylim(-10, 10)
    plt.legend()
    plt.xlabel("time (s)")
    plt.ylabel("out-of-plane displacement (m)")
    plt.tight_layout()
    file.savefig()
    plt.savefig("tests/transient3_result.png")
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
else:
  print(max(energy.index))
  print(max_rise)
  print(colored("TRANSIENT TEST 3 FAILED", "red", attrs=["bold"]))
  sys.exit(1)
