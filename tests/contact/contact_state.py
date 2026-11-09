#!/usr/bin/python3

# Reads the node states of the free rods of contact tests 2 and 3.

import numpy as np
import pandas as pd



def read_state(path, start_a, end_a, start_b, end_b, elements):
  """Position, velocity of the nodes of both rods, along each rod: (times, pos_a, vel_a, pos_b, vel_b).

  gmsh numbers the four end points first, then the interior nodes of rod A and of rod B."""
  data = pd.read_csv(path, index_col=["time", "label"], skipinitialspace=True)
  disp, velo = data.xs("disp", level="label"), data.xs("velo", level="label")
  inner = elements - 1
  order_a = [0] + list(range(4, 4 + inner)) + [1]
  order_b = [2] + list(range(4 + inner, 4 + 2 * inner)) + [3]

  def take(frame, order):
    return np.array([[frame[f"{dof}[{node}]"].to_numpy() for dof in ("dx", "dy", "dz")] for node in order]).transpose(2, 0, 1)

  ref_a = np.linspace(start_a, end_a, elements + 1)
  ref_b = np.linspace(start_b, end_b, elements + 1)
  return (disp.index.to_numpy(), ref_a[None] + take(disp, order_a), take(velo, order_a),
          ref_b[None] + take(disp, order_b), take(velo, order_b))


def linear_momentum(vel, mass_per_length, element):
  """Consistent mass: the momentum of an element is its mass times the mean of the end velocities."""
  return (mass_per_length * element / 2. * (vel[:, :-1] + vel[:, 1:])).sum(axis=1)
