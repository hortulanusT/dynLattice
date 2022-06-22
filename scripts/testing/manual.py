#!/usr/bin/python3

import os, subprocess
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages


for time_step in np.geomspace(1e-4, 1e-8):
  try:
    os.mkdir(f'tests/manual/testing_dt{time_step:.3E}', )
  except OSError:
    pass
  finally:
    subprocess.call(['bin/nonlinRod'
    , '-p', f'MODIFIER=\"_dt{time_step:.3E}\"'
    , 'tests/manual/testing.pro'
    , '-p', f'Solver.integrator.deltaTime={time_step}'])

for entry in os.scandir("tests/manual"):
  if entry.is_dir() and "testing_dt" in str(entry):
    study_type = entry.name[8:].upper()
    print( "\n", study_type )

    states = pd.read_csv(os.path.join(entry, "stateVectors.gz"),
     index_col=["time", "label"])
    states.columns = pd.MultiIndex.from_tuples([tuple([name[:name.find("[")], 
      int(name[name.find("[")+1:name.find("]")])])\
        for name in states.columns], names=["dof", "node"])
    print( "\t", states.index.unique("time").max(), " sec")
    # states = states.loc[.045:.055]
    states.loc[slice(None), ["rx", "ry", "rz"]] *= 180/np.pi

    for state in states.index.unique("label"):
      with PdfPages(os.path.join(entry, 
          f"{state}_{time_step}.pdf")) as file:
        for node in states.columns.unique("node"):
          print(f"\t{state}\tnode{node}")
          states.xs(state, 
            level="label").xs(node, axis="columns", 
            level="node").plot.line( subplots=True, layout=[2,3], 
            figsize=(10,6), title=f"{study_type} -- node{node}") 
          plt.tight_layout(rect=[0,0,1,0.95])
          file.savefig(plt.gcf())
          plt.close()