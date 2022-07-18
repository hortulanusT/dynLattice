#!/usr/bin/python3

# TEST 0 (Wave propagation after Dirac Pulse)

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from termcolor import colored
from matplotlib.cm import get_cmap
from matplotlib.backends.backend_pdf import PdfPages
from scipy import signal
np.seterr(invalid='ignore')

test_passed = False

try:
  data = pd.read_csv("tests/transient/test0/disp.gz", index_col=["time", "label"])
  data.columns = pd.MultiIndex.from_tuples([tuple([name[:name.find("[")], 
      int(name[name.find("[")+1:name.find("]")])])\
        for name in data.columns], names=["dof", "node"])
  dx = data["dx"]
  dy = data["dy"]
  dz = data["dz"]
  d_2 = np.sign(dx)*dx**2 + np.sign(dy)*dy**2 + np.sign(dz)*dz**2
  data = np.sign(d_2) * np.sqrt(np.abs(d_2))
   
  data[data.shape[1]] = data[1]
  data.drop(1, axis=1, inplace=True)
  data.columns = np.arange(data.shape[1])

  disp = data.xs("disp", level="label")
  velo = data.xs("velo", level="label")
  acce = data.xs("acce", level="label")
except IOError:
  pass
else:
  test_passed = True

times = disp.index.to_numpy()
time_step = np.diff(times).mean()
nnodes = data.shape[1]
wave_pos = []
wave_time = []

energy_in = disp.iloc[100, -1] * 2e9

energy = pd.DataFrame()
energy["potential"] = 0.5 * 205e9 / (1/(nnodes-1)) * (disp.diff(axis='columns')**2).sum(axis='columns') 
energy["kinetic"] = 0.5 * 7850 *(1/(nnodes-1)) * (velo**2).sum(axis='columns') # first and last only half length missing!
energy["sum"] = energy.sum(axis='columns')

for node in data:
  test = disp[node]>.5e-3
  if any(test):
    wave_time.append(disp[node][test].index[0])
    wave_pos.append(node/(nnodes-1))

wave_pos = np.array(wave_pos)
wave_times_cal = np.array(wave_time)

wave_speed_cal = -1/(nnodes-1) / np.diff(wave_times_cal).mean()
wave_speed_ana = np.sqrt(205e9/7850)

wave_times_ana = (1-wave_pos)/wave_speed_ana + 100*time_step

eigenfreqs = wave_speed_ana * (2*np.arange(5) + 1) / 4 / 1e3
end_spec = np.fft.rfft(disp[data.shape[1]-1]) / disp[data.shape[1]-1].size
frequencies = np.fft.rfftfreq(disp.shape[0], time_step) / 1e3
ipeaks, _ = signal.find_peaks(np.abs(end_spec))

diff_freq = np.zeros_like( eigenfreqs )

try:
  for i in range(len(eigenfreqs)):
    diff_freq[i] = (eigenfreqs[i]-frequencies[ipeaks[i]])/eigenfreqs[i]
    # test_passed &= np.abs(diff_freq[i]) < 0.025
except:
  test_passed = False

