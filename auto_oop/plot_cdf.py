import os
import sys
import glob
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import PercentFormatter
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import FormatStrFormatter
from matplotlib.pyplot import figure


filename = 'latency_per_flow_mimic_200_max_0999_q128_merged.csv'

print(filename)
df = pd.read_csv(filename)
df = pd.DataFrame(df)

cond = (df['timestamp'] > 1000)
df = df[cond]

data_sim = df['delayAverage']
data_finC = df['latency_finC']

data_sim_sorted = np.sort(data_sim)
data_finC_sorted = np.sort(data_finC)

# p_sim = 1. * np.arange(len(data_sim)) / (len(data_sim) - 1)
# p_finC = 1. * np.arange(len(data_finC)) / (len(data_finC) - 1)
p_sim = np.arange(len(data_sim_sorted))/float(len(data_sim_sorted))
p_finC = np.arange(len(data_finC_sorted))/float(len(data_finC_sorted))

font = FontProperties(weight='bold', size=12)
fig, ax = plt.subplots()
ax.plot(data_sim_sorted, p_sim,color='black', label='Simulation',
                              linestyle='dashed', markerfacecolor='none')
ax.plot(data_finC_sorted, p_finC, color='red', label='Analytical (finite censored)',
                              linestyle='dotted', markerfacecolor='none')
ax.set_xlabel('$Latency (ms)$')
ax.set_ylabel('$Fraction$')
plt.savefig("cdf.png", dpi=500)

print("90th percentile of sim : ", np.percentile(data_sim_sorted, 90))
print("90th percentile of finC : ", np.percentile(data_finC_sorted, 90))