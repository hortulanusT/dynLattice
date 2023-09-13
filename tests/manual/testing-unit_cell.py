#!/usr/bin/python3

import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

fig, axs = plt.subplots(2, 2)
for mod, ax in zip(["elastic", "noHard", "isoHard", "kinHard"], axs.flatten()):
  regular = pd.read_csv(
      f"tests/manual/testing-unit_cell_{mod}.csv", skipinitialspace=True)
  scaled = pd.read_csv(
      f"tests/manual/testing-unit_cell_{mod}_scaled.csv", skipinitialspace=True)
  error = np.abs(regular["C_44"]-scaled["C_44"])/regular["C_44"]
  ax.plot(error, label="rel error")
  ax.legend()
  ax.set_title(mod)
fig.tight_layout()
fig.savefig("tests/manual/testing-unit_cell1.pdf")

fig, ax = plt.subplots()
for mod, ls in zip(["elastic", "noHard", "isoHard", "kinHard"], ["-", "--", "-.", ":"]):
  regular = pd.read_csv(
      f"tests/manual/testing-unit_cell_{mod}.csv", skipinitialspace=True)
  ax.plot(regular["C_44"], ls, label=mod)
ax.legend()
fig.tight_layout()
fig.savefig("tests/manual/testing-unit_cell2.pdf")