if test_passed:
  with PdfPages("tests/transient/test0/result.pdf") as pdf:

    # wave progression
    fig, ax = plt.subplots(1,1, sharex=True, figsize=(10,10))
    for node in disp:
      ax.plot(disp[node].values + node/(nnodes-1), times)
    ax.plot(wave_pos, wave_times_cal, "k:", label="resulting wave")
    ax.plot(wave_pos, wave_times_ana, "k", lw=3, alpha=0.2, label="analytical wave")
    ax.set_xlabel("Position [m]")
    ax.set_ylabel("Time [s]")
    ax.set_xlim([0, 1.05])
    ax.set_ylim([0, max(times)])
    ax.tick_params(bottom=True, top=True, left=True, right=True)
    ax.legend(loc="upper left")
    ax.set_title(f"Wave speed approx. {wave_speed_cal:.0f} m/s\nShould be {wave_speed_ana:.0f} m/s")
    ax.invert_yaxis()
    fig.tight_layout()
    pdf.savefig(fig)

    coloring = get_cmap("inferno")

    modes = [[0.] for _ in range(5)]

    # Node Spectra
    fig, axs = plt.subplots(2,1, sharex=True, figsize=(10,8))

    for node in disp.columns[1:]:
      spectrum = np.fft.rfft(disp[node]) / disp[node].size
      for i in range(len(modes)):
        modes[i].append(spectrum[ipeaks[i]])    
      axs[0].loglog(frequencies, np.abs(spectrum), c=coloring(node/(nnodes-1)), label=f"Node #{node}")
      axs[1].semilogx(frequencies, np.angle(spectrum, deg=True), c=coloring(node/(nnodes-1)), label=f"Node #{node}")

    axs[0].set_ylabel("Amplitude [m]")
    axs[0].set_xlim(right = eigenfreqs[-1])
    axs[0].set_ylim(top = max(np.abs(end_spec)) * 1.2)
    for freq in eigenfreqs:
      axs[0].axvline(freq, alpha=0.5, lw=0.5)

    axs[1].set_xlabel("Frequency [kHz]")
    axs[1].set_ylabel("Phase Angle [deg]")
    axs[1].set_ylim(-180, 180)
    for freq in eigenfreqs:
      axs[1].axvline(freq, alpha=0.5, lw=0.5)
    for ang in [-90,0,90]:
      axs[1].axhline(ang, alpha=0.5, lw=0.5)

    fig.suptitle(f"Bode Plot of Node Spectra")
    fig.tight_layout(rect=[0,0,1,0.95])
    pdf.savefig(fig)

    # eigenmodes
    fig, axs = plt.subplots(2,1, sharex=True, figsize=(10,8))

    loc = np.linspace(0,1,nnodes)
    for i in range(len(modes)):
      axs[0].plot(loc, np.abs(modes[i]), label=f"Mode {i} res at {frequencies[ipeaks[i]]:.2f} kHz")
      axs[1].plot(loc, np.angle(modes[i], deg=True), label=f"{frequencies[ipeaks[i]]:.2f} kHz")
    
    for i in range(len(modes)):
      axs[0].plot(loc, np.abs(np.sin(loc * (2*i+1)/2 * np.pi)) * np.abs(modes[i])[-1], "k:", label=f"vs ana at  {eigenfreqs[i]:.2f} kHz")
      
    for ang in [-90,0,90]:
      axs[1].axhline(ang, alpha=0.5, lw=0.5)
    
    axs[0].set_xlim(0,1)
    axs[0].set_ylim(0, max(np.abs(modes[0])) * 1.1)
    axs[1].set_ylim(-180, 180)

    axs[0].set_ylabel("Amplitude [m]")
    axs[1].set_ylabel("Phase Angle [deg]")
    axs[1].set_xlabel("Position [m]")

    axs[0].legend( ncol=2 )

    fig.suptitle(f"Amplitude/Phase of Eigenmodes")
    fig.tight_layout(rect=[0,0,1,0.95])
    pdf.savefig(fig)

    # energy
    fig, ax = plt.subplots(1,1, figsize=(10,6))
    ax.axhline( energy_in, alpha=0.5, lw=0.5, label="energy input")
    energy.plot.line(ax=ax)
    ax.set_xlim([min(times), max(times)])
    ax.set_ylim([0, energy_in*1.2])
    pdf.savefig(fig)

    # raw spectra
    fig, axs = plt.subplots(5,1, sharex=True, figsize=(10,20))
    for node in disp.columns[1:]:
      spectrum_disp = np.fft.rfft(disp[node]) / disp[node].size
      spectrum_velo = np.fft.rfft(velo[node]) / velo[node].size
      spectrum_acce = np.fft.rfft(acce[node]) / acce[node].size
    
      axs[0].loglog(frequencies, np.abs(spectrum_disp), c=coloring(node/(nnodes-1)), label=f"Node #{node}")
      axs[1].loglog(frequencies, np.abs(spectrum_velo), c=coloring(node/(nnodes-1)), label=f"Node #{node}")
      axs[2].loglog(frequencies, np.abs(spectrum_acce), c=coloring(node/(nnodes-1)), label=f"Node #{node}")
      axs[3].loglog(frequencies, np.abs(spectrum_velo)/np.abs(spectrum_disp), c=coloring(node/(nnodes-1)), label=f"Node #{node}")
      axs[4].loglog(frequencies, np.abs(spectrum_acce)/np.abs(spectrum_disp), c=coloring(node/(nnodes-1)), label=f"Node #{node}")
    
    axs[3].loglog(frequencies, frequencies*np.pi*2e3, "k:", lw=.5, alpha=.5)
    axs[4].loglog(frequencies, frequencies**2*np.pi**2*4e6, "k:", lw=.5, alpha=.5)
     
    axs[0].set_ylabel("Displacements [m]")
    axs[1].set_ylabel("Velocity direct [m/s]")
    axs[2].set_ylabel("Accelaration direct [m/s^2]")
    axs[3].set_ylabel("Velo/Disp factors [1/s]")
    axs[4].set_ylabel("Acce/Disp factors [1/s^2]")
    axs[-1].set_xlabel("Frequency [kHz]")

    fig.suptitle(f"Amplitudes of Node Spectra")
    fig.tight_layout(rect=[0,0,1,0.99])
    pdf.savefig(fig)

  print(colored("TRANSIENT TEST 0 PASSED", "green"))
else:
  print(colored("TRANSIENT TEST 0 FAILED", "red", attrs=["bold"]))