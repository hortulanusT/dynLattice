#!/usr/bin/python3

import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

states = pd.read_csv("tests/manual/testing/stateVectors.csv", index_col=["time", "state"])
states.columns = pd.MultiIndex.from_tuples([tuple([name[:2], int(name[3])]) for name in states.columns], names=["dof", "node"])

pos0 = states.xs(0, level="state").xs(0, axis="columns", level="node")
vel0 = states.xs(1, level="state").xs(0, axis="columns", level="node")
acc0 = states.xs(2, level="state").xs(0, axis="columns", level="node")
pos1 = states.xs(0, level="state").xs(1, axis="columns", level="node")
vel1 = states.xs(1, level="state").xs(1, axis="columns", level="node")
acc1 = states.xs(2, level="state").xs(1, axis="columns", level="node")
del states

forces = pd.read_csv("tests/manual/testing/forceVectors.csv", index_col=["time", "force"])
forces.columns = pd.MultiIndex.from_tuples([tuple([name[:2], int(name[3])]) for name in forces.columns], names=["dof", "node"])

int0 = forces.xs("intVector", level="force").xs(0, axis="columns", level="node")
gyro0 = forces.xs("gyroVector", level="force").xs(0, axis="columns", level="node")
int1 = forces.xs("intVector", level="force").xs(1, axis="columns", level="node")
gyro1 = forces.xs("gyroVector", level="force").xs(1, axis="columns", level="node")
del forces

acc0.plot.line( y=["ry","rz"], subplots=True, title="acc0" )
acc1.plot.line( y=["ry","rz"], subplots=True, title="acc1", ylim=[-10,10] )
int0.plot.line( y=["ry","rz"], subplots=True, title="int0" )
int1.plot.line( y=["ry","rz"], subplots=True, title="int1" )

with PdfPages("tests/manual/testing/plots.pdf") as file:
  for n in plt.get_fignums():
    file.savefig(plt.figure(n))