import pandas as pd
import numpy as np

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
    data = pd.read_csv('flowlog_all_' + date + '_' + t + '.csv')
    print('Processing ' + t)
    df = pd.DataFrame(data)
    # df = df[(df['GEG1'] >= 0) & (df['GED1'] >= 0)]
    grouped_multiple = df.groupby(['offered data rate', 'src address','des address', 'node ID', 'interface ID', 'stage']). agg({'data rate': ['mean'], 'rho': ['mean'],'rho_total': ['mean'], 'num_flows': ['mean'], 'CA_sqr': ['mean'], 'CS_sqr': ['mean'], 'latency_sim':['mean'], 'GEG1':['mean'], 'GED1':['mean'], 'link delay':['mean']})
    grouped_multiple.columns = ['data rate', 'rho', 'rho_total', 'num_flows', 'CA_sqr', 'CS_sqr', 'latency_sim', 'GEG1', 'GED1', 'link delay']
    grouped_multiple = grouped_multiple[(grouped_multiple['GEG1'] >= 0) & (grouped_multiple['GED1'] >= 0)]
    grouped_multiple.to_csv('flowlog_mean_' + date +'_' + t + '.csv')
    
    
for t in traffic:    
    data = pd.read_csv('flowlog_mean_' + date + '_' + t + '.csv')
    for queue, id in zip(queue_list, id_list):
        df = pd.DataFrame(data)
        print('Processing ' + t + ' ' + queue)
        cond = (df['node ID'] == -1)
        for i in id:
            cond = cond | (df['node ID'] == i)
        df = df[cond]
        if queue == 'edge_down' or queue == 'agg_down':
            cond = (df['interface ID'] == -1)
            for port in down_port_list:
                cond = cond | (df['interface ID'] == port)
            df = df[cond]
        if queue == 'edge_up' or queue == 'agg_up':
            cond = (df['interface ID'] == -1)
            for port in up_port_list:
                cond = cond | (df['interface ID'] == port)
            df = df[cond]
        df.to_csv('flowlog_mean_' + date +'_' + t + '_' + queue + '.csv', index=False)
        

for t in traffic:    
    for queue in queue_list:
        data = pd.read_csv('flowlog_mean_' + date + '_' + t + '_' + queue + '.csv')        
        df = pd.DataFrame(data)
        df['error_GEG1_diff'] = df['GEG1'] - df['latency_sim']
        df['error_GED1_diff'] = df['GED1'] - df['latency_sim']
        df['error_link_diff'] = df['link delay'] - df['latency_sim']
        df['error_GEG1_pct'] = abs(df['error_GEG1_diff']) / df['latency_sim'] * 100
        df['error_GED1_pct'] = abs(df['error_GED1_diff']) / df['latency_sim'] * 100
        df['error_link_pct'] = abs(df['error_link_diff']) / df['latency_sim'] * 100
        df['min_error_pct'] = df[['error_GEG1_pct', 'error_GED1_pct', 'error_link_pct']].min(axis = 1)
        df['best model'] = np.where(df['error_GEG1_pct'] <= df['error_link_pct'], 'GEG1', 'link')
        df['best latency'] = np.where(df['best model'] == 'GEG1', df['GEG1'], df['link delay'])
        df.to_csv('flowlog_mean_DT_' + date +'_' + t + '_' + queue + '.csv', index=False)
    