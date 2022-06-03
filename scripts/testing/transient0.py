#!/usr/bin/python3

# TEST 0 (Wave propagation after Dirac Pulse)

from re import A
import vtkmodules.all as vtk
from vtk.util.numpy_support import vtk_to_numpy
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
from matplotlib.cm import get_cmap
import numpy as np
from matplotlib.backends.backend_pdf import PdfPages
from scipy import signal

disp_data = {}
velo_data = {}
acce_data = {}

reader = vtk.vtkXMLUnstructuredGridReader()
root = ET.parse("tests/transient/test0/visual/step.pvd").getroot()
assert root.tag=="VTKFile"
assert root[0].tag=="Collection"

for steptag in root[0]:
  assert steptag.tag=="DataSet"

  reader.SetFileName(f"tests/transient/test0/visual/{steptag.attrib['file']}")
  reader.Update()
  data = reader.GetOutput().GetPointData()
  time = float(steptag.attrib["timestep"])
  disp_data[time] = vtk_to_numpy(data.GetAbstractArray("Displacement"))
  velo_data[time] = vtk_to_numpy(data.GetAbstractArray("Velocity"))
  acce_data[time] = vtk_to_numpy(data.GetAbstractArray("Acceleration"))


ploty = list(disp_data.keys())
nnodes = disp_data[time].shape[0]
plotx = { node: [] for node in range(nnodes) }
wave_times = {}

# permutation = np.concatenate([[0], range(2, nnodes), [1]])

# for time in disp_data:
#   disp_data[time] = disp_data[time][permutation, :]
#   velo_data[time] = velo_data[time][permutation, :]
#   acce_data[time] = acce_data[time][permutation, :]

time_step = np.diff(list(disp_data.keys())).mean()

for node in plotx:
  plotx[node] = np.array([disp_data[time][node,-1] for time in ploty]) 
  wave_start = np.flatnonzero(plotx[node]>1e-3)
  if wave_start.size > 0:
    wave_times[node/(nnodes-1)] = ploty[min(wave_start)]

wave_pos = np.array(list(wave_times.keys()))
wave_times_cal = np.array(list(wave_times.values()))

wave_speed_cal = np.gradient(wave_pos, wave_times_cal) * -1
wave_speed_ana = np.sqrt(205e9/7850)

wave_times_ana = (1-wave_pos)/wave_speed_ana + 100*time_step

fig, ax = plt.subplots(1,1, sharex=True, figsize=(10,12))
for node in plotx:
  ax.plot(plotx[node] + node/(nnodes-1), ploty)
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
end_spec = np.fft.rfft(plotx[nnodes-1])
ipeaks, _ = signal.find_peaks(np.abs(end_spec))
frequencies = np.fft.rfftfreq(plotx[nnodes-1].size, time_step) / 1e3
coloring = get_cmap("inferno")

modes = [[] for _ in range(5)]

with PdfPages("tests/transient/test0/spectrum.pdf") as pdf:
  fig, axs = plt.subplots(2,1, sharex=True, figsize=(10,8))

  for node in plotx:
    spectrum = np.fft.rfft(plotx[node]) / len(plotx[node])
    for i in range(len(modes)):
      modes[i].append(spectrum[ipeaks[i]])
    
    axs[0].plot(frequencies, np.abs(spectrum), c=coloring(node/nnodes), label=f"Node #{node}")
    axs[0].vlines(eigenfreqs, 0, 100, "r", alpha=0.5, lw=0.5)
    axs[0].set_ylabel("Amplitude [m]")
    axs[0].set_xlim(0, 12)
    axs[0].set_ylim(0, 2e-2)
    # axs[0].legend(ncol=3, loc="upper right")

    axs[1].plot(frequencies, np.angle(spectrum, deg=True), c=coloring(node/nnodes), label=f"Node #{node}")
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