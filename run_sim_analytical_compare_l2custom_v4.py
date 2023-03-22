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
import multiprocessing

sys.path.append('utils/')
from functions import *

def run_simulator (argument_tuple) :
    ########################################################################################
    ## Create directory structure and a TAG that identifies the current run
    ########################################################################################
    ## Decode arguments
    randomSeed = argument_tuple[0]
    tag        = argument_tuple[1]

    ## Append seed to tag name
    tag = tag + '_s' + str(randomSeed)

    ########################################################################################
    ## Cleanup directories
    ## TOFIX: Clean this up
    ########################################################################################
    os.system("rm -rf statistics/*")
    shutil.rmtree("runs/" + tag, ignore_errors=True)

    ## Create directory structure
    os.makedirs('./runs/' + tag + '/reports_sim', exist_ok=True)
    os.makedirs('./runs/' + tag + '/outputs_sim', exist_ok=True)
    os.makedirs('./runs/' + tag + '/reports_ana', exist_ok=True)
    os.makedirs('./runs/' + tag + '/plots', exist_ok=True)

    ## Serializing JSON for file write
    config_data_object = json.dumps(config_data, indent=4)

    ## Writing JSON to run directory for affirmation
    with open('./runs/' + tag + '/config_file.json', 'w') as outfile:
        outfile.write(config_data_object)
    ## with open('./runs/' + tag + '/config_file.json', 'w') as outfile:

    ########################################################################################
    ## Run NS3 Simulator
    ########################################################################################
    print()
    print('[ INFO] Running NS3 simulator..')
    print('#' * 80)

    ## Pass additional parameters for synthetic traffic
    if 'mimic' not in str(config_data['SimulationParamsInputs']['trafficFile']) :
        additional_parameters = "--netProtocol='ns3::TcpSocketFactory'" 
    else :
        additional_parameters = ""
    ## if 'mimic' not in str(config_data['SimulationParamsInputs']['trafficFile']) :

    ## Set output file name
    output_filename = 'runs/' + tag + '/reports_sim/sim_output.log'

    os.system("./waf --run-no-build=\"" + \
        config_data['SimulationParamsInputs']['programFile'] + " " \
        "--randSeed=" + str(randomSeed) + " " + \
        "--queueSize=" + str(config_data['SimulationParamsInputs']['queueSize']) + " " + \
        "--ecmp=" + str(config_data['DCNStructure']['ecmpRouting']) + " " + \
        "--switchRadix=" + str(config_data['DCNStructure']['switchRadix']) + " " + \
        "--numNodes=" + str(config_data['DCNStructure']['numNodes']) + " " + \
        additional_parameters + " " + \
        "--trafficFile=" + str(config_data['SimulationParamsInputs']['trafficFile']) + " " + \
        "--appTime=" + str(float(config_data['SimulationParamsInputs']['simTime_inms'])/1000) + " " + \
        "--warmup_ms=" + str(float(config_data['SimulationParamsInputs']['warmup_inms'])) + " " + \
        "--simTime=" + str(float(config_data['SimulationParamsInputs']['simTime_inms'])/1000 + 1) + " " + \
        "--window_ms=" + str(config_data['SimulationParamsInputs']['windowTime_inms']) + " " + \
        "--enableLog=" + str(config_data['SimulationParamsInputs']['enableLog']) + " " \
        "--enable_first_pod=" + str(config_data['SimulationParamsInputs']['enable_first_pod']) + " " \
        "--numPods=" + str(config_data['DCNStructure']['numPods']) + " " + \
        "--dataRate_OnOff=" + str(config_data['SimulationParamsInputs']['dataRate_OnOff']) + " " + \
        "--p_burst=" + str(config_data['SimulationParamsInputs']['p_burst']) + " " + \
        "--minPacketSize=" + str(config_data['SimulationParamsInputs']['minPacketSize']) + " " + \
        "--maxPacketSize=" + str(config_data['SimulationParamsInputs']['maxPacketSize']) + " " + \
        "--tag=" + tag + \
    "\" > " + output_filename)

    ## Attach header information to the generated CSV files
    ## Merge simulation and analytical reports
    merge_header(tag, randomSeed, "end_to_end_latency.csv")
    merge_header(tag, randomSeed, "flow_queue_log.csv")

