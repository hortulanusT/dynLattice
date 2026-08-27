#!/usr/bin/python3

# Minimal parser for the multi-piece VTU/PVD files written by ParaViewModule.
# meshio refuses these files because the "beams" and "plate1" pieces don't
# share the same point/cell data fields, so each <Piece> is parsed by hand
# instead, using the exact layout ParaViewModule.cpp writes.

import xml.etree.ElementTree as ET
from pathlib import Path

import numpy as np


def read_pvd(pvd_path):
  """Return (time, vtu_path) pairs from a .pvd collection, sorted by time."""
  root = ET.parse(pvd_path).getroot()
  entries = [
      (float(dataset.get("timestep")), Path(pvd_path).parent / dataset.get("file"))
      for dataset in root.iter("DataSet")
  ]
  entries.sort(key=lambda item: item[0])
  return entries


def _floats(text):
  return np.array(text.split(), dtype=float)


def _read_data_arrays(parent_tag, piece, n_rows):
  data = {}
  for data_arr in piece.findall(f"{parent_tag}/DataArray"):
    n_comp = int(data_arr.get("NumberOfComponents", 1))
    data[data_arr.get("Name")] = _floats(data_arr.text).reshape(n_rows, n_comp)
  return data


def read_pieces(vtu_path):
  """Return one dict per <Piece> in the file: points, line segments (as
  pairs of point indices) and the associated point/cell data arrays."""
  root = ET.parse(vtu_path).getroot()
  pieces = []

  for piece in root.iter("Piece"):
    points = _floats(piece.find("Points/DataArray").text).reshape(-1, 3)

    connectivity = _floats(
        piece.find("Cells/DataArray[@Name='connectivity']").text
    ).astype(int)
    offsets = _floats(piece.find(
        "Cells/DataArray[@Name='offsets']").text).astype(int)
    starts = np.concatenate([[0], offsets[:-1]])
    lines = np.array([connectivity[s:e] for s, e in zip(starts, offsets)])

    pieces.append(
        {
            "points": points,
            "lines": lines,
            "point_data": _read_data_arrays("PointData", piece, len(points)),
            "cell_data": _read_data_arrays("CellData", piece, len(lines)),
        }
    )
  return pieces
