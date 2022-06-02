#!/usr/bin/python3

# TEST 0 (Wave propagation after Dirac Pulse)

import vtkmodules.all as vtk
from vtk.util.numpy_support import vtk_to_numpy
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
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

for node in plotx:
  plt.plot(plotx[node] + node/(nnodes-1), ploty)
plt.plot(wave_pos, wave_times_cal, "g:", label="calculated wave")
plt.plot(wave_pos, wave_times_ana, "b:", label="predicted wave")
plt.xlabel("Length [m]")
plt.ylabel("Time [s]")
plt.xlim([0, 1.05])
plt.ylim([0, 4e-4])
plt.legend(loc="upper left")
plt.title(f"Wave speed approx. {wave_speed_cal.mean():.0f} m/s\nShould be {wave_speed_ana:.0f} m/s")
plt.gca().invert_yaxis()
plt.tight_layout()
plt.savefig("tests/transient/test0/speed.pdf")
plt.close()

eigenfreqs = wave_speed_ana * np.pi * (2*np.arange(5) + 1)/2 / 1e3

with PdfPages("tests/transient/test0/spectrum.pdf") as pdf:
  fig, axs = plt.subplots(2,1, sharex=True, figsize=(15,15))
  for node in plotx:
    spectrum = np.fft.rfft(plotx[node])
    ipeaks, _ = signal.find_peaks(np.absolute(spectrum))
    frequencies = np.fft.rfftfreq(plotx[node].size, time_step) / 1e3
    
    axs[0].plot(frequencies, np.absolute(spectrum))
    axs[0].vlines(eigenfreqs, 0, 50, "r", alpha=0.5, lw=0.5)
    axs[0].set_ylabel("Amplitude [?]")
    axs[0].set_xlim(0, 25)
    axs[0].set_ylim(0, 50)

    axs[1].plot(frequencies, np.angle(spectrum, deg=True), label=f"Node #{node}")
    axs[1].hlines([-90,0,90], 0, 100e3, alpha=0.5, lw=0.5)
    axs[1].set_xlabel("Frequency [kHz]")
    axs[1].set_ylabel("Phase Angle [deg]")
    axs[1].set_ylim(-180, 180)
    axs[1].legend()

  fig.suptitle(f"Spectra for the nodes")
  fig.tight_layout(rect=[0,0,1,0.95])
  pdf.savefig(fig)
  plt.close(fig)