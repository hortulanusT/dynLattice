#!/usr/bin/python3

# TEST 2 (Swinging Rod)

from termcolor import colored
from pathlib import Path
import pandas as pd

data = pd.read_csv("tests/transient/test2/disp.gz")

if max(data.index) > 0.9:
  print(colored("TRANSIENT TEST 2 RUN THROUGH", "green"))
  Path('tests/transient/test2/result.pdf').touch()
else:
  print(colored("TRANSIENT TEST 2 FAILED", "red", attrs=["bold"]))
