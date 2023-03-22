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

filename = 'packets_trace_tcp_1.csv'

print(filename)
df = pd.read_csv(filename)
df = pd.DataFrame(df)

df_count = df.groupby(['hw_src', 'hw_dest']).size().sort_values(ascending=False) .reset_index(name='count') 
df_count.to_csv("trace_src_dest_count.csv", index=False)


m_dict = dict()
for ind in range(0, 20):
    src = df_count['hw_src'][ind]
    dest = df_count['hw_dest'][ind]
    if src not in m_dict:
        m_dict[src] = len(m_dict)
    if dest not in m_dict:
        m_dict[dest] = len(m_dict)
    print(src, dest)
    df_src_dest = df[(df['hw_src'] == src) & (df['hw_dest'] == dest)]
    # print(df_src_dest)
    # df_src_dest.to_csv('results_c' + str(m_dict[src]) + '_s' + str(m_dict[dest]) + '.csv', index=False)
    data_time = df_src_dest['Time']
    data_time = data_time.diff()
    data_time = data_time.dropna()
    data_length = df_src_dest['Length']
    data_length = data_length.tail(data_length.shape[0] -1)
    # data_time.to_csv('results_c' + str(m_dict[src]) + '_s' + str(m_dict[dest]) + '_time.csv', index=False)
    # data_length.to_csv('results_c' + str(m_dict[src]) + '_s' + str(m_dict[dest]) + '_length.csv', index=False)
    data_merge = pd.merge(data_time, data_length, how='left', left_index=True, right_index=True)
    data_merge.to_csv('trace_c' + str(m_dict[src]) + '_s' + str(m_dict[dest]) + '.csv', index=False, header=False)


print(m_dict)
with open("trace_mapping.csv", 'w') as f: 
    for key, value in m_dict.items(): 
        f.write('%s,%s\n' % (value, key))