########################################################################################
## Parse arguments
########################################################################################
parser = argparse.ArgumentParser(description='Master script that runs NS3, analytical models, compares and generates plots for analysis', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('-c', dest='config_file_json', type=str, help='Path to configuration JSON file', default='./config_file.json')
args = parser.parse_args()

config_file_json = args.config_file_json

########################################################################################
## Steps performed
########################################################################################
## 1. Read the configuration JSON file
## 2. Print configuration JSON file to screen
## 3. Create the tag that is used to identify the runs
## 4. Run NS3
## 5. Run analytical model based on simulation inputs
## 6. Perform comparisons and generate plots (PENDING!)

########################################################################################
## Read the config JSON file
########################################################################################
with open(config_file_json, 'r') as config_file_json_f :
    config_data = json.load(config_file_json_f)
## with open('./config_file.json', 'r') as config_file_json :

########################################################################################
## Print the configuration to file and screen
########################################################################################
print()
print('[ INFO] Simulation configuration and parameters')
print('#' * 80)
for category in config_data :
    print('**%s**' %(category))
    for parameter in config_data[category] :
        print('%20s : %s' %(parameter, config_data[category][parameter]))
    ## for parameter in config_data[category] :
    print('-' * 80)
## for category in config_data :
print('#' * 80)

## Get random seeds
randomSeeds = list(config_data['SimulationParamsInputs']['randomSeed'])

########################################################################################
## Create directory structure and a TAG that identifies the current run
########################################################################################
## Create tag
program_name = os.path.basename(config_data['SimulationParamsInputs']['programFile'])
tag_comments = config_data['SimulationParamsInputs']['tagComments']

## If no tag comment is specified, let's use "none"
tag_comments = 'none' if (tag_comments == "") else tag_comments

tag = program_name + '_q' + str(config_data['SimulationParamsInputs']['queueSize']) + \
    '_w' + str(config_data['SimulationParamsInputs']['windowTime_inms']) + \
    '_t' + str(tag_comments)

## Create tuples of arguments for run simulator
argument_tuples = []
for index in range(len(randomSeeds)) :
    argument_tuples.append([randomSeeds[index], tag])
## for index in range(len(randomSeeds)) :

########################################################################################
## Launch parallel simulations using multiprocessing
########################################################################################
## ## Pre-compile the code before launching multiprocessing script to ensure we don't break
## os.system("./waf")
## 
## ## Compile the analytical model code before launching multiprocessing script to ensure binaries are not recompiled
## os.chdir('./auto_oop')  ## Navigate into the analytical model directory
## os.system('make')  ## Compile the analytical models before running them
## os.chdir('../')  ## Navigate back into the parent directory

## Launch the simulations in parallel using multiprocessing
p = multiprocessing.Pool(5)
p.map(run_simulator, argument_tuples)

########################################################################################
## Merge reports from multiple seeds
########################################################################################
merged_tag = 'runs/' + tag + '_merged'
os.makedirs(merged_tag, exist_ok=True)
os.makedirs(merged_tag + '/reports_others', exist_ok=True)

merge_seeds(tag, merged_tag, randomSeeds)

########################################################################################
## Run analytical model based on inputs provided from simulation
########################################################################################
print()
print('[ INFO] Running analytical models..')
print('#' * 80)
os.chdir('./auto_oop')  ## Navigate into the analytical model directory
os.system("./analytical_model " + \
    "-s " + str(config_data['SimulationParamsInputs']['simTime_inms']) + " " \
    "-w " + str(config_data['SimulationParamsInputs']['windowTime_inms']) + " " \
    "-m " + str(config_data['SimulationParamsInputs']['warmup_inms']) + " " \
    "-q " + str(config_data['SimulationParamsInputs']['queueSize']) + " " + \
    "-n " + str(config_data['DCNStructure']['numPods']) + " " \
    "-k " + str(config_data['DCNStructure']['switchRadix']) + " " \
    "-v " + str(config_data['DCNStructure']['numNodes']) + " " \
    "-y " + str(config_data['DCNStructure']['topology']) + " " \
    "-t " + tag + "_merged " \
)
os.chdir('../')  ## Navigate back into the parent directory

## Post-process simulation and analytical reports to create a merged report of data for analysis
print()
print('[ INFO] Post-processing simulation and analytical model outputs to create merged reports')
print('#' * 80)
sim_flow_filename  = merged_tag + '/reports_sim/end_to_end_latency.csv'
sim_queue_filename = merged_tag + '/reports_sim/flow_queue_log.csv'
ana_flow_filename  = merged_tag + '/reports_ana/latency_per_flow.csv'
ana_queue_filename = merged_tag + '/reports_ana/latency_per_queue.csv'

latency_per_flow_filename  = merged_tag + '/reports_ana/latency_per_flow_merged.csv'
latency_per_queue_filename = merged_tag + '/reports_ana/latency_per_queue_merged.csv'

merge_latency_per_flow(tag, sim_flow_filename, ana_flow_filename, latency_per_flow_filename)
merge_latency_per_queue(tag, sim_queue_filename, ana_queue_filename, latency_per_queue_filename)

print('[ INFO] Output file containing latency per flow: ' + latency_per_flow_filename)
print('[ INFO] Output file containing latency per queue: ' + latency_per_queue_filename)
