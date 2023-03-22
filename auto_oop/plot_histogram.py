import os
import sys
import glob
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import PercentFormatter

filename = 'latency_per_flow_mimic_400_q128_merged.csv'

print(filename)
df = pd.read_csv(filename)
df = pd.DataFrame(df)
data = df['abs_error_latency_finC']

plt.hist(data, weights=np.ones(len(data)) / len(data), bins=[0, 10, 20, 30, 40, 50, 60 ,70,80,90,100], rwidth = 1)
# plt.hist(data, weights=np.ones(len(data)) / len(data), bins=11,density=True)
plt.gca().yaxis.set_major_formatter(PercentFormatter(1))
plt.savefig("hisogram.png", dpi=500)


