import re
import os
import sys
import glob
import json
import argparse
import shutil
import numpy as np
import pandas as pd

sys.path.append('utils/')
from functions import *

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
## 1. Read the config JSON file
########################################################################################
with open(config_file_json, 'r') as config_file_json_f :
    config_data = json.load(config_file_json_f)
## with open('./config_file.json', 'r') as config_file_json :

########################################################################################
## 2. Print the configuration to file and screen
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
    
########################################################################################
## 3. Create directory structure and a TAG that identifies the current run
########################################################################################
## Create tag
program_name = os.path.basename(config_data['SimulationParamsInputs']['programFile'])
tag_comments = config_data['SimulationParamsInputs']['tagComments']

## If no tag comment is specified, let's use "none"
tag_comments = 'none' if (tag_comments == "") else tag_comments

tag = program_name + '_q' + str(config_data['SimulationParamsInputs']['queueSize']) + \
      '_w' + str(config_data['SimulationParamsInputs']['windowTime_inms']) + \
      '_t' + str(tag_comments)

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
## 4. Run NS3 Simulator
########################################################################################
print()
print('[ INFO] Running NS3 simulator..')
print('#' * 80)
os.system("./waf --run=\"" + \
      config_data['SimulationParamsInputs']['programFile'] + " " \
      "--randSeed=" + str(config_data['SimulationParamsInputs']['randomSeed']) + " " + \
      "--queueSize=" + str(config_data['SimulationParamsInputs']['queueSize']) + " " + \
      "--ecmp=" + str(config_data['DCNStructure']['ecmpRouting']) + " " + \
      "--trafficFile=" + str(config_data['SimulationParamsInputs']['trafficFile']) + " " + \
      "--simTime=" + str(config_data['SimulationParamsInputs']['simTime_inms']) + " " + \
      "--appTime=" + str(config_data['SimulationParamsInputs']['simTime_inms']) + " " + \
      "--enableLog=" + str(config_data['SimulationParamsInputs']['enableLog']) + " " \
      "--dataRate_OnOff=" + str(config_data['SimulationParamsInputs']['dataRate_OnOff']) + " " + \
      "--switchRadix=" + str(config_data['DCNStructure']['switchRadix']) + " " + \
      "--tag=" + tag + \
"\"")

## Attach header information to the generated CSV files
merge_header(tag, "end_to_end_latency.csv")
merge_header(tag, "flow_queue_log.csv")


"""
########################################################################################
## 5. Run analytical model based on inputs provided from simulation
########################################################################################
print()
print('[ INFO] Running analytical models..')
print('#' * 80)
os.chdir('./auto_oop')  ## Navigate into the analytical model directory
os.system('make')  ## Compile the analytical models before running them
os.system("./analytical_model " + \
      "-s " + str(config_data['SimulationParamsInputs']['simTime_inms']) + " " \
      "-w " + str(config_data['SimulationParamsInputs']['windowTime_inms']) + " " \
      "-q " + str(config_data['SimulationParamsInputs']['queueSize']) + " " + \
      "-n " + str(config_data['DCNStructure']['numPods']) + " " \
      "-t " + tag + " " \
)
os.chdir('../')  ## Navigate back into the parent directory

## Post-process simulation and analytical reports to create a merged report of data for analysis
print()
print('[ INFO] Post-processing simulation and analytical model outputs to create merged reports')
print('#' * 80)
sim_flow_filename  = 'runs/' + tag + '/reports_sim/end_to_end_latency.csv'
sim_queue_filename = 'runs/' + tag + '/reports_sim/flow_queue_log.csv'
ana_flow_filename  = 'runs/' + tag + '/reports_ana/latency_per_flow.csv'
ana_queue_filename = 'runs/' + tag + '/reports_ana/latency_per_queue.csv'

latency_per_flow_filename  = 'runs/' + tag + '/reports_ana/latency_per_flow_merged.csv'
latency_per_queue_filename = 'runs/' + tag + '/reports_ana/latency_per_queue_merged.csv'

merge_latency_per_flow(tag, sim_flow_filename, ana_flow_filename, latency_per_flow_filename)
merge_latency_per_queue(tag, sim_queue_filename, ana_queue_filename, latency_per_queue_filename)

print('[ INFO] Output file containing latency per flow: ' + latency_per_flow_filename)
print('[ INFO] Output file containing latency per queue: ' + latency_per_queue_filename)
"""
