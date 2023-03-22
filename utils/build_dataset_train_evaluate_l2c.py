import re
import os
import sys
import glob
import json
import argparse
import shutil
import numpy as np
import pandas as pd
import contextlib
import random
random.seed(1)
import time
import multiprocessing

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

sys.path.append(os.path.abspath('./'))
from analytical_compare_l2custom import *

########################################################################################
## Parse arguments
########################################################################################
parser = argparse.ArgumentParser(description='Script that builds dataset using a list of configuration files', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('-l', dest='list_configs', type=str, help='List of configuration JSON file', default='./synthetic_configs')
args = parser.parse_args()
list_configs = args.list_configs

########################################################################################
## Read file, iterate over it
########################################################################################
with open(list_configs, 'r') as f_list_configs :
    configs = list(f_list_configs.readlines())
## with open(list_configs, 'r') as f_list_configs :

## We randomly pick 60% of the configurations to train
sampled_configs = random.sample(configs, int(0.6 * len(configs)))  

# configs.append('configFiles/config_size1024_anarchy.json')
# configs.append('configFiles/config_size128_anarchy.json')
# configs.append('configFiles/config_size16_anarchy.json')
# configs.append('configFiles/config_size432_anarchy.json')

## Add particular configurations of interest
## sampled_configs = [i for i in sampled_configs if 'imc' not in i]
## sampled_configs.extend([i for i in configs if 'imc' in i])
## sampled_configs = [i for i in sampled_configs if 'facebook' not in i]
## sampled_configs.extend([i for i in configs if 'facebook' in i])
## sampled_configs.append('configFiles/config_size128_alltoall_disGE_packetUniform_dr0p59.json')
## sampled_configs.append('configFiles/config_size432_incast_disPoisson_packetUniform_dr0p12.json')
## sampled_configs.append('configFiles/config_size432_incast_disPoisson_packetFixed_dr0p12.json')
## sampled_configs.append('configFiles/config_size1024_incast_disPoisson_packetFixed_dr0p02.json')
# sampled_configs.append('configFiles/config_size1024_incast_disGE_packetFixed_dr0p07.json')
# sampled_configs.append('configFiles/config_size1024_incast_disGE_packetUniform_dr0p07.json')
# sampled_configs.append('configFiles/config_size128_alltoall_disPoisson_packetUniform_dr0p59.json')
# sampled_configs.append('configFiles/config_size128_alltoall_disPoisson_packetFixed_dr0p59.json')
# sampled_configs.append('configFiles/config_size432_incast_disGE_packetFixed_dr0p06.json')
# sampled_configs.append('configFiles/config_size432_incast_disGE_packetUniform_dr0p06.json')
# sampled_configs.append('configFiles/config_size1024_incast_disGE_packetUniform_dr0p02.json')
# sampled_configs.append('configFiles/config_size1024_incast_disGE_packetFixed_dr0p02.json')
# sampled_configs.append('configFiles/config_size128_incast_disGE_packetFixed_dr0p59.json')
# sampled_configs.append('configFiles/config_size1024_anarchy.json')
# sampled_configs.append('configFiles/config_size128_anarchy.json')
# sampled_configs.append('configFiles/config_size16_anarchy.json')
# sampled_configs.append('configFiles/config_size432_anarchy.json')

## Initialize a list for filenames
latency_per_flow_merged_rtt_filenames = []

## Get list of config filenames
for config_file_index, config_file_line in enumerate(configs) :
    config_file_json = config_file_line.strip()

    ########################################################################################
    ## Read the config JSON file
    ########################################################################################
    with open(config_file_json, 'r') as config_file_json_f :
        config_data = json.load(config_file_json_f)
    ## with open('./config_file.json', 'r') as config_file_json :

    ## Create tag
    program_name = os.path.basename(config_data['SimulationParamsInputs']['programFile'])
    tag_comments = config_data['SimulationParamsInputs']['tagComments']

    ## If no tag comment is specified, let's use "none"
    tag_comments = 'none' if (tag_comments == "") else tag_comments

    tag = program_name + '_q' + str(config_data['SimulationParamsInputs']['queueSize']) + \
        '_w' + str(config_data['SimulationParamsInputs']['windowTime_inms']) + \
        '_t' + str(tag_comments)

    ## Get merged directory name
    dir = './runs/' + tag + '_merged'

    ## Get latency_per_flow_merged.csv path
    latency_per_flow_merged_path = dir + '/reports_ana/latency_per_queue_merged.csv'
    latency_per_flow_merged_rtt_filename = latency_per_flow_merged_path.replace('queue_merged.csv', 'flow_merged.csv.rtt')
    latency_per_flow_merged_rtt_filenames.append(latency_per_flow_merged_rtt_filename)
## for config_file_index, config_file_line in enumerate(sampled_configs) :

########################################################################################
## Read the individual files and created a merged dataframe
########################################################################################
for config_file_index, config_file_line in enumerate(sampled_configs) :
    config_file_json = config_file_line.strip()

    ########################################################################################
    ## Read the config JSON file
    ########################################################################################
    with open(config_file_json, 'r') as config_file_json_f :
        config_data = json.load(config_file_json_f)
    ## with open('./config_file.json', 'r') as config_file_json :

    ## Create tag
    program_name = os.path.basename(config_data['SimulationParamsInputs']['programFile'])
    tag_comments = config_data['SimulationParamsInputs']['tagComments']

    ## If no tag comment is specified, let's use "none"
    tag_comments = 'none' if (tag_comments == "") else tag_comments

    tag = program_name + '_q' + str(config_data['SimulationParamsInputs']['queueSize']) + \
        '_w' + str(config_data['SimulationParamsInputs']['windowTime_inms']) + \
        '_t' + str(tag_comments)

    ## Get merged directory name
    dir = './runs/' + tag + '_merged'

    ## Check if file exists
    if not os.path.exists(latency_per_flow_merged_path) :
        print('[ERROR] File - ' + latency_per_flow_merged_path + ' does not exist. Exiting...')
        exit()
    ## if not os.path.exists(latency_per_flow_merged_path) :

    ## Get latency_per_flow_merged.csv path
    latency_per_flow_merged_path = dir + '/reports_ana/latency_per_queue_merged.csv'

    ## Read CSV
    data = pd.read_csv(latency_per_flow_merged_path)

    sample_data = data.copy()

    ## Create a master dataframe when at the first file
    if config_file_index == 0 :
        full_data = sample_data.copy()
    else :
        full_data = pd.concat([full_data, sample_data], axis=0)
    ## if index == 0 :
    print('fulldata')
    
## for line in sampled_configs :

## Write out the file
## full_data.to_csv('data.csv', index=False)

########################################################################################
## Training the regression model
########################################################################################
## Set directory names
timestr        = time.strftime("%Y%m%d-%H%M%S")

output_dirname = './runs/dataset_' + timestr
os.makedirs(output_dirname, exist_ok=True)

## Open file handle to output file for reports
output_file = open(output_dirname + '/regression_model_reports.rpt', 'w')

## Print headers
print('%-15s %10s %20s %20s' %('Queue', 'PacketType', 'ME-Model-MAPE(%)', 'ME+Reg-MAPE(%)'))
output_file.write('%-15s %10s %20s %20s\n' %('Queue', 'PacketType', 'ME-Model-MAPE(%)', 'ME+Reg-MAPE(%)'))

## Open file handle to a file that will contain all regression model functions for each of the queue-types
model_CPP_filename = output_dirname + '/' + 'regression_tree_model.cpp'
model_pkl_filename = output_dirname + '/' + 'regression_tree_model.pkl'
f = open(model_CPP_filename, 'w')
f.close()

## Specify type of packets
packet_types = {'small': 100, 'large': 200}

## Analytical model features
all_features = ['datarate_ana','rho_ana','rho_total_ana','CA_sqr_ana','CS_sqr_ana','packet_size_ana','1_rho_ana_i','1_rhoT_ana_i', 'dr_1_rho_ana_i','dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i']

## These features are selected by systematic feature selection methods
feature_mask_cfs = {
    'edge.*up'   : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i', 'dr_1_rho_ana_i'],
#    'aggr.*up'   : ['CA_sqr_ana'],
    'core.*down' : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i'],
#    'aggr.*down' : ['rho_total_ana', 'packet_size_ana', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['datarate_ana', 'rho_total_ana', 'packet_size_ana', '1_rhoT_ana_i', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_cfs_visual = {
    'edge.*up'   : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i'],
#    'aggr.*up'   : ['datarate_ana', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i'],
#    'aggr.*down' : ['datarate_ana', 'rho_total_ana', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['datarate_ana', 'rho_total_ana', '1_rho_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_FTest = {
    'edge.*up'   : ['CS_sqr_ana', 'packet_size_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
#    'aggr.*up'   : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
#    'aggr.*down' : ['CS_sqr_ana', 'packet_size_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['CS_sqr_ana', 'packet_size_ana', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_mutual_info_reg_visual = {
    'edge.*up'   : ['rho_total_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
#    'aggr.*up'   : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
#    'aggr.*down' : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_mutual_info_reg = {
    'edge.*up'   : ['CA_sqr_ana', '1_rho_ana_i', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
#    'aggr.*up'   : ['rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['rho_total_ana', '1_rhoT_ana_i', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
#    'aggr.*down' : ['rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}
feature_mask_mutual_info_reg_visual = {
    'edge.*up'   : ['packet_size_ana', 'rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
#    'aggr.*up'   : ['packet_size_ana', 'rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'core.*down' : ['packet_size_ana', 'rho_total_ana', '1_rhoT_ana_i', 'dr_1_rho_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
#es:l2c    'aggr.*down' : ['packet_size_ana', 'rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
    'edge.*down' : ['packet_size_ana', 'rho_total_ana', 'CA_sqr_ana', '1_rhoT_ana_i', 'dr_1_rhoT_ana_i', 'CA_sqr_CS_sqr_1_rho_i'],
}

## Iterate over packet types and train a regression model for each type
for packet_type in packet_types :

    ## Add columns with certain computations
    ## full_data.loc[:, ('queuing_delay_sim')] = full_data.loc[:, ('latency_sim')] - full_data.loc[:, ('link_delay_sim')]
    ## full_data.loc[:, ('queuing_delay_ana')] = full_data.loc[:, ('latency_inf')] - full_data.loc[:, ('link_delay_ana')]
    ## full_data.loc[:, ('ME_modeling_error')] = full_data.loc[:, ('queuing_delay_sim')] - full_data.loc[:, ('queuing_delay_ana')]

    ## Specifying the queues of interest
#es:l2c    queues = ['edge.*up', 'aggr.*up', 'core.*down', 'aggr.*down', 'edge.*down']
    queues = ['edge.*up', 'core.*down', 'edge.*down']

    ## Retain a copy of the original dataframe
    df_orig = full_data.copy()

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
            queue_model_pkl_filename = output_dirname + '/q_' + queue_name + '_p_' + packet_type + '.pkl'
            with open(model_pkl_filename, 'wb') as f :
                f = dill.dump(model, f)
            ## with open(model_pkl_filename, 'wb') as f :

            ## Translate the pickle file to a C/C++ function using the m2cgen module
            if feature_index == -1 :
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
                df_for_train.to_csv(output_dirname + '/training_data_' + queue_name + '_' + packet_type + '.csv', index=False)
                df.to_csv(output_dirname + '/regression_data_' + queue_name + '_' + packet_type + '.csv', index=False)
            ## if feature_index == -1 :

        ## for feature_index in range(df_for_train.shape[1] - 1) :

        print()
        output_file.write('\n')

    ## for queue in queues :
## for packet_type in packet_types :

########################################################################################
## Copying new model into auto_oop directory and compile analytical models
########################################################################################
shutil.copy(model_CPP_filename, './auto_oop/')
os.chdir('./auto_oop')  ## Navigate into the analytical model directory
os.system('make')  ## Compile the analytical models before running them
os.chdir('../')  ## Navigate back into the parent directory

########################################################################################
## Running analytical models to evaluate performance
########################################################################################
def run_analytical(config_file) :
    config_file = config_file.strip()
    analytical_compare_l2custom(config_file)
    return
## def run_analytical(config_file) :

with multiprocessing.Pool(40) as p :
    p.map(run_analytical, configs)
## with multiprocessing.Pool(12) as p :

########################################################################################
## Running plotting_rtt script to generate final scripts
########################################################################################
## Temporarily redirect output to file
evaluation_filename = output_dirname + '/evaluation_summary.rpt'
evaluation_file = open(evaluation_filename, 'w')

## Close file handle
evaluation_file.close()

for latency_per_flow_merged_rtt_filename in latency_per_flow_merged_rtt_filenames :
    os.system("python utils/plotting_rtt.py " + latency_per_flow_merged_rtt_filename + " >> " + evaluation_filename)
## for file in latency_per_flow_merged_rtt_filenames :
