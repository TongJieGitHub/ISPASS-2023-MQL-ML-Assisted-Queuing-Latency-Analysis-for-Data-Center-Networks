import os
import sys
import glob
import pandas as pd
import numpy as np

# suffix = 'mimic_nodes_16_load_07_rand_1'
# filename_sim = '../statistics_' + suffix + '/mimiclog_' + suffix + '_pkt500_delta0_queue128_Tcp_ecmp_time10.csv'
# filename_ana = 'latency_per_flow_' + suffix + '_max_0999_q128.csv'
# filename_output = 'latency_per_flow_' + suffix + '_max_0999_q128_merged.csv'
# filename_sim = '../statistics_mimic_1x1bi_cwnd_6hops_rand2/mimiclog_mimic_1x1bi_pkt500_delta0_queue128_Tcp_ecmp_time100.csv'
# filename_ana = 'latency_per_flow_mimic_1x1bi_cwnd_6hops_rand2_me_q128.csv'
# filename_output = 'latency_per_flow_mimic_1x1bi_cwnd_6hops_rand2_q128_me_merged.csv'
# filename_sim = '../statistics_5/mimiclog_mimic_pkt500_delta0_queue128_Tcp_ecmp_time10.csv'
# filename_ana = 'latency_per_flow_mimic_5_q128.csv'
# filename_output = 'latency_per_flow_mimic_5_q128_merged.csv'
filename_sim = '../runs/dcn_fattree_finite_mimic_v3_q128_w200_t1x1bi/reports_sim/end_to_end_latency.csv'
filename_ana = '../runs/dcn_fattree_finite_mimic_v3_q128_w200_t1x1bi/reports_ana/latency_per_flow.csv'
filename_output = '../runs/dcn_fattree_finite_mimic_v3_q128_w200_t1x1bi/reports_ana/latency_per_flow_merged.csv'

# averageDelayUnits = 1e6

print(filename_sim)
data_sim = pd.read_csv(filename_sim)
df_sim = pd.DataFrame(data_sim)
# df_sim['latency_sim'] = df_sim['latency_sim'] / averageDelayUnits
# df_sim['rtt'] = df_sim['rtt'] / averageDelayUnits

print(filename_ana)
data_ana = pd.read_csv(filename_ana)
df_ana = pd.DataFrame(data_ana)

df_merged = pd.merge(df_sim, df_ana,
                     on=['timestamp', 'flowId', 'client', 'server'],
                     how='inner')


df_merged['pct_error_latency_inf'] = ((df_merged['latency_inf'] - df_merged['latency_sim']) / df_merged['latency_sim']) * 100
df_merged['pct_error_latency_finC'] = ((df_merged['latency_finC'] - df_merged['latency_sim']) / df_merged['latency_sim']) * 100
df_merged['pct_error_latency_finR'] = ((df_merged['latency_finR'] - df_merged['latency_sim']) / df_merged['latency_sim']) * 100

df_merged['abs_pct_error_latency_inf'] = ((df_merged['latency_inf'] - df_merged['latency_sim']) / df_merged['latency_sim']).abs() * 100
df_merged['abs_pct_error_latency_finC'] = ((df_merged['latency_finC'] - df_merged['latency_sim']) / df_merged['latency_sim']).abs() * 100
df_merged['abs_pct_error_latency_finR'] = ((df_merged['latency_finR'] - df_merged['latency_sim']) / df_merged['latency_sim']).abs() * 100

print(filename_output)
df_merged.to_csv(filename_output, index=False)