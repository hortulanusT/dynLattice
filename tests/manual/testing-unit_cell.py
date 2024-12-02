#!/usr/bin/python3

import numpy as np
import pandas as pd
import re
from collections import deque
from matplotlib import pyplot as plt


def uniaxialStiffness(raw_data: pd.DataFrame, thickness=0.75e-3):
  H_22 = raw_data["H22"].to_numpy()
  H_11 = raw_data["H11"].to_numpy()
  P_22 = raw_data["P22"].to_numpy()

  E_y = np.gradient(P_22, H_22) / thickness
  nu_xy = -1. * np.gradient(H_11, H_22)

  return pd.DataFrame({"E_y": E_y, "nu_xy": nu_xy}, raw_data.index)


def constrainedModulus(compliance: np.ndarray, index):
  indices = deque(range(4))
  indices.rotate(-index)
  a, b, c, d = indices

  f_da = -1. * compliance[:, d, a]/compliance[:, d, d]
  f_db = -1. * compliance[:, d, b]/compliance[:, d, d]
  f_dc = -1. * compliance[:, d, c]/compliance[:, d, d]

  f_ca = -1. * (compliance[:, c, a] + compliance[:, c, d] *
                f_da) / (compliance[:, c, c] + compliance[:, c, d] * f_dc)
  f_cb = -1. * (compliance[:, c, a] + compliance[:, c, d] *
                f_db) / (compliance[:, c, c] + compliance[:, c, d] * f_dc)

  f_ba = -1. * (compliance[:, b, a] + compliance[:, b, c]*f_ca + compliance[:, b, d]*(f_da + f_dc*f_ca)) / (
      compliance[:, b, b] + compliance[:, b, c]*f_cb + compliance[:, b, d]*(f_db + f_dc*f_cb))

  constrainedCompliance = compliance[:, a, a] + compliance[:, a, b]*f_ba + compliance[:, a, c]*(
      f_ca + f_cb * f_ba) + compliance[:, a, d] * (f_da + f_dc*f_ca + (f_db + f_dc*f_cb) * f_ba)

  return 1. / constrainedCompliance


def freeModulus(compliance: np.ndarray, index):
  indices = deque(range(4))
  if index in [1, 2]:
    indices = deque([1, 0, 3, 2])
  if index > 1:
    indices.reverse()
  a, b, c, d = indices

  f_ca = -1. * compliance[:, c, a]/compliance[:, c, c]
  f_cb = -1. * compliance[:, c, b]/compliance[:, c, c]

  f_ba = -1. * (compliance[:, b, a] + compliance[:, b, c] *
                f_ca) / (compliance[:, b, b] + compliance[:, b, c] * f_cb)

  freeCompliance = compliance[:, a, a] + compliance[:, a,
                                                    b] * f_ba + compliance[:, a, c] * (f_ca + f_cb * f_ba)

  lateralCompliance = compliance[:, d, a] + compliance[:, d,
                                                       b] * f_ba + compliance[:, d, c] * (f_ca + f_cb * f_ba)

  return 1. / freeCompliance, -1. * lateralCompliance / freeCompliance


def translateStiffness(raw_data: pd.DataFrame, filter_cols=r"[CSPH]_\d*"):
  tangentStiff = raw_data.filter(like="C").to_numpy()
  tangentStiff = tangentStiff.reshape(-1, 4, 4)
  tangentCompliance = np.linalg.inv(
      0.5 * tangentStiff + 0.5 * tangentStiff.transpose([0, 2, 1]))

  modules = {}
  for col in raw_data.columns:
    if not re.match(filter_cols, col):
      modules[col] = raw_data[col].to_numpy()
  modules["E_x"], modules["nu_xy"] = freeModulus(
      tangentCompliance, 0)
  modules["E_y"], modules["nu_yx"] = freeModulus(
      tangentCompliance, 3)
  modules["M_x"] = constrainedModulus(tangentCompliance, 0)
  modules["G_xy"] = constrainedModulus(tangentCompliance, 1)
  modules["G_yx"] = constrainedModulus(tangentCompliance, 2)
  modules["M_y"] = constrainedModulus(tangentCompliance, 3)

  return pd.DataFrame(modules, raw_data.index)


print(" ... plotting")

fig, ax = plt.subplots(1, 1, figsize=(29.7 / 2.54, 21 / 2.54))

# , "_contact", "_plastic_contact"]):
for model in ["", "_plastic"]:
  raw_data = pd.read_csv(
      f"studies/output/tno-design/ARH/evo/uniaxial-1x1{model}.csv")
  data = translateStiffness(raw_data)
  data = data.join(uniaxialStiffness(data), rsuffix="_raw")

  ax.plot(-100 * data["H22"], data["E_y"], label=f"E_y{model}")
  ax.plot(-100 * data["H22"], data["E_y_raw"], "--", label=f"E_y{model} (raw)")
  ax.legend(loc="lower left")
  ax.set_xlim(left=0, right=0.5)
  ax.set_ylim(bottom=0, top=2.5e9)

fig.tight_layout()
fig.savefig("tests/manual/testing-unit_cell.pdf")
