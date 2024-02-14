#!/usr/bin/python3

import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

testing_measure = "P22"

# creat a figure with 3x4 subplots and A4 size
fig, axs = plt.subplots(3, 4, sharex=True, figsize=(29.7 / 2.54, 21 / 2.54))

# elastic
regular = pd.read_csv(
    f"tests/manual/testing-unit_cell_elastic.csv", skipinitialspace=True, index_col="H22")
scaled = pd.read_csv(
    f"tests/manual/testing-unit_cell_elastic_scaled.csv", skipinitialspace=True, index_col="H22")

idxs = pd.Index.union(scaled.index, regular.index)
regular = regular.reindex(idxs).interpolate()
scaled = scaled.reindex(idxs).interpolate()

error = np.abs(regular[testing_measure] -
               scaled[testing_measure])/regular[testing_measure]
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

error = np.abs(regular[testing_measure] -
               scaled[testing_measure])/regular[testing_measure]
axs[0, 1].plot(error, label="rel error")
axs[0, 1].legend()
axs[0, 1].set_title("ideal plastic")

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

error_scaled = np.abs(regular[testing_measure] -
                      scaled[testing_measure])/regular[testing_measure]
error_unscaled = np.abs(
    regular[testing_measure]-unscaled[testing_measure])/regular[testing_measure]
axs[0, 3].plot(error_scaled, label="rel error")
axs[0, 3].legend()
axs[0, 3].set_title("isotropic hard plastic\n(scaled)")
axs[0, 2].plot(error_unscaled, label="rel error")
axs[0, 2].legend()
axs[0, 2].set_title("isotropic hard plastic\n(unscaled)")

# kinematic hardening plastic
regular = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard.csv", skipinitialspace=True, index_col="H22")
scaled2 = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_scaled2.csv", skipinitialspace=True, index_col="H22")
scaledK = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_scaledK.csv", skipinitialspace=True, index_col="H22")
unscaled = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_unscaled.csv", skipinitialspace=True, index_col="H22")
yieldUnscaled = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_yieldUnscaled.csv", skipinitialspace=True, index_col="H22")
yieldScaled2 = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_yieldScaled2.csv", skipinitialspace=True, index_col="H22")
yieldScaledK = pd.read_csv(
    f"tests/manual/testing-unit_cell_kinHard_yieldScaledK.csv", skipinitialspace=True, index_col="H22")

idxs = pd.Index.union(scaled2.index, regular.index).union(scaledK.index).union(
    unscaled.index).union(yieldScaled2.index).union(yieldScaledK.index).union(yieldUnscaled.index)
regular = regular.reindex(idxs).interpolate()
scaled2 = scaled2.reindex(idxs).interpolate()
scaledK = scaledK.reindex(idxs).interpolate()
unscaled = unscaled.reindex(idxs).interpolate()
yieldUnscaled = yieldUnscaled.reindex(idxs).interpolate()
yieldScaled2 = yieldScaled2.reindex(idxs).interpolate()
yieldScaledK = yieldScaledK.reindex(idxs).interpolate()

error_scaled2 = np.abs(regular[testing_measure] -
                       scaled2[testing_measure])/regular[testing_measure]
error_scaledK = np.abs(regular[testing_measure] -
                       scaledK[testing_measure])/regular[testing_measure]
error_unscaled = np.abs(
    regular[testing_measure]-unscaled[testing_measure])/regular[testing_measure]
error_yieldScaled2 = np.abs(
    regular[testing_measure]-yieldScaled2[testing_measure])/regular[testing_measure]
error_yieldScaledK = np.abs(
    regular[testing_measure]-yieldScaledK[testing_measure])/regular[testing_measure]
error_yieldUnscaled = np.abs(
    regular[testing_measure]-yieldUnscaled[testing_measure])/regular[testing_measure]

axs[2, 3].plot(error_scaled2, label="rel error")
axs[2, 3].legend()
axs[2, 3].set_title("kinematic hard plastic\n(scaled2)")
axs[1, 3].plot(error_scaledK, label="rel error")
axs[1, 3].legend()
axs[1, 3].set_title("kinematic hard plastic\n(scaledK)")
axs[1, 2].plot(error_yieldScaledK, label="rel error")
axs[1, 2].legend()
axs[1, 2].set_title("kinematic hard plastic\n(yield scaledK)")
axs[2, 2].plot(error_yieldScaled2, label="rel error")
axs[2, 2].legend()
axs[2, 2].set_title("kinematic hard plastic\n(yield scaled2)")
axs[1, 1].plot(error_yieldUnscaled, label="rel error")
axs[1, 1].legend()
axs[1, 1].set_title("kinematic hard plastic\n(unscaled)")
axs[1, 0].plot(error_unscaled, label="rel error")
axs[1, 0].legend()
axs[1, 0].set_title("kinematic hard plastic\n(yield unscaled)")


for ax in axs.flatten():
  ax.ticklabel_format(useOffset=False, style='plain')

fig.delaxes(axs[2, 0])
fig.delaxes(axs[2, 1])

fig.tight_layout()
fig.savefig("tests/manual/testing-unit_cell1.pdf")

fig, ax = plt.subplots()
for mod, ls in zip(["elastic", "noHard", "isoHard", "kinHard"], ["-", "--", "-.", ":"]):
  regular = pd.read_csv(
      f"tests/manual/testing-unit_cell_{mod}.csv", skipinitialspace=True, index_col="H22")
  ax.plot(regular[testing_measure], ls, label=mod)
ax.legend()
fig.tight_layout()
fig.savefig("tests/manual/testing-unit_cell2.pdf")
