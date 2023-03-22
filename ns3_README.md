
The Network Simulator, Version 3
================================

## Table of Contents:

1) [Building ns-3](#building-ns-3-and-analytical-model)
2) [Directory structure](#directory-structure)
3) [Running ns-3](#running-ns-3)

Note:  Much more substantial information about ns-3 can be found at
https://www.nsnam.org

## Building ns-3 and analytical model 
(ONCE per repository unless required otherwise)

NS3 provides a set of libraries that users can deploy with their OWN programs.
Building NS3 only builds the internal libraries, and should be performed ONLY ONCE per repository (unless we change the libraries/models - not expected typically!)

```shell
./waf configure --build-profile=optimized --disable-werror
./waf
```

```shell
cd ./auto_oop
make clean && make
cd ../
```

## Directory structure
```shell
.
├── auto_oop                                   ## Directory for analytical model
│   ├── main.cpp                               ## Main program for analytical model 
│   ├── util.cpp                               ## Helper functions for analytical model
├── build                                      ## Build directory that is created automatically when waf is executed
├── config_file.json                           ## The configuration file that contains the parameters and inputs for simulation and analytical models
├── doc                                        ## Documentation
├── header.txt                                 ## 
├── LICENSE                                    ## 
├── Makefile                                   ## 
├── ns3_README.md                              ## The master README to be used by DCN developers/users
├── runs                                       ## After a simulation/analytical model run, the runs directory will hold the reports/plots/data for each run
│   └── <tag>                                  ## tag is crucial to identify each run. It is named after some parameters in config_file.json
│       ├── config_file.json                   ## The configuration file that is used for this particular run
│       ├── outputs_sim                        ## Outputs of simulation, which are used by analytical model (contains flow details for each timestamp)
│           └── mimic*.csv                     ## Output files per timestamp, which are used by the analytical model
│       ├── reports_ana                        ## Reports after running the analytical model
│           └── latency_per_flow.csv           ## Intermediate report from analytical model with latency per flow
│           └── latency_per_flow_merged.csv    ## Final merged report of simulation and analytical latencies per flow
│           └── latency_per_queue.csv          ## Intermediate report from analytical model with latency per queue
│           └── latency_per_queue_merged.csv   ## Final merged report of simulation and analytical latencies per queue
│       └── reports_sim                        ## Reports after running the simulations
│           └── end_to_end_latency.csv         ## End-to-end latency reports
│           └── flow_queue_log.csv             ## Per queue reports
│           └── timing.csv                     ## Time taken for simulation
├── run_sim_analytical_compare.py              ## MASTER script that runs simulation, analytical models, performs comparison and generates plots
├── scratch                                    ## Directory that contains the user programs (TCP/UDP, etc.)
├── src                                        ## Source code directory of ns-3 libraries
├── trafficFiles                               ## Directory that contains the input traffic patterns to the DCN
├── utils                                      ## 
├── utils.py                                   ## 
├── VERSION                                    ## 
├── waf                                        ## Script that builds the ns-3 libraries
├── waf-tools                                  ## Build scripts
```

## Running ns-3

The DCN structure, topology and other inputs/parameters are specified in `config_file.json`.
After modifying/confirming the parameters in the configuration file, ns-3 can be run by:

```shell
python3 run_sim_analytical_compare.py -c <configuration_file.json> [ Default is: './config_file.json' ]

Examples:
python3 run_sim_analytical_compare.py -c ./config_file.json
python3 run_sim_analytical_compare.py -c ./config_file_window100ms_alltoalltraffic.json
```

This script automatically reads the `config_file.json`, runs simulation, analytical models, compares the two and generates plots using the above directory structure.


