import pandas as pd
import numpy as np
from DT_predict import *

date = '0504'
#traffic = ['incast', 'outcast']
traffic = ['alltoall', 'incast', 'interleaved', 'outcast']
queue_list = ['client_up', 'edge_down', 'edge_up', 'agg_down', 'agg_up', 'core_down']
id_client_up = [20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35]
id_edge_down = [12, 13, 14, 15, 16, 17, 18, 19]
id_edge_up = [12, 13, 14, 15, 16, 17, 18, 19]
id_agg_down = [4, 5, 6, 7, 8, 9, 10, 11]
id_agg_up = [4, 5, 6, 7, 8, 9, 10, 11]
id_core_down = [0, 1, 2, 3]
id_list = [id_client_up, id_edge_down, id_edge_up, id_agg_down, id_agg_up, id_core_down]
up_port_list = [1, 2]
down_port_list = [3, 4] 

for t in traffic:    
    for queue in queue_list:
        data = pd.read_csv('flowlog_mean_DT_' + date + '_' + t + '_' + queue + '.csv')        
        df = pd.DataFrame(data)

        if queue == 'client_up':
            df['DT best model'] = df.apply(lambda row : predict_client_up(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
        if queue == 'edge_down':
            df['DT best model'] = df.apply(lambda row : predict_edge_down(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
        if queue == 'edge_up':
            df['DT best model'] = df.apply(lambda row : predict_edge_up(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
        if queue == 'agg_down':
            df['DT best model'] = df.apply(lambda row : predict_agg_down(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
        if queue == 'agg_up':
            df['DT best model'] = df.apply(lambda row : predict_agg_up(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
        if queue == 'core_down':
            df['DT best model'] = df.apply(lambda row : predict_core_down(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
            
        df['DT accuracy'] = np.where(df['best model'] == df['DT best model'], 1, 0)
        print(t + " " + queue + " DT accracy: ", df['DT accuracy'].mean())
        df['DT latency'] = np.where(df['DT best model'] == 'GEG1', df['GEG1'], df['link delay'])
        df['DT latency error'] = abs(df['DT latency'] - df['latency_sim']) / df['latency_sim'] * 100
        
        if queue == 'client_up':
            df['DT_weight'] = df.apply(lambda row : predict_client_up_weighted(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
        if queue == 'edge_down':
            df['DT_weight'] = df.apply(lambda row : predict_edge_down_weighted(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
        if queue == 'edge_up':
            df['DT_weight'] = df.apply(lambda row : predict_edge_up_weighted(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)        
        if queue == 'agg_down':
            df['DT_weight'] = df.apply(lambda row : predict_agg_down_weighted(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
        if queue == 'agg_up':
            df['DT_weight'] = df.apply(lambda row : predict_agg_up_weighted(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)
        if queue == 'core_down':
            df['DT_weight'] = df.apply(lambda row : predict_core_down_weighted(row['data rate'],  row['rho'], row['rho_total'], row['num_flows'], row['CA_sqr'], row['CS_sqr'],), axis = 1)                                
        
        df['weight_GEG1'] = df['DT_weight'].str[0]
        df['weight_link'] = df['DT_weight'].str[1]
        df['weighted_latency'] = df['weight_GEG1'] * df['GEG1'] + df['weight_link'] * df['link delay']
        df['weighted error'] = abs(df['weighted_latency'] - df['latency_sim']) / df['latency_sim'] * 100
        #print(t + " " + queue + " weighted mape: ", df['weighted error'].mean())
        
        df.to_csv('flowlog_mean_PostDT_' + date +'_' + t + '_' + queue + '.csv', index=False)