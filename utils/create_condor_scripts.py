## Import libraries
import os
import re
import sys
import math
import glob
import shutil
import random
import scipy.io


## Create run script directory
run_script_dir = './scripts/'
os.makedirs(run_script_dir, exist_ok=True)

## Get all config files
## Get command line argument for directory of config files
config_files = glob.glob(sys.argv[1] + '/config*json')

## Master script to launch all runs
master_script = open('./run_script.sh', 'w')

## Iterate over all config files
for config_file in config_files :

    ## Get filename
    config_filename = os.path.basename(config_file)
    config_filename = config_filename.replace('.json', '')

    ## Create shell script file name 
    shell_script_filename  = run_script_dir + '/run_' + config_filename + '.sh'
    submit_script_filename = run_script_dir + '/run_' + config_filename + '.sub'

    ## Create shell script file handles 
    shell_script  = open(shell_script_filename, 'w')
    submit_script = open(submit_script_filename, 'w')

    shell_script.write('#!/bin/bash\n')
    shell_script.write('\n')
    shell_script.write('# untar your Python installation. Make sure you are using the right version!\n')
    shell_script.write('#transfer_input_files = http://proxy.chtc.wisc.edu/SQUID/chtc/el8/python310.tar.gz, packages.tar.gz, ns-3.35-dcn.tar.gz\n')
    shell_script.write('tar -xzf python310.tar.gz\n')
    shell_script.write('# (optional) if you have a set of packages (created in Part 1), untar them also\n')
    shell_script.write('tar -xzf packages.tar.gz\n')
    shell_script.write('tar -xzf ns-3.35-dcn.tar.gz\n')
    shell_script.write('\n')
    shell_script.write('# make sure the script will use your Python installation, \n')
    shell_script.write('# and the working directory as its home location\n')
    shell_script.write('export PATH=$PWD/python/bin:$PATH\n')
    shell_script.write('export PYTHONPATH=$PWD/packages\n')
    shell_script.write('export HOME=$PWD\n')
    shell_script.write('\n')
    shell_script.write('#change directory to executr the python script below\n')
    shell_script.write('cd ns-3.35-dcn/\n')
    shell_script.write('#before running ns3, build waf\n')
    shell_script.write('./waf configure --build-profile=optimized --disable-werror\n')
    shell_script.write('# run your script\n')
    shell_script.write('python3 run_sim_analytical_compare.py -c ' + config_file + '\n')
    shell_script.write('#cd to results directory\n')
    shell_script.write('mv runs  runs_' + config_filename + '\n')
    shell_script.write('#tar all the folders\n')
    shell_script.write('tar -czf runs_' + config_filename + '.tar.gz runs_' + config_filename + '/\n')
    shell_script.write('mv runs_' + config_filename + '.tar.gz ../\n')
    shell_script.write('#exit\n')
    shell_script.write('exit\n')


    submit_script.write('# run_ns3.sub\n')
    submit_script.write('# My very first HTCondor submit file\n')
    submit_script.write('#\n')
    submit_script.write('# Specify the HTCondor Universe (vanilla is the default and is used\n')
    submit_script.write('#  for almost all jobs) and your desired name of the HTCondor log file,\n')
    submit_script.write('#  which is where HTCondor will describe what steps it takes to run \n')
    submit_script.write('#  your job. Wherever you see $(Cluster), HTCondor will insert the \n')
    submit_script.write('#  queue number assigned to this set of jobs at the time of submission.\n')
    submit_script.write('universe = vanilla\n')
    submit_script.write('log = run_ns3_$(Cluster).log\n')
    submit_script.write('transfer_input_files = http://proxy.chtc.wisc.edu/SQUID/chtc/el8/python310.tar.gz, packages.tar.gz, ns-3.35-dcn.tar.gz\n')
    submit_script.write('#\n')
    submit_script.write('# Specify your executable (single binary or a script that runs several\n')
    submit_script.write('#  commands), arguments, and a files for HTCondor to store standard\n')
    submit_script.write('#  output (or "screen output").\n')
    submit_script.write('#  $(Process) will be a integer number for each job, starting with "0"\n')
    submit_script.write('#  and increasing for the relevant number of jobs.\n')
    submit_script.write('executable = ' + run_script_dir + '/run_' + config_filename + '.sh\n')
    submit_script.write('arguments = $(Process)\n')
    submit_script.write('output = run_ns3_$(Cluster)_$(Process).out\n')
    submit_script.write('error = run_ns3_$(Cluster)_$(Process).err\n')
    submit_script.write('#\n')
    submit_script.write('# Specify that HTCondor should transfer files to and from the\n')
    submit_script.write('#  computer where each job runs. The last of these lines *would* be\n')
    submit_script.write('#  used if there were any other files needed for the executable to use.\n')
    submit_script.write('should_transfer_files = YES\n')
    submit_script.write('when_to_transfer_output = ON_EXIT\n')
    submit_script.write('# transfer_input_files = file1,/absolute/pathto/file2,etc\n')
    submit_script.write('\n')
    submit_script.write('#notify the user when there is any change in the run\n')
    submit_script.write('notification = Always\n')
    submit_script.write('notify_user = narayana3@wisc.edu\n')
    submit_script.write('#\n')
    submit_script.write('# Tell HTCondor what amount of compute resources\n')
    submit_script.write('#  each job will need on the computer where it runs.\n')
    submit_script.write('request_cpus = 3\n')
    submit_script.write('request_memory = 50GB\n')
    submit_script.write('request_disk = 5GB\n')
    submit_script.write('#\n')
    submit_script.write('# Tell HTCondor to run 3 instances of our job:\n')
    submit_script.write('queue 1\n')

    ## Close file handles
    shell_script.close()
    submit_script.close()

    ## Write out command to master script
    master_script.write('condor_submit ' + run_script_dir + 'run_' + config_filename + '.sh\n')
