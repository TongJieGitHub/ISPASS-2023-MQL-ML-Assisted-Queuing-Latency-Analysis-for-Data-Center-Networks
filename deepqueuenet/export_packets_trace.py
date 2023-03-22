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
import matplotlib.ticker as ticker
import seaborn as sns
from fitter import Fitter, get_common_distributions, get_distributions
import pathlib

dirname = 'poisson/rsim1/'
filename = 'fattree16/poisson/rsim1.csv'
pathlib.Path(dirname).mkdir(parents=True, exist_ok=True) 

print(filename)
df = pd.read_csv(filename)
df = pd.DataFrame(df)

df['src'] = df['src_pc']
df['dest'] = (df['cur_hub'] - 12) * 2 + df['cur_port']

df_count = df.groupby(['src', 'dest']).size().sort_values(ascending=False) .reset_index(name='count') 
df_count.to_csv(dirname + "trace_src_dest_count.csv", index=False)


for ind in range(0, len(df_count)):
    src = df_count['src'][ind]
    dest = df_count['dest'][ind]

    print(src, dest)
    df_src_dest = df[(df['src'] == src) & (df['dest'] == dest)]
    # print(df_src_dest)
    # df_src_dest.to_csv('results_c' + str(m_dict[src]) + '_s' + str(m_dict[dest]) + '.csv', index=False)
    data_time = df_src_dest['timestamp (sec)']
    data_time = data_time.diff()
    data_time = data_time.dropna()
    data_length = df_src_dest['pkt len (byte)']
    data_length = data_length.tail(data_length.shape[0] -1)

    data_merge = pd.merge(data_time, data_length, how='left', left_index=True, right_index=True)
    data_merge.to_csv(dirname + 'trace_c' + str(src) + '_s' + str(dest) + '.csv', index=False, header=False)


