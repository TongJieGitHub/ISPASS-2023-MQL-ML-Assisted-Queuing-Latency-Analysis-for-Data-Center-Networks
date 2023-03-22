## Import libraries
import os
import re
import math
import shutil
import random
import scipy.io
import numpy as np
import pandas as pd                
import matplotlib.pyplot as plt
import sklearn
import pickle

## Import libraries
from sklearn.model_selection import train_test_split  
from sklearn.tree import DecisionTreeRegressor      
from sklearn.tree import DecisionTreeClassifier      
from sklearn.preprocessing import StandardScaler
from sklearn import datasets
from sklearn import tree
from sklearn.linear_model import LinearRegression
from sklearn.feature_selection import SelectKBest, SelectPercentile, f_regression, mutual_info_regression
from sklearn.ensemble import RandomForestRegressor
from sklearn.preprocessing import RobustScaler
import pydotplus
import dill
import m2cgen
from xgboost import XGBRegressor

## Specify set of input files
files = [ \
#        'runs/dcn_fattree_finite_large_v3_q128_w5000_t128x128alltoall_pkt_500_r_0.3_merged/latency_per_queue_merged.csv'
#        'runs/dcn_fattree_finite_large_v3_q128_w5000_t128x128alltoall_pkt_500_r_0.05_s11/reports_ana/latency_per_queue_merged.csv'
#        'runs/dcn_fattree_finite_large_v3_q128_w30000_tsize128_alltoall_disPoisson_packetFixed_dr0p2_s4/reports_ana/latency_per_queue_merged.csv'
        'runs/dcn_fattree_finite_large_v3_q128_w30000_tsize16_alltoall_merged/reports_ana/latency_per_queue_merged.csv'
]

