import run_sim_analytical_compare as sim_analytical
import multiprocessing
import platform

## Get current platform type
current_platform = platform.system()

## Specify injection rates to execute
## A scale value of 10 implies that a new application/job/DAG is injected every 10 microseconds
randomSeed_list = [1,2,3,4,5,6,7,8,9,10]
randomSeed_list = [1]

## Run with multiprocessing library if platform is Linux
if current_platform.lower() == "linux" :
    p = multiprocessing.Pool(10)
    p.map(sim_analytical.run_simulator, randomSeed_list)
