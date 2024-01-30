#!/usr/bin/python3

import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

fig, axs = plt.subplots(4, 4, sharex=True, figsize=(10, 10))

# elastic
regular = pd.read_csv(
    f"tests/manual/testing-unit_cell_elastic.csv", skipinitialspace=True, index_col="H22")
scaled = pd.read_csv(
    f"tests/manual/testing-unit_cell_elastic_scaled.csv", skipinitialspace=True, index_col="H22")

idxs = pd.Index.union(scaled.index, regular.index)
regular = regular.reindex(idxs).interpolate()
scaled = scaled.reindex(idxs).interpolate()

error = np.abs(regular["C_44"]-scaled["C_44"])/regular["C_44"]
axs[0, 0].plot(error, label="rel error")
axs[0, 0].legend()
axs[0, 0].set_title("elastic")


# ideal plastic
regular = pd.read_csv(
    f"tests/manual/testing-unit_cell_noHard.csv", skipinitialspace=True, index_col="H22")
scaled = pd.read_csv(
    f"tests/manual/testing-unit_cell_noHard_scaled.csv", skipinitialspace=True, index_col="H22")

idxs = pd.Index.union(scaled.index, regular.index)
regular = regular.reindex(idxs).interpolate()
scaled = scaled.reindex(idxs).interpolate()

error = np.abs(regular["C_44"]-scaled["C_44"])/regular["C_44"]
axs[1, 0].plot(error, label="rel error")
axs[1, 0].legend()
axs[1, 0].set_title("ideal plastic")


# isotropic hardening plastic
regular = pd.read_csv(
    f"tests/manual/testing-unit_cell_isoHard.csv", skipinitialspace=True, index_col="H22")
scaled = pd.read_csv(
    f"tests/manual/testing-unit_cell_isoHard_scaled.csv", skipinitialspace=True, index_col="H22")
unscaled = pd.read_csv(
    f"tests/manual/testing-unit_cell_isoHard_unscaled.csv", skipinitialspace=True, index_col="H22")

idxs = pd.Index.union(scaled.index, regular.index).union(unscaled.index)
regular = regular.reindex(idxs).interpolate()
scaled = scaled.reindex(idxs).interpolate()
unscaled = unscaled.reindex(idxs).interpolate()

error_scaled = np.abs(regular["C_44"]-scaled["C_44"])/regular["C_44"]
error_unscaled = np.abs(regular["C_44"]-unscaled["C_44"])/regular["C_44"]
axs[2, 1].plot(error_scaled, label="rel error")
axs[2, 1].legend()
axs[2, 1].set_title("isotropic hard plastic\n(scaled)")
axs[2, 0].plot(error_unscaled, label="rel error")
axs[2, 0].legend()
axs[2, 0].set_title("isotropic hard plastic\n(unscaled)")

# kinematic hardening plastic
regular = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard.csv", skipinitialspace=True, index_col="H22")
scaled = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_scaled.csv", skipinitialspace=True, index_col="H22")
unscaled = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_unscaled.csv", skipinitialspace=True, index_col="H22")
yieldScaled2 = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_yieldScaled2.csv", skipinitialspace=True, index_col="H22")
yieldScaledK = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_yieldScaledK.csv", skipinitialspace=True, index_col="H22")

idxs = pd.Index.union(scaled.index, regular.index).union(
    unscaled.index).union(yieldScaled2.index).union(yieldScaledK.index)
regular = regular.reindex(idxs).interpolate()
scaled = scaled.reindex(idxs).interpolate()
unscaled = unscaled.reindex(idxs).interpolate()
yieldScaled2 = yieldScaled2.reindex(idxs).interpolate()
yieldScaledK = yieldScaledK.reindex(idxs).interpolate()

error_scaled = np.abs(regular["C_44"]-scaled["C_44"])/regular["C_44"]
error_unscaled = np.abs(regular["C_44"]-unscaled["C_44"])/regular["C_44"]
error_yieldScaled2 = np.abs(
    regular["C_44"]-yieldScaled2["C_44"])/regular["C_44"]
error_yieldScaledK = np.abs(
    regular["C_44"]-yieldScaledK["C_44"])/regular["C_44"]

axs[3, 3].plot(error_scaled, label="rel error")
axs[3, 3].legend()
axs[3, 3].set_title("kinematic hard plastic\n(scaled)")
axs[3, 2].plot(error_yieldScaledK, label="rel error")
axs[3, 2].legend()
axs[3, 2].set_title("kinematic hard plastic\n(yield scaledK)")
axs[3, 1].plot(error_yieldScaled2, label="rel error")
axs[3, 1].legend()
axs[3, 1].set_title("kinematic hard plastic\n(yield scaled2)")
axs[3, 0].plot(error_unscaled, label="rel error")
axs[3, 0].legend()
axs[3, 0].set_title("kinematic hard plastic\n(unscaled)")


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
