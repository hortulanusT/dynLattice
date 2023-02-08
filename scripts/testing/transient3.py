#!/usr/bin/python3

# TEST 3 (Simo Paper 3D-Test)

import pandas as pd
import matplotlib.pyplot as plt

energy = pd.read_csv("tests/transient/test3/energy.csv",
                     index_col="time")
disp = pd.read_csv("tests/transient/test3/disp.gz", index_col="time")
ref_disp = pd.read_csv("tests/transient/ref_data/test2_ref.csv",
                       header=[0, 1])
print(ref_disp)

ellbow = disp[["dx[1]", "dy[1]", "dz[1]"]]
tip = disp[["dx[2]", "dy[2]", "dz[2]"]]

plt.plot(ellbow["dz[1]"], label="ellbow")
plt.plot(ref_disp["Ellbow"]["X"],
         ref_disp["Ellbow"]["Y"],
         "--",
         label="ellbow (Simo et al. 1988)")
plt.plot(tip["dz[2]"], label="tip")
plt.plot(ref_disp["Tip"]["X"],
         ref_disp["Tip"]["Y"],
         "--",
         label="tip (Simo et al. 1988)")
plt.xlim(0, 30)
plt.ylim(-10, 10)
plt.legend()
plt.tight_layout()

plt.savefig("tests/transient/test3/result.pdf")
