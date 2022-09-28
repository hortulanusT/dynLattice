#!/usr/bin/python3

import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

prop_cycle = plt.rcParams['axes.prop_cycle']
colors = prop_cycle.by_key()['color']
fig, axs = plt.subplots(2, 3, figsize=(10, 6))

for entry in os.scandir("tests/manual"):
  if entry.is_dir():
    study_type = entry.name[8:].upper()
    print("\n", study_type)

    states = pd.read_csv(os.path.join(entry, "stateVectors.gz"),
                         index_col=["time", "label"])
    states.columns = pd.MultiIndex.from_tuples([tuple([name[:name.find("[")],
                                                       int(name[name.find("[")+1:name.find("]")])])
                                                for name in states.columns], names=["dof", "node"])
    print("\t", states.index.unique("time").max(), " sec")
    states = states.loc[:.01]
    # states.loc[slice(None), ["rx", "ry", "rz"]] *= 180/np.pi

    # rz1_pos = states.xs("disp", level="label").xs(1, axis="columns", level="node")["rz"]
    # rz1_pos.to_csv(os.path.join(entry, "rz1_extract"))
    # print(rz1_pos)

    for state in states.index.unique("label"):
      with PdfPages(os.path.join(entry,
                                 f"{state}_0.01sec.pdf")) as file:
        for node in states.columns.unique("node"):
          print(f"\t{state}\tnode{node}")
          plot_data = states.xs(state, level="label").xs(
              node, axis="columns", level="node")

          axs[0, 0].plot(plot_data["dx"], label="dx", color=colors[0])
          axs[0, 1].plot(plot_data["dy"], label="dy", color=colors[1])
          axs[0, 2].plot(plot_data["dz"], label="dz", color=colors[2])
          axs[1, 0].plot(plot_data["rx"], label="rx", color=colors[3])
          axs[1, 1].plot(plot_data["ry"], label="ry", color=colors[4])
          axs[1, 2].plot(plot_data["rz"], label="rz", color=colors[5])

          for ax in axs.flatten():
            ax.legend()

          fig.suptitle(f"{study_type} -- node{node}")
          fig.tight_layout(rect=[0, 0, 1, 0.95])
          file.savefig(fig)
          for ax in axs.flatten():
            ax.clear()

fig.clear()
plt.close(fig)