## These features are selected by systematic feature selection methods
feature_mask_cfs = {
    'edge.*up'   : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i', 'dr_1_rho_ana_i'],
    'aggr.*up'   : ['CA_sqr_ana'],
    'core.*down' : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i'],
    'aggr.*down' : ['rho_total_ana', 'packet_size_ana', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['datarate_ana', 'rho_total_ana', 'packet_size_ana', '1_rhoT_ana_i', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_cfs_visual = {
    'edge.*up'   : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i'],
    'aggr.*up'   : ['datarate_ana', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i'],
    'aggr.*down' : ['datarate_ana', 'rho_total_ana', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_FTest = {
    'edge.*up'   : ['CS_sqr_ana', 'packet_size_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'aggr.*up'   : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'aggr.*down' : ['CS_sqr_ana', 'packet_size_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['CS_sqr_ana', 'packet_size_ana', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_mutual_info_reg_visual = {
    'edge.*up'   : ['rho_total_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'aggr.*up'   : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'aggr.*down' : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_mutual_info_reg = {
    'edge.*up'   : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'aggr.*up'   : ['rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['rho_total_ana', '1_rhoT_ana_i', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'aggr.*down' : ['rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_mutual_info_reg_visual = {
    'edge.*up'   : ['packet_size_ana', 'rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'aggr.*up'   : ['packet_size_ana', 'rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['packet_size_ana', 'rho_total_ana', '1_rhoT_ana_i', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'aggr.*down' : ['packet_size_ana', 'rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['packet_size_ana', 'rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}

## Specify type of packets
packet_types = {'small': 100, 'large': 200}

## Analytical model features
all_features = ['datarate_ana','rho_ana','rho_total_ana','CA_sqr_ana','CS_sqr_ana','packet_size_ana','1_rho_ana_i','1_rhoT_ana_i', 'dr_1_rho_ana_i','dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i']

## Simulation related features
## Only for debugging purposes
## all_features = ['meanDatarateArrival','rho','rho_total','CA_sqr','CS_sqr','meanPacketSize','1_rho_ana_i','1_rhoT_ana_i', 'dr_1_rho_ana_i','dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i', 'correction_inf']

## Open file handle to output file for reports
output_file = open(os.path.dirname(files[0]) + '/regression_model_reports.rpt', 'w')

## Iterate over each of the files
for file in files :

    ## Print filename
    print('#' * 100)
    print(file)
    print('#' * 100)

    ## Print headers
    print('%-15s %10s %20s %20s' %('Queue', 'PacketType', 'ME-Model-MAPE(%)', 'ME+Reg-MAPE(%)'))
    output_file.write('%-15s %10s %20s %20s\n' %('Queue', 'PacketType', 'ME-Model-MAPE(%)', 'ME+Reg-MAPE(%)'))

    ## Open file handle to a file that will contain all regression model functions for each of the queue-types
    model_CPP_filename = os.path.dirname(file) + '/' + 'regression_tree_model.cpp'
    f = open(model_CPP_filename, 'w')
    f.close()

    ## Iterate over packet types
    for packet_type in packet_types :

        ## Read CSV
        df = pd.read_csv(file, header=0, low_memory=False)

        ## Add columns with certain computations
        df.loc[:, ('queuing_delay_sim')] = df.loc[:, ('latency_sim')] - df.loc[:, ('link_delay_sim')]
        df.loc[:, ('queuing_delay_ana')] = df.loc[:, ('latency_inf')] - df.loc[:, ('link_delay_ana')]
        df.loc[:, ('ME_modeling_error')] = df.loc[:, ('queuing_delay_sim')] - df.loc[:, ('queuing_delay_ana')]

        ## Specifying the queues of interest
        queues = ['edge.*up', 'aggr.*up', 'core.*down', 'aggr.*down', 'edge.*down']

        ## Retain a copy of the original dataframe
        df_orig = df.copy()

        ## Create an empty dataframe that will aggregate data from the different queues
        full_df = pd.DataFrame(columns=df_orig.columns)

        ## Iterate over each queue type
        for queue in queues :

            ## Get data only for the particular queue and the packet type
            if 'small' in packet_type :
                df = df_orig[(df_orig['queue'].str.contains(queue)) & (df_orig['meanPacketSize'] < packet_types[packet_type])].copy()
            elif 'large' in packet_type :
                df = df_orig[(df_orig['queue'].str.contains(queue)) & (df_orig['meanPacketSize'] > packet_types[packet_type])].copy()
            else :
                print('[ERROR] Incorrect packet type. Packet type can be either `small` or `large` referring to `ACK` and `DATA` packets respectively.')
                exit(1)
            ## if 'small' in packet_type :

            ################################################
            ## Add few terms to the dataframe if using analytical features
            ################################################
            ## Add 1 / (1 - rho)
            df.loc[:, ('1_rho_ana_i')] = 1 / (1 - df.loc[:, ('rho_ana')])

            ## Add 1 / (1 - rho_total)
            df.loc[:, ('1_rhoT_ana_i')] = 1 / (1 - df.loc[:, ('rho_total_ana')])

            ## Add datarate / (1 - rho)
            df.loc[:, ('dr_1_rho_ana_i')] = df.loc[:, ('datarate_ana')] / (1 - df.loc[:, ('rho_ana')])

            ## Add datarate / (1 - rho_total)
            df.loc[:, ('dr_1_rhoT_ana_i')] = df.loc[:, ('datarate_ana')] / (1 - df.loc[:, ('rho_total_ana')])

            ## Add occupancy related term into the dataframe
            df.loc[:, ('CA_sqr_CS_sqr_1_rho_i')] = (df.loc[:, ('CA_sqr_ana')] + df.loc[:, ('CS_sqr_ana')]) / (1 - df.loc[:, ('rho_total_ana')])

            ##  ################################################
            ##  ## Add few terms to the dataframe if using simulation features
            ##  ################################################
            ##  ## Add 1 / (1 - rho)
            ##  df.loc[:, ('1_rho_ana_i')] = 1 / (1 - df.loc[:, ('rho')])

            ##  ## Add 1 / (1 - rho_total)
            ##  df.loc[:, ('1_rhoT_ana_i')] = 1 / (1 - df.loc[:, ('rho_total')])

            ##  ## Add datarate / (1 - rho)
            ##  df.loc[:, ('dr_1_rho_ana_i')] = df.loc[:, ('meanDatarateArrival')] / (1 - df.loc[:, ('rho')])

            ##  ## Add datarate / (1 - rho_total)
            ##  df.loc[:, ('dr_1_rhoT_ana_i')] = df.loc[:, ('meanDatarateArrival')] / (1 - df.loc[:, ('rho_total')])

            ##  ## Add occupancy related term into the dataframe
            ##  df.loc[:, ('CA_sqr_CS_sqr_1_rho_i')] = (df.loc[:, ('CA_sqr')] + df.loc[:, ('CS_sqr')]) / (1 - df.loc[:, ('rho_total')])

            ################################################

            ## Get the columns that have the potential to form the input features
            df_for_train_all = df.loc[:, all_features].copy()

            print('%-15s %10s ' %(queue, packet_type), end='')
            output_file.write('%-15s %10s ' %(queue, packet_type))

            ## Leave each feature out one-by-one
            ## -1 means we don't drop any feature
            ## -2 means we'll use the features systematically selected by feature selection methods
            ## for feature_index in range(-1, df_for_train_all.shape[1] - 1, 1) :
            ## for feature_index in range(-2, 0, 1) :
            ## This piece of code is little messed up because we don't do the leave-one-out selection anymore
            for feature_index in [-1] :

                ## If feature index is -1, we don't drop any feature
                if feature_index == -2 :
                    df_for_train  = df_for_train_all.loc[:, feature_mask_mutual_info_reg_visual[queue]].copy()
                    feature_names = feature_mask_mutual_info_reg_visual[queue]
                elif feature_index != -1 :
                    df_for_train = df_for_train_all.drop(columns=df_for_train_all.columns[feature_index]).copy()
                else :
                    df_for_train  = df_for_train_all.copy()
                    feature_names = all_features
                ## if feature_index != -1 :

                ## Add the target to the dataframe
                df_for_train.loc[:, ('correction_sim_inf')] = df.loc[:, ('correction_sim_inf')]

                ## Extract data and fit to the model
                data     = df_for_train.copy()
                features = data.iloc[:, 0:data.shape[1] - 1]
                feature_columns = features.columns
                values   = data.iloc[:, -1]

                ## Scale features to improve ML model learning
                ## scaler   = RobustScaler()
                ## features = scaler.fit_transform(features)
                ## features = pd.DataFrame(features, columns=feature_columns)

                ## Initialize a regression tree model
                model = DecisionTreeRegressor(max_depth=12)
                ## model = LinearRegression()
                ## model = RandomForestRegressor(n_estimators = 100, max_depth = 12)
                ## model = XGBRegressor(n_estimators=250, max_depth=12, eta=0.1, subsample=0.7, colsample_bytree=0.8)
                model.fit(features, values)

                ## Save the regression model as a pickle file
                queue_name = queue.replace('.*', '_')
                model_pkl_filename = os.path.dirname(file) + '/q_' + queue_name + '_p_' + packet_type + '.pkl'
                with open(model_pkl_filename, 'wb') as f :
                    f = dill.dump(model, f)
                ## with open(model_pkl_filename, 'wb') as f :

                ## Translate the pickle file to a C/C++ function using the m2cgen module
                if feature_index == -1 :
                    model_CPP_filename = os.path.dirname(file) + '/' + 'regression_tree_model.cpp'
                    with open(model_CPP_filename, 'a') as f :
                        f.write(m2cgen.export_to_c(model, function_name=queue_name + '_' + packet_type))
                ## with open(model_CPP_filename, 'w') as f :

                ## Perform feature selection using K-Best algorithm and print out the selected features
                ## if feature_index == -1 :
                ##     selector = SelectKBest(mutual_info_regression, k=5)
                ##     features_reduced = selector.fit_transform(features, values)
                ##     cols = selector.get_support(indices=True)
                ##     selected_columns = features.iloc[:,cols].columns.tolist()
                ##     print(selected_columns)
                ## if feature_index == -1 :

                ## Get estimates from the model
                predicted_values = model.predict(features)

                ## Add a column to the dataframe with the estimated latency to add to the ME model
                df.loc[:, ('model_predicted_error')] = predicted_values

                ## Get total analytical latency
                predicted_queuing_latency = predicted_values + df['latency_inf']
                df.loc[:, ('new_analytical_latency')] = predicted_queuing_latency

                ## Get MAPE with respect to simulation latency
                mape_latency = np.abs(predicted_queuing_latency - df['latency_sim']) / df['latency_sim'] * 100
                df.loc[:, ('mape_latency_new')] = mape_latency

                mape_latency_old = np.abs(df['latency_inf'] - df['latency_sim']) / df['latency_sim'] * 100
                df.loc[:, ('mape_latency_old')] = mape_latency_old

                ana_model_latency = np.mean(mape_latency)

                if feature_index == -1 :
                    ME_model_latency = np.mean(mape_latency_old)
                    print('%20.2f %20.2f' %(ME_model_latency, ana_model_latency), end='')
                    output_file.write('%20.2f %20.2f' %(ME_model_latency, ana_model_latency))
                else :
                    print(' %20.2f' %(ana_model_latency), end='')
                    output_file.write(' %20.2f' %(ana_model_latency))

                ## Perform data manipulations only when all features are included
                if feature_index == -1 :
                    ## Aggregate data
                    full_df = pd.concat([full_df, df], axis=0)

                    ## Write the particular queue data to a separate CSV
                    df_for_train.to_csv(os.path.dirname(file) + '/training_data_' + queue_name + '_' + packet_type + '.csv', index=False)
                    df.to_csv(os.path.dirname(file) + '/regression_data_' + queue_name + '_' + packet_type + '.csv', index=False)
                ## if feature_index == -1 :

            ## for feature_index in range(df_for_train.shape[1] - 1) :

            print()
            output_file.write('\n')

        ## for queue in queues :

    ## for packet_type in packet_types :

    ## Write out the aggregated data frame to the CSV
    ## full_df.to_csv(os.path.dirname(file) + '/aftermodel_latency_per_queue_merged.csv')

## for file in files :

## Close file handle to output file for reports
output_file.close()
