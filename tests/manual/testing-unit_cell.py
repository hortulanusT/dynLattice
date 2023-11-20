#!/usr/bin/python3

import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

fig, axs = plt.subplots(2, 2)
for mod, ax in zip(["elastic", "noHard", "isoHard", "kinHard"], axs.flatten()):
    regular = pd.read_csv(
        f"tests/manual/testing-unit_cell_{mod}.csv", skipinitialspace=True, index_col="H22")
    scaled = pd.read_csv(
        f"tests/manual/testing-unit_cell_{mod}_scaled.csv", skipinitialspace=True, index_col="H22")
    regular.loc[regular.index.difference(scaled.index)] = np.nan
    regular = regular.sort_index().interpolate()
    scaled.loc[scaled.index.difference(regular.index)] = np.nan
    scaled = scaled.sort_index().interpolate()
    error = np.abs(regular["C_44"]-scaled["C_44"])/regular["C_44"]
    ax.plot(error, label="rel error")
    ax.legend()
    ax.set_title(mod)
fig.tight_layout()
fig.savefig("tests/manual/testing-unit_cell1.pdf")

fig, ax = plt.subplots()
for mod, ls in zip(["elastic", "noHard", "isoHard", "kinHard"], ["-", "--", "-.", ":"]):
    regular = pd.read_csv(
        f"tests/manual/testing-unit_cell_{mod}.csv", skipinitialspace=True, index_col="H22")
    ax.plot(regular["C_44"], ls, label=mod)
ax.legend()
fig.tight_layout()
fig.savefig("tests/manual/testing-unit_cell2.pdf")
