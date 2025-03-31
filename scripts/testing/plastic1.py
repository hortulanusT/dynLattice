#!/usr/bin/python3

# TEST 1 from Smirit et al. (axial stretch under twist, no hardening)
import numpy as np
from termcolor import colored
from matplotlib import pyplot as plt
import pandas as pd
from scipy.integrate import cumulative_trapezoid

test_passed = False

try:
  sim_disp = np.loadtxt("tests/plastic/test1/disp.csv", delimiter=',')
  sim_resp = np.loadtxt("tests/plastic/test1/resp.csv", delimiter=',')
  ref_data = np.loadtxt("tests/plastic/ref_data/test1_ref.csv", delimiter=',')

  plt.figure(figsize=(16/3, 6))
  plt.plot(sim_disp[:, 1], sim_resp[:, 1]/10, label="(custom)")
  plt.plot(ref_data[:, 0]*1e-3, ref_data[:, 1], ":", label="(Smriti et al.)")
  plt.legend(loc="lower right")
  plt.xlabel("Axial strain")
  plt.ylabel("Axial load (per yield limit)")
  plt.axhline(0, alpha=.2, color="k", lw=.5)
  plt.axhline(1, alpha=.2, color="k", lw=.5)
  plt.axhline(-1, alpha=.2, color="k", lw=.5)
  plt.axvline(0, alpha=.2, color="k", lw=.5)
  plt.xlim(-6e-3, 6e-3)
  plt.ylim(-1.1, 1.1)
except Exception as e:
  print(e)
else:
  test_passed = True

if test_passed:
  print(colored("PLASTIC TEST 1 RUN THROUGH", "green"))

  plt.tight_layout()
  plt.savefig("tests/plastic/test1/result.pdf")

  energy = pd.read_csv('tests/plastic/test1/energy.csv', skipinitialspace=True)
  disp = pd.read_csv('tests/plastic/test1/disp.csv',
                     header=None, skipinitialspace=True)
  resp = pd.read_csv('tests/plastic/test1/resp.csv',
                     header=None, skipinitialspace=True)

  input_energy = cumulative_trapezoid(
      resp[1], disp[1], initial=0) + cumulative_trapezoid(resp[4], disp[4], initial=0) + 0.5 * resp[4][0] * disp[4][0]

  plt.plot(input_energy, label='integration')
  plt.plot(energy['E_pot']+energy['E_diss'], '--', label='sim tot')
  plt.plot(energy['E_pot'], label='sim pot')
  plt.plot(energy['E_diss'], label='sim diss')
  plt.plot(input_energy-energy['E_pot'], '--', label='calc diss')
  plt.legend()
  plt.tight_layout()
  plt.savefig('tests/plastic/test1/energy.pdf')

  pd.DataFrame({"disp": disp[1], "resp": resp[1]}).to_csv(
      "tests/plastic/test1/plastic_benchmark.csv", index=False)
else:
  print(colored("PLASTIC TEST 1 FAILED", "red", attrs=["bold"]))
