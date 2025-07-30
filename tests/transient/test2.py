#!/usr/bin/python3 -Wignore

# TEST 2 (Swinging Rod)

from termcolor import colored
from pathlib import Path
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

data = pd.read_csv("tests/transient/test2/disp.gz",
                   index_col=["time", "label"]).xs("disp",
                                                   level="label")
data.columns = pd.MultiIndex.from_tuples([
    tuple([
        name[:name.find("[")],
        int(name[name.find("[") + 1:name.find("]")])
    ]) for name in data.columns
],
    names=["dof", "node"])

dx = data.xs("dx", axis=1, level="dof")
dz = data.xs("dz", axis=1, level="dof")

end_dx = dx[1]
end_dz = dz[1]
for i in range(1, data.columns.unique("node").max()):
  dx[i] = dx[i + 1]
  dz[i] = dz[i + 1]
dx[i + 1] = end_dx
dz[i + 1] = end_dz

ref_data = pd.read_csv("tests/transient/ref_data/test2_ref.csv",
                       header=[0, 1])

plt.figure(figsize=(10, 8))

for time in ref_data.columns.unique(0):
  t_ref = ref_data.loc[:, time]
  t_ref['dist'] = t_ref[['X', 'Y']].apply(lambda row: np.linalg.norm(
      (row.X, row.Y)),
      axis=1)
  t_ref.sort_values('dist', ignore_index=True, inplace=True)

  t_val = float(time[1:])
  if t_val == 0.0:
    dx.loc[0.0] = [0.0] * len(dx.columns)
    dz.loc[0.0] = [0.0] * len(dz.columns)
  if t_val not in dx.index:
    dx.loc[t_val] = np.nan
    dx = dx.sort_index().interpolate()
  if t_val not in dz.index:
    dz.loc[t_val] = np.nan
    dz = dz.sort_index().interpolate()

  plt.plot(t_ref["X"], t_ref["Y"], label="t = " +
           time[1:] + " (Lang et al. 2011)")
  plt.plot(dx.loc[t_val] + 1 /
           (len(dx.columns) - 1) * dx.columns.unique(),
           dz.loc[t_val],
           "--",
           label="t = " + time[1:] + " (custom implementation)")

plt.legend(ncol=2)
plt.tight_layout()
plt.savefig("tests/transient/test2/result.pdf")
plt.savefig("tests/transient2_result.png")

if max(data.index) > 0.9:
  print(colored("TRANSIENT TEST 2 RUN THROUGH", "green"))
else:
  print(colored("TRANSIENT TEST 2 FAILED", "red", attrs=["bold"]))
