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
# import multiprocessing
import billiard as multiprocessing

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
from analytical_compare import *

########################################################################################
## Parse arguments
########################################################################################
parser = argparse.ArgumentParser(description='Script that builds dataset using a list of configuration files', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('-l', dest='list_configs', type=str, help='List of configuration JSON file', default='./synthetic_configs_test')
args = parser.parse_args()
list_configs = args.list_configs

########################################################################################
## Read file, iterate over it
########################################################################################
with open(list_configs, 'r') as f_list_configs :
    configs = list(f_list_configs.readlines())
## with open(list_configs, 'r') as f_list_configs :

## We randomly pick 60% of the configurations to train
sampled_configs = random.sample(configs, int(1 * len(configs)))  

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
    # latency_per_flow_merged_path = dir + '/reports_ana/latency_per_queue_merged.csv'
    # latency_per_flow_merged_rtt_filename = latency_per_flow_merged_path.replace('queue_merged.csv', 'flow_merged.csv.rtt')
    latency_per_flow_merged_rtt_filename = dir + '/latency_per_flow_merged.csv.rtt'
    latency_per_flow_merged_rtt_filenames.append(latency_per_flow_merged_rtt_filename)
## for config_file_index, config_file_line in enumerate(sampled_configs) :

    
## for line in sampled_configs :

## Write out the file
## full_data.to_csv('data.csv', index=False)

########################################################################################
## Training the regression model
########################################################################################
## Set directory names
# timestr        = time.strftime("%Y%m%d-%H%M%S")
timestr = "20230303-111813"

output_dirname = './runs/dataset_' + timestr
os.makedirs(output_dirname, exist_ok=True)


## Open file handle to a file that will contain all regression model functions for each of the queue-types
model_CPP_filename = output_dirname + '/' + 'regression_tree_model.cpp'

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
    analytical_compare(config_file)
    return
## def run_analytical(config_file) :

with multiprocessing.Pool(8) as p :
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
    os.system("python3 utils/plotting_rtt.py " + latency_per_flow_merged_rtt_filename + " >> " + evaluation_filename)
## for file in latency_per_flow_merged_rtt_filenames :
