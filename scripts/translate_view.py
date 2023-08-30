#!/usr/bin/python3

import numpy as np
import gmsh
import sys

data_file = sys.argv[1]
load_step = int(sys.argv[2])

gmsh.initialize()

gmsh.open(data_file)
views = gmsh.view.get_tags()

_, data_tags, data, _, _ = gmsh.view.get_model_data(
    views[-1], load_step)

if not data:
  RuntimeError(f"No data for loadstep {load_step}")

fact = 1e3 * 72 / 25.4  # 1m==1e3 mm  | 1 mm ~ 72/25.4 pt
tikz_str = ""

_, el_tags, _ = gmsh.model.mesh.get_elements(1)
new_coords = np.array([np.nan, np.nan, np.nan])
for element in el_tags[0]:
  _, el_nodes, _, _ = gmsh.model.mesh.get_element(element)
  for j, node in enumerate(el_nodes):
    old_coords = new_coords
    new_coords, _, _, _ = gmsh.model.mesh.get_node(node)
    if (old_coords == new_coords).all():
      continue
    idx = np.squeeze(np.where(data_tags == node))
    tikz_str += ('\n' if j == 0 else ' -- ') + \
        f"({(new_coords[0]+data[idx][0])*fact},{(new_coords[1]+data[idx][1])*fact})"

with open(f"{data_file[:-4]}{load_step}.tikz", "w") as file:
  file.write(tikz_str + "\n")

gmsh.finalize()
