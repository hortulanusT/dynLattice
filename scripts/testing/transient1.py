#!/usr/bin/python3

# TEST 1 (Ex 5.1 from dynamic Simo Paper)
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

test_passed = False

try:
  sim_disp = np.loadtxt("tests/transient/test1/disp.csv", delimiter=",")

  t = sim_disp[:, -1]
  psi = sim_disp[:, -2]

  px = 10 + sim_disp[:, 0]
  py = sim_disp[:, 1]
  rz = sim_disp[:, 5]

  sim_psi = np.arctan2( py, px )
  grad_sim = np.diff(sim_psi)
  add = 0
  for i in range(len(grad_sim)):
    sim_psi[i] += add*np.pi
    if grad_sim[i] < -0.5*np.pi:
      add += 2
  sim_psi[-1] += add*np.pi

  sim_u1 = np.sqrt( px**2 + py**2 );

  shadow_px = 10*np.cos(rz)
  shadow_py = 10*np.sin(rz)

  delta_vect = np.stack([shadow_px-px, shadow_py-py], -1)
  out_vect = np.stack([np.cos(rz), np.sin(rz)], -1)

  dev_rz = np.rad2deg(psi - rz)
  dev_psi = np.rad2deg(psi - sim_psi)
  dev_u1 = sim_u1 - 10
  dev_u2 = np.cross(out_vect, delta_vect)
except IOError:
  pass
else:
  test_passed = True

if test_passed:
  print(colored("TEST 1 RUN THROUGH", "green"))

  with PdfPages("tests/transient/test1/result.pdf") as pdf:
    plt.plot( t, psi )
    plt.xlabel("time [s]")
    plt.ylabel("rotation [rad]")
    plt.xlim([min(t), max(t)])
    plt.axhline(alpha=0.5, c="k", lw=0.5)
    plt.tight_layout()
    pdf.savefig()
    plt.close()
    
    plt.plot( t, dev_rz, label="crossection of the tip" )
    plt.plot( t, dev_psi, label="position of the tip" )
    plt.legend()
    plt.xlabel("time [s]")
    plt.ylabel("rotational deviation [deg]")
    plt.xlim([min(t), max(t)])
    plt.axhline(alpha=0.5, c="k", lw=0.5)
    plt.tight_layout()
    pdf.savefig()
    plt.close()

    plt.plot( t, dev_u1 )
    plt.xlabel("time [s]")
    plt.ylabel("u1 deviation [m]")
    plt.xlim([min(t), max(t)])
    plt.axhline(alpha=0.5, c="k", lw=0.5)
    plt.tight_layout()
    pdf.savefig()
    plt.close()

    plt.plot( t, dev_u2 )
    plt.xlabel("time [s]")
    plt.ylabel("u2 deviation [m]")
    plt.xlim([min(t), max(t)])
    plt.axhline(alpha=0.5, c="k", lw=0.5)
    plt.tight_layout()
    pdf.savefig()
    plt.close()
else:
  print(colored("TEST 0 FAILED", "red", attrs=["bold"]))