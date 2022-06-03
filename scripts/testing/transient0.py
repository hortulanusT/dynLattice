#!/usr/bin/python3

# TEST 0 (Wave propagation after Dirac Pulse)

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.cm import get_cmap
from matplotlib.backends.backend_pdf import PdfPages
from scipy import signal

data = pd.read_csv("tests/transient/test0/stateVectors.csv", index_col="time")
data[f"dz{data.shape[1]}"] = data["dz[1]"]
data.drop(columns=["step", "state", "dz[1]"], inplace=True)
data.columns = np.arange(data.shape[1])

times = data.index.to_numpy()
time_step = np.diff(times).mean()
nnodes = data.shape[1]
wave_pos = []
wave_time = []

for node in data:
  test = data[node]>1e-3
  if any(test):
    wave_time.append(data[node][test].index[0])
    wave_pos.append(node/(nnodes-1))

wave_pos = np.array(wave_pos)
wave_times_cal = np.array(wave_time)

wave_speed_cal = np.gradient(wave_pos, wave_times_cal) * -1
wave_speed_ana = np.sqrt(205e9/7850)

wave_times_ana = (1-wave_pos)/wave_speed_ana + 100*time_step

fig, ax = plt.subplots(1,1, sharex=True, figsize=(10,10))
for node in data:
  ax.plot(data[node] + node/(nnodes-1), times)
ax.plot(wave_pos, wave_times_cal, "k:", label="resulting wave")
ax.plot(wave_pos, wave_times_ana, "k", lw=3, alpha=0.2, label="analytical wave")
ax.set_xlabel("Position [m]")
ax.set_ylabel("Time [s]")
ax.set_xlim([0, 1.05])
ax.set_ylim([0, 6e-4])
ax.legend(loc="upper left")
ax.set_title(f"Wave speed approx. {wave_speed_cal.mean():.0f} m/s\nShould be {wave_speed_ana:.0f} m/s")
ax.invert_yaxis()
fig.tight_layout()
fig.savefig("tests/transient/test0/speed.pdf")

eigenfreqs = wave_speed_ana * np.pi * (2*np.arange(5) + 1)/2 / 1e3
end_spec = np.fft.rfft(data[node])
ipeaks, _ = signal.find_peaks(np.abs(end_spec))
frequencies = np.fft.rfftfreq(data.shape[0], time_step) / 1e3
coloring = get_cmap("inferno")

modes = [[] for _ in range(5)]

with PdfPages("tests/transient/test0/spectrum.pdf") as pdf:
  fig, axs = plt.subplots(2,1, sharex=True, figsize=(10,8))

  for node in data:
    spectrum = np.fft.rfft(data[node]) / data[node].size
    for i in range(len(modes)):
      modes[i].append(spectrum[ipeaks[i]])
    
    axs[0].plot(frequencies, np.abs(spectrum), c=coloring(node/(nnodes-1)), label=f"Node #{node}")
    axs[0].vlines(eigenfreqs, 0, 100, "r", alpha=0.5, lw=0.5)
    axs[0].set_ylabel("Amplitude [m]")
    axs[0].set_xlim(0, 12)
    axs[0].set_ylim(0, 2e-2)
    # axs[0].legend(ncol=3, loc="upper right")

    axs[1].plot(frequencies, np.angle(spectrum, deg=True), c=coloring(node/(nnodes-1)), label=f"Node #{node}")
    axs[1].hlines([-90,0,90], 0, 100e3, alpha=0.5, lw=0.5)
    axs[1].set_xlabel("Frequency [kHz]")
    axs[1].set_ylabel("Phase Angle [deg]")
    axs[1].set_ylim(-180, 180)

  fig.suptitle(f"Bode Plot of Node Spectra")
  fig.tight_layout(rect=[0,0,1,0.95])
  pdf.savefig(fig)

  fig, axs = plt.subplots(2,1, sharex=True, figsize=(10,8))

  for i in range(len(modes)):
    axs[0].plot(np.linspace(0,1,nnodes), np.abs(modes[i]), label=f"Mode {i} at {frequencies[ipeaks[i]]:.2f} kHz")
    axs[1].plot(np.linspace(0,1,nnodes), np.angle(modes[i], deg=True), label=f"Mode {i} at {frequencies[ipeaks[i]]:.2f} kHz")
  
  axs[0].hlines(0, 0, 1, alpha=0.5, lw=0.5)
  axs[1].hlines([-90,0,90], 0, 1, alpha=0.5, lw=0.5)
  
  axs[0].set_xlim(0,1)
  axs[0].set_ylim(0, 2e-2)
  axs[1].set_ylim(-180, 180)

  axs[0].set_ylabel("Amplitude [m]")
  axs[1].set_ylabel("Phase Angle [deg]")
  axs[1].set_xlabel("Position [m]")

  axs[0].legend()

  fig.suptitle(f"Amplitude/Phase of Eigenmodes")
  fig.tight_layout(rect=[0,0,1,0.95])
  pdf.savefig(fig)