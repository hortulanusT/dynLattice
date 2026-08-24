#!/usr/bin/python3

# Static geometry/BC schematic for TEST 1 (Ex 5.1 from Simo, Vu-Quoc 1988).
# Standalone: draws the setup from test1.geo/test1.pro without needing a
# simulation run, so it can be regenerated independently as a doc asset.

from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.patches import Arc, Circle, FancyArrowPatch

ROD_LENGTH = 10.0

fig, ax = plt.subplots(figsize=(7, 3.2))

# the rod itself, spanning the "fixed" (x=0) to "free" (x=ROD_LENGTH) ends
ax.plot([0, ROD_LENGTH], [0, 0], color="tab:blue",
        lw=4, solid_capstyle="round", zorder=2)
ax.plot([0], [0], "o", color="k", ms=8, zorder=3)
ax.plot([ROD_LENGTH], [0], "o", color="k", ms=8, zorder=3)

ax.annotate("fixed", (0, 0), xytext=(0, -0.9), ha="center", fontsize=11)
ax.annotate("free", (ROD_LENGTH, 0), xytext=(
    ROD_LENGTH, -0.9), ha="center", fontsize=11)

# curved arrow at the "fixed" end: prescribed rotation rz(t) about the
# out-of-plane axis drives the whole example (see model.model.disp.scaleFunc)
rot_arc = Arc((0, 0), 3.2, 3.2, angle=0, theta1=20,
              theta2=290, color="tab:red", lw=2, zorder=4)
ax.add_patch(rot_arc)
ax.add_patch(FancyArrowPatch((1.6 * 0.34, 1.6 * 0.94), (1.6 * 0.17, 1.6 * 0.98),
                             arrowstyle="-|>", mutation_scale=16, color="tab:red", zorder=4))
ax.annotate(r"prescribed $r_z(t)$", (0, 1.7),
            ha="center", color="tab:red", fontsize=10)

# small coordinate triad at the fixed end (z points out of the page)
ax.annotate("", xy=(1.1, 0), xytext=(0, 0),
            arrowprops=dict(arrowstyle="->", color="k"))
ax.annotate("x", (1.25, 0), va="center", fontsize=10)
ax.annotate("", xy=(0, 1.1), xytext=(0, 0),
            arrowprops=dict(arrowstyle="->", color="k"))
ax.annotate("y", (0, 1.3), ha="center", fontsize=10)
ax.add_patch(Circle((0, 0), 0.06, facecolor="k", zorder=5))
ax.annotate("z", (-0.35, -0.35), ha="center", fontsize=10)

ax.annotate(f"L = {ROD_LENGTH:.0f} m",
            (ROD_LENGTH / 2, 0.4), ha="center", fontsize=10)

ax.set_xlim(-2.2, ROD_LENGTH + 1)
ax.set_ylim(-2.1, 2.3)
ax.set_aspect("equal")
ax.axis("off")
ax.set_title("Test 1 setup: flexible rod spun up by a prescribed base rotation")

fig.tight_layout()
fig.savefig(Path(__file__).parent / "test1_schematic.png", dpi=150)
