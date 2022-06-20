#!/usr/bin/python3

# TEST 0 (Wave propagation after Dirac Pulse)

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from termcolor import colored
from matplotlib.cm import get_cmap
from matplotlib.backends.backend_pdf import PdfPages
from scipy import signal

test_passed = False

try:
  data = pd.read_csv("tests/transient/test0/disp.gz", index_col=["time", "state"])
  data[f"dz[{data.shape[1]}]"] = data["dz[1]"]
  data.drop(columns="dz[1]", inplace=True)
  data.columns = np.arange(data.shape[1])

  disp = data.xs(0, level="state")
  velo = data.xs(1, level="state")
except IOError:
  pass
else:
  test_passed = True

times = disp.index.to_numpy()
time_step = np.diff(times).mean()
nnodes = data.shape[1]
wave_pos = []
wave_time = []

energy_in = disp.iloc[100, -1] * 1e10

energy = pd.DataFrame()
energy["potential"] = 0.5 * 205e9*np.pi*0.05**2/ (1/(nnodes-1)) * (disp.diff(axis='columns')**2).sum(axis='columns') 
energy["kinetic"] = 0.5 * 7850*np.pi*0.05**2*(1/(nnodes-1)) * (velo**2).sum(axis='columns') # first and last only half length missing!
energy["sum"] = energy.sum(axis='columns')

for node in data:
  test = disp[node]>2e-3
  if any(test):
    wave_time.append(disp[node][test].index[0])
    wave_pos.append(node/(nnodes-1))

wave_pos = np.array(wave_pos)
wave_times_cal = np.array(wave_time)

wave_speed_cal = np.gradient(wave_pos, wave_times_cal) * -1
wave_speed_ana = np.sqrt(205e9/7850)

wave_times_ana = (1-wave_pos)/wave_speed_ana + nnodes*time_step

eigenfreqs = wave_speed_ana * (2*np.arange(10) + 1) / 4 / 1e3
end_spec = np.fft.rfft(disp[data.shape[1]-1]) / disp[data.shape[1]-1].size
frequencies = np.fft.rfftfreq(disp.shape[0], time_step) / 1e3
ipeaks, _ = signal.find_peaks(np.abs(end_spec))

try:
  for i in range(len(eigenfreqs)):
    test_passed &= np.abs(eigenfreqs[i]-frequencies[ipeaks[i]])/eigenfreqs[i] < 0.025
except:
  test_passed = False

if test_passed:
  with PdfPages("tests/transient/test0/result.pdf") as pdf:
    fig, ax = plt.subplots(1,1, sharex=True, figsize=(10,10))
    for node in disp:
      ax.plot(disp[node].values + node/(nnodes-1), times)
    ax.plot(wave_pos, wave_times_cal, "k:", label="resulting wave")
    ax.plot(wave_pos, wave_times_ana, "k", lw=3, alpha=0.2, label="analytical wave")
    ax.set_xlabel("Position [m]")
    ax.set_ylabel("Time [s]")
    ax.set_xlim([0, 1.05])
    ax.set_ylim([time_step*50, time_step*50 + 5e-4])
    ax.tick_params(bottom=True, top=True, left=True, right=True)
    ax.legend(loc="upper left")
    ax.set_title(f"Wave speed approx. {wave_speed_cal.mean():.0f} m/s\nShould be {wave_speed_ana:.0f} m/s")
    ax.invert_yaxis()
    fig.tight_layout()
    pdf.savefig(fig)

    coloring = get_cmap("inferno")

    modes = [[] for _ in range(5)]

    fig, axs = plt.subplots(2,1, sharex=True, figsize=(10,8))

    for node in disp:
      spectrum = np.fft.rfft(disp[node]) / disp[node].size
      for i in range(len(modes)):
        modes[i].append(spectrum[ipeaks[i]])      
      axs[0].plot(frequencies, np.abs(spectrum), c=coloring(node/(nnodes-1)), label=f"Node #{node}")
      axs[1].plot(frequencies, np.angle(spectrum, deg=True), c=coloring(node/(nnodes-1)), label=f"Node #{node}")

    axs[0].set_ylabel("Amplitude [m]")
    axs[0].set_xlim(0, 20)
    axs[0].set_ylim(0, max(np.abs(end_spec)) * 1.2)
    axs[0].vlines(eigenfreqs, 0, 1, alpha=0.5, lw=0.5)

    axs[1].set_xlabel("Frequency [kHz]")
    axs[1].set_ylabel("Phase Angle [deg]")
    axs[1].set_ylim(-180, 180)
    axs[1].vlines(eigenfreqs, -180, 180, alpha=0.5, lw=0.5)
    axs[1].hlines([-90,0,90], 0, 20, alpha=0.5, lw=0.5)

    fig.suptitle(f"Bode Plot of Node Spectra")
    fig.tight_layout(rect=[0,0,1,0.95])
    pdf.savefig(fig)

    fig, axs = plt.subplots(2,1, sharex=True, figsize=(10,8))

    for i in range(len(modes)):
      axs[0].plot(np.linspace(0,1,nnodes), np.abs(modes[i]), label=f"Mode {i} at {frequencies[ipeaks[i]]:.2f} kHz")
      axs[1].plot(np.linspace(0,1,nnodes), np.angle(modes[i], deg=True), label=f"Mode {i} at {frequencies[ipeaks[i]]:.2f} kHz")
    
    axs[1].hlines([-90,0,90], 0, 1, alpha=0.5, lw=0.5)
    
    axs[0].set_xlim(0,1)
    axs[0].set_ylim(0, max(np.abs(end_spec)) * 1.2)
    axs[1].set_ylim(-180, 180)

    axs[0].set_ylabel("Amplitude [m]")
    axs[1].set_ylabel("Phase Angle [deg]")
    axs[1].set_xlabel("Position [m]")

    axs[0].legend()

    fig.suptitle(f"Amplitude/Phase of Eigenmodes")
    fig.tight_layout(rect=[0,0,1,0.95])
    pdf.savefig(fig)

    fig, ax = plt.subplots(1,1, figsize=(10,6))
    ax.hlines( energy_in, 0, 1, alpha=0.5, lw=0.5, label="insert energy")
    energy.plot.line(ax=ax)
    ax.set_xlim([min(times), max(times)])
    pdf.savefig(fig)

  print(colored("TRANSIENT TEST 0 PASSED", "green"))
else:
  print(colored("TRANSIENT TEST 0 FAILED", "red", attrs=["bold"]))