#!/usr/bin/python3

import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

for entry in os.scandir("tests/manual"):
  if entry.is_dir() and "testing" == entry.name:
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
          f"{state}.pdf")) as file:
        for node in states.columns.unique("node"):
          print(f"\t{state}\tnode{node}")
          axs = states.xs(state, 
            level="label").xs(node, axis="columns", 
            level="node").plot.line( subplots=True, layout=[2,3], 
            figsize=(10,6), title=f"{study_type} -- node{node}") 

          f = axs.flatten()[0].get_figure()
          f.tight_layout(rect=[0,0,1,0.95])
          file.savefig(f)
          f.clear()
          plt.close(f)