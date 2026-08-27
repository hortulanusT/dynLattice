#!/usr/bin/python3

# Undeformed geometry/BC schematic for TEST 2 (re-entrant honeycomb impact,
# tests/docs/re-entrant.geo). Reads the first ParaView frame (undeformed
# configuration) since the mesh is too intricate to sketch by hand, unlike
# test1_schematic.py.

from _vtu import read_pieces
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.collections import LineCollection
from matplotlib.patches import FancyArrowPatch

sys.path.insert(0, str(Path(__file__).parent))

CASE_DIR = Path(__file__).parent / "test2"

beams, plate = read_pieces(CASE_DIR / "vis0.vtu")

fig, ax = plt.subplots(figsize=(5, 4))

segments = beams["points"][beams["lines"]][:, :, :2]
ax.add_collection(LineCollection(segments, colors="tab:blue", linewidths=1))
plate_segments = plate["points"][plate["lines"]][:, :, :2]
ax.add_collection(LineCollection(
    plate_segments, colors="tab:gray", linewidths=300))

x_min, y_min = beams["points"][:, :2].min(axis=0)
x_max, y_max = beams["points"][:, :2].max(axis=0)
pad = 0.15 * (y_max - y_min)

# impactor: prescribed initial velocity applied to the (central) "ymax" group
arrow_x = 0.5 * (x_min + x_max)
ax.add_patch(FancyArrowPatch((arrow_x, y_max + pad), (arrow_x, y_max + 0.15 * pad),
                             arrowstyle="-|>", mutation_scale=18, color="k"))
ax.annotate(r"$v_0 = 35$ m/s", (arrow_x + 0.3*pad, y_max + 0.8 * pad),
            ha="left", va="center", fontsize=10)

# compliant support plate below "ymin", modeled as a spring-mass rod
plate_height = np.ptp(plate["points"][:, 1])
ax.annotate("spring-supported\nbase plate", (0, y_min - plate_height - 0.1 * pad),
            ha="center", va="top", color="tab:gray", fontsize=9)

ax.set_xlim(x_min - pad, x_max + pad)
ax.set_ylim(y_min - plate_height - 0.6 * pad, y_max + 1.3 * pad)
ax.set_aspect("equal")
ax.axis("off")
ax.set_title("Test 2 setup: re-entrant honeycomb lattice under impact")

fig.tight_layout()
fig.savefig(CASE_DIR.parent / "test2_schematic.png", dpi=150)
