#!/usr/bin/python3

# TEST 1 (Ex 5.1 from dynamic Simo Paper)
import numpy as np
import pandas as pd
from termcolor import colored
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

test_passed = False

try:
  sim_disp = np.loadtxt("tests/transient/test1/disp.gz", delimiter=",")
  BC_ref = np.loadtxt("tests/transient/ref_data/test1_ref_BC.csv", delimiter=",")
  angle_ref = pd.read_csv("tests/transient/ref_data/test1_ref_angle.csv", delimiter=";", decimal=",").values
  u1_ref = pd.read_csv("tests/transient/ref_data/test1_ref_u1.csv", delimiter=";", decimal=",").values
  u2_ref = pd.read_csv("tests/transient/ref_data/test1_ref_u2.csv", delimiter=";", decimal=",").values

  t = sim_disp[:, -1]
  psi = sim_disp[:, -2]

  px = 10 + sim_disp[:, 0]
  py = sim_disp[:, 1]
  rz = sim_disp[:, 5]

  shadow_px = 10*np.cos(psi)
  shadow_py = 10*np.sin(psi)

  delta_vect = np.stack([px-shadow_px, py-shadow_py], -1)
  out_vect = np.stack([np.cos(psi), np.sin(psi)], -1)
  tan_vect = np.stack([-1*np.sin(psi), np.cos(psi)], -1)

  dev_rz = np.rad2deg(psi - rz)
  dev_u1 = (delta_vect*out_vect).sum(axis=1)
  dev_u2 = (delta_vect*tan_vect).sum(axis=1)
except IOError:
  pass
else:
  test_passed = True

if test_passed:
  with PdfPages("tests/transient/test1/result.pdf") as pdf:
    plt.plot( BC_ref[:,0], BC_ref[:,1], label="Simo et al. 1988")
    plt.plot( t, psi, "--", label="Simulation" )
    plt.legend(loc="upper left")
    plt.xlabel("time [s]")
    plt.ylabel("rotation [rad]")
    plt.xlim([min(t), max(t)])
    plt.axhline(alpha=0.5, c="k", lw=0.5)
    plt.tight_layout()
    pdf.savefig()
    plt.close()
    
    plt.plot( angle_ref[:,0], angle_ref[:,1], label="Simo et al. 1988")
    plt.plot( t, dev_rz, "--", label="orientation of the tip" )
    plt.legend(loc="upper right")
    plt.xlabel("time [s]")
    plt.ylabel("rotational deviation [deg]")
    plt.xlim([min(t), max(t)])
    plt.axhline(alpha=0.5, c="k", lw=0.5)
    plt.tight_layout()
    pdf.savefig()
    plt.close()

    plt.plot( u1_ref[:,0], u1_ref[:,1], label="Simo et al. 1988")
    plt.plot( t, dev_u1, "--", label="Simulation")
    plt.legend(loc="lower left")
    plt.xlabel("time [s]")
    plt.ylabel("u1 deviation [m]")
    plt.xlim([min(t), max(t)])
    plt.axhline(alpha=0.5, c="k", lw=0.5)
    plt.tight_layout()
    pdf.savefig()
    plt.close()

    plt.plot( u2_ref[:,0], u2_ref[:,1], label="Simo et al. 1988")
    plt.plot( t, dev_u2 , "--", label="Simulation" )
    plt.legend(loc="lower left")
    plt.xlabel("time [s]")
    plt.ylabel("u2 deviation [m]")
    plt.xlim([min(t), max(t)])
    plt.axhline(alpha=0.5, c="k", lw=0.5)
    plt.tight_layout()
    pdf.savefig()
    plt.close()

  print(colored("TRANSIENT TEST 1 RUN THROUGH", "green"))
else:
  print(colored("TRANSIENT TEST 1 FAILED", "red", attrs=["bold"]))