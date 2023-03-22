import os
import sys
import glob
import pandas as pd
import numpy as np

# suffix = 'mimic_nodes_16_load_07_rand_1'
# filename_sim = '../statistics_' + suffix + '/flowlog_' + suffix + '_pkt500_delta0_queue128_Tcp_ecmp_time10.csv'
# filename_ana = 'queue_' + suffix + '_max_0999_q128.csv'
# filename_output = 'queue_' + suffix + '_max_0999_q128_merged.csv'
# filename_sim = '../statistics_mimic_1x1bi_cwnd_6hops_rand2/flowlog_mimic_1x1bi_pkt500_delta0_queue128_Tcp_ecmp_time100.csv'
# filename_ana = 'queue_mimic_1x1bi_cwnd_6hops_rand2_me_q128.csv'
# filename_output = 'queue_mimic_1x1bi_cwnd_6hops_rand2_me_q128_merged.csv'
filename_sim = '../runs/dcn_fattree_finite_mimic_v3_q128_w200_t1x1bi/outputs_sim/flow_queue_log.csv'
filename_ana = '../runs/dcn_fattree_finite_mimic_v3_q128_w200_t1x1bi/reports_ana/latency_per_queue.csv'
filename_output = '../runs/dcn_fattree_finite_mimic_v3_q128_w200_t1x1bi/reports_ana/latency_per_queue_merged.csv'

# averageDelayUnits = 1e6

print(filename_sim)
data_sim = pd.read_csv(filename_sim)
df_sim = pd.DataFrame(data_sim)
# df_sim['latency_sim'] = df_sim['latency_sim'] / averageDelayUnits
# df_sim['link_delay_sim'] = df_sim['link_delay_sim'] / averageDelayUnits

print(filename_ana)
data_ana = pd.read_csv(filename_ana)
df_ana = pd.DataFrame(data_ana)

df_merged = pd.merge(df_sim, df_ana,
                     on=['timestamp', 'flowId', 'client', 'server', 'nodeId', 'queueId'],
                     how='inner')

# df_merged['abs_error_datarate'] = ((df_merged['observedDatarate'] - df_merged['datarate_ana']) / df_merged['observedDatarate']).abs() * 100

df_merged['abs_error_CA_sqr'] = ((df_merged['CA_sqr'] - df_merged['CA_sqr_ana']) / df_merged['CA_sqr']).abs() * 100

# df_merged['abs_error_meanPacketSize'] = ((df_merged['meanPacketSize'] - df_merged['packet_size_ana']) / df_merged['meanPacketSize']).abs() * 100
df_merged['pct_error_latency_inf'] = ((df_merged['latency_inf'] - df_merged['latency_sim']) / df_merged['latency_sim']) * 100
df_merged['pct_error_latency_finC'] = ((df_merged['latency_finC'] - df_merged['latency_sim']) / df_merged['latency_sim']) * 100
df_merged['pct_error_latency_finR'] = ((df_merged['latency_finR'] - df_merged['latency_sim']) / df_merged['latency_sim']) * 100

df_merged['abs_pct_error_latency_inf'] = ((df_merged['latency_inf'] - df_merged['latency_sim']) / df_merged['latency_sim']).abs() * 100
df_merged['abs_pct_error_latency_finC'] = ((df_merged['latency_finC'] - df_merged['latency_sim']) / df_merged['latency_sim']).abs() * 100
df_merged['abs_pct_error_latency_finR'] = ((df_merged['latency_finR'] - df_merged['latency_sim']) / df_merged['latency_sim']).abs() * 100

print(filename_output)
df_merged.to_csv(filename_output, index=False)