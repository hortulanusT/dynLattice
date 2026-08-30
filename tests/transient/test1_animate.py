#!/usr/bin/python3

import numpy as np
import meshio
import matplotlib.animation as animation
import matplotlib.pyplot as plt
import xml.etree.ElementTree as ET
from pathlib import Path

import matplotlib
matplotlib.use("Agg")

CASE_DIR = Path(__file__).parent / "test1"
PVD_FILE = CASE_DIR / "visual" / "step.pvd"
OUT_FILE = Path(__file__).parent / "test1_animate.gif"

FPS = 25
FIGSIZE = (7, 4)


def read_pvd(pvd_path):
  root = ET.parse(pvd_path).getroot()
  entries = [
      (float(dataset.get("timestep")), pvd_path.parent / dataset.get("file"))
      for dataset in root.iter("DataSet")
  ]
  entries.sort(key=lambda item: item[0])
  return entries


if not PVD_FILE.exists():
  raise SystemExit(f"{PVD_FILE} not found -- run test1 first")

frames = read_pvd(PVD_FILE)

fig, ax = plt.subplots(figsize=FIGSIZE)
writer = animation.PillowWriter(fps=FPS)

with writer.saving(fig, str(OUT_FILE), dpi=100):
  for time, vtu_path in frames:
    mesh = meshio.read(vtu_path)
    disp = mesh.point_data["Displacement"]
    order = np.argsort(mesh.points[:, 0])
    deformed = mesh.points[order, :2] + disp[order, :2]

    ax.clear()
    ax.plot(deformed[:, 0], deformed[:, 1], "-", color="tab:blue")
    ax.plot(deformed[0, 0], deformed[0, 1], "o", color="k", ms=5)
    ax.plot(deformed[-1, 0], deformed[-1, 1], "o", color="k", ms=5)
    ax.set_xlim(-11, 11)
    ax.set_ylim(-11, 11)
    ax.set_aspect("equal")
    ax.set_title(f"Test 1: t = {time:5.2f} s")
    ax.set_xlabel("x (m)")
    ax.set_ylabel("y (m)")
    fig.tight_layout()

    writer.grab_frame()

plt.close(fig)
