#!/usr/bin/python3

# TEST 0 (Wave propagation after Dirac Pulse)

import vtkmodules.all as vtk
from vtk.util.numpy_support import vtk_to_numpy
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
import numpy as np

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

for node in plotx:
  plotx[node] = np.array([disp_data[time][node,-1] for time in ploty]) 
  wave_start = np.flatnonzero(plotx[node]>1e-3)
  plotx[node] = plotx[node] + node/(nnodes-1)
  if wave_start.size > 0:
    wave_times[node/(nnodes-1)] = ploty[min(wave_start)]

wave_pos = np.array(list(wave_times.keys()))
wave_times = np.array(list(wave_times.values()))

wave_speed = np.gradient(wave_pos, wave_times) * -1

for node in plotx:
  plt.plot(plotx[node], ploty)
plt.plot(wave_pos, wave_times, "k:")
plt.xlabel("Length [m]")
plt.ylabel("Time [s]")
plt.xlim([0, 1.05])
plt.ylim([0, 1e-3])
plt.title(f"Wave speed approx. {wave_speed.mean():.0f} m/s\nShould be {np.sqrt(205e9/7850):.0f} m/s")
plt.tight_layout()
plt.gca().invert_yaxis()
plt.show()