#!/usr/bin/python3

import numpy as np
import gmsh
import sys
import os
from matplotlib import colormaps

viridis = colormaps["plasma"]

data_file = sys.argv[1]
load_step = int(sys.argv[2])
if len(sys.argv) > 3:
  visual_fact = float(sys.argv[3])
else:
  visual_fact = 1.

gmsh.initialize()

plast_strain = False

gmsh.open(data_file)
if os.path.isfile(data_file.replace("Disp", "plast_strain")):
  gmsh.merge(data_file.replace("Disp", "plast_strain"))
  plast_strain = True
views = gmsh.view.get_tags()

if plast_strain:
  _, data_tags, data, _, _ = gmsh.view.get_model_data(
      views[0], load_step)
  _, plast_strain_tags, plast_strain_data, _, _ = gmsh.view.get_model_data(
      views[-1], load_step)
  max_plast_strain = np.sum([np.abs(a)[3] for a in plast_strain_data])
  if "honeycomb" in data_file:
    max_plast_strain = np.max([np.linalg.norm(a) for a in plast_strain_data])
  if max_plast_strain == 0:
    max_plast_strain = 1
else:
  _, data_tags, data, _, _ = gmsh.view.get_model_data(
      views[-1], load_step)

if not data:
  RuntimeError(f"No data for loadstep {load_step}")

fact = 1e3 * 72 / 25.4  # 1m==1e3 mm  | 1 mm ~ 72/25.4 pt
tikz_str = ""

_, el_tags, _ = gmsh.model.mesh.get_elements(1)
_, indices = np.unique(el_tags[0], return_index=True)
elements = el_tags[0][sorted(indices)]

if plast_strain:
  new_coords = np.array([np.nan, np.nan, np.nan])
  for element in elements:
    plast_strain_idx = np.squeeze(np.where(plast_strain_tags == element))
    color = viridis(
        np.abs(plast_strain_data[plast_strain_idx][3])/max_plast_strain)
    if "honeycomb" in data_file:
      color = viridis(
          np.abs(np.linalg.norm(plast_strain_data[plast_strain_idx])/max_plast_strain))
    tikz_str += f"\n\\draw[color={{rgb,255:red,{color[0]*255:.0f};green,{color[1]*255:.0f};blue,{color[2]*255:.0f}}}]"

    _, el_nodes, _, _ = gmsh.model.mesh.get_element(element)
    for j, node in enumerate(el_nodes):
      old_coords = new_coords
      new_coords, _, _, _ = gmsh.model.mesh.get_node(node)
      idx = np.squeeze(np.where(data_tags == node))
      if "honeycomb" in data_file:
        tikz_str += (' ' if j == 0 else ' -- ') + \
            f"({(new_coords[0]+data[idx][0]*visual_fact)*fact},{(new_coords[1]+data[idx][1]*visual_fact)*fact})"
      else:
        tikz_str += (' ' if j == 0 else ' -- ') + \
            f"({(new_coords[1]+data[idx][1]*visual_fact)*fact},{(new_coords[2]+data[idx][2]*visual_fact)*fact})"

    tikz_str += ";"
else:
  new_coords = np.array([np.nan, np.nan, np.nan])
  for element in elements:
    _, el_nodes, _, _ = gmsh.model.mesh.get_element(element)
    for j, node in enumerate(el_nodes):
      old_coords = new_coords
      new_coords, _, _, _ = gmsh.model.mesh.get_node(node)
      if (old_coords == new_coords).all():
        continue
      idx = np.squeeze(np.where(data_tags == node))
      tikz_str += ('\n' if j == 0 else ' -- ') + \
          f"({(new_coords[0]+data[idx][0]*visual_fact)*fact},{(new_coords[1]+data[idx][1]*visual_fact)*fact})"

with open(f"{data_file[:-8]}_{load_step}.tikz", "w") as file:
  file.write(tikz_str + "\n")

gmsh.finalize()
