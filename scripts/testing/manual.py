#!/usr/bin/python3

import os
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

statename = {0:"pos", 1:"velo", 2:"acc"}

for entry in os.scandir("tests/manual"):
  if entry.is_dir() and entry.name == "testing":
    study_type = entry.name[8:].upper()
    print( study_type )

    states = pd.read_csv(os.path.join(entry, "stateVectors.gz"), index_col=["time", "state"])
    states.columns = pd.MultiIndex.from_tuples([tuple([name[:2], int(name[3])]) for name in states.columns], names=["dof", "node"])
    states = states.loc[:5]

    forces = pd.read_csv(os.path.join(entry, "forceVectors.gz"), index_col=["time", "force"])
    forces.columns = pd.MultiIndex.from_tuples([tuple([name[:2], int(name[3])]) for name in forces.columns], names=["dof", "node"])
    print(forces.index.get_level_values("time").max())
    forces = forces.loc[:5]

    with PdfPages(os.path.join(entry, "plots.pdf")) as file:
      for node in range(2):
        for state in range(3):
          states.xs(state, level="state").xs(node, axis="columns", level="node").plot.line( subplots=True, layout=[2,3], figsize=(10,6), title=f"{study_type} -- {statename[state]} node{node}" )
          print(f"\tnode{node}\t{statename[state]}")
          plt.tight_layout(rect=[0,0,1,0.95])
          file.savefig(plt.gcf())
          plt.close()
        for force in ["int", "ext", "gyro"]:
          forces.xs(f"{force}Vector", level="force").xs(node, axis="columns", level="node").plot.line( subplots=True, layout=[2,3], figsize=(10,6), title=f"{study_type} -- {force}Force node{node}" )
          print(f"\tnode{node}\t{force}Force")
          plt.tight_layout(rect=[0,0,1,0.95])
          file.savefig(plt.gcf())
          plt.close()
