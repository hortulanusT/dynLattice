#!/usr/bin/python3

# Renders the ParaView output of TEST 2 into a GIF for the docs.
# Requires test2 to have been run first (tests/docs/test2/vis*.vtu).

from _vtu import read_pieces, read_pvd
import sys
from pathlib import Path

import matplotlib.animation as animation
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.collections import LineCollection

import matplotlib
matplotlib.use("Agg")

sys.path.insert(0, str(Path(__file__).parent))

CASE_DIR = Path(__file__).parent / "test2"
PVD_FILE = CASE_DIR / "vis.pvd"
OUT_FILE = Path(__file__).parent / "test2_animate.gif"

FPS = 20
FIGSIZE = (5, 4)

if not PVD_FILE.exists():
  raise SystemExit(
      f"{PVD_FILE} not found -- run test2 (tests/docs/test2.pro) first")

frames = read_pvd(PVD_FILE)

# fix the plotted extent to the (undeformed) first frame, since the
# re-entrant/auxetic lattice only shrinks under this compressive impact
beams0, _ = read_pieces(frames[0][1])
x_min, y_min = beams0["points"][:, :2].min(axis=0)
x_max, y_max = beams0["points"][:, :2].max(axis=0)
pad = 0.1 * (x_max - x_min)

fig, ax = plt.subplots(figsize=FIGSIZE)
writer = animation.PillowWriter(fps=FPS)

with writer.saving(fig, str(OUT_FILE), dpi=150):
  for time, vtu_path in frames:
    beams, plate = read_pieces(vtu_path)

    deformed_beams = (
        beams["points"] + beams["point_data"]["Displacement"])[:, :2]
    deformed_plate = (
        plate["points"] + plate["point_data"]["Displacement"])[:, :2]
    # equivalent plastic strain per rod element, used to color plastic hinges
    plast_strain = np.linalg.norm(beams["cell_data"]["plast_strain"], axis=1)

    ax.clear()
    beam_segments = LineCollection(
        deformed_beams[beams["lines"]
                       ], array=plast_strain, cmap="viridis", linewidths=2.5
    )
    ax.add_collection(beam_segments)
    ax.add_collection(LineCollection(
        deformed_plate[plate["lines"]], colors="tab:gray", linewidths=300))

    ax.set_xlim(x_min - pad, x_max + pad)
    ax.set_ylim(y_min - pad, y_max + pad)
    ax.set_aspect("equal")
    ax.set_title(f"Test 2: t = {time * 1e6:6.2f} $\\mu$s")
    ax.set_xlabel("x (m)")
    ax.set_ylabel("y (m)")
    fig.tight_layout()

    writer.grab_frame()

plt.close(fig)
