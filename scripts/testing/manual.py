#!/usr/bin/python3

import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("tests/manual/testing/stateVectors.csv", index_col=["time", "state"])

pos = data.xs(0, level="state")
vel = data.xs(1, level="state")
acc = data.xs(2, level="state")

pos.plot( y=["ry[1]","rz[1]"], subplots=True, xlim=[4.0305, 4.0315], title="pos" )
vel.plot( y=["ry[1]","rz[1]"], subplots=True, xlim=[4.0305, 4.0315], title="vel" )
acc.plot( y=["ry[1]","rz[1]"], subplots=True, xlim=[4.0305, 4.0315], title="acc" )
plt.show()