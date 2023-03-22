import os
import sys
import argparse

########################################################################################
## Parse arguments
########################################################################################
parser = argparse.ArgumentParser(description='Master script creates config files for L3 and L2custom fattrees - it is passed as an argument to run_sim_analytical_compare.py', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('-l', dest='topology',        type=str, help='fattree topology L3 or L2custom', default='L3')
parser.add_argument('-t', dest='trafficType',     type=str, help='traffic type, e.g. alltoall, incast, broadcast', default='incast')
parser.add_argument('-p', dest='netProtocol', type=str, help='network protocol, e.g. TCP, UDP', default='TCP')
args = parser.parse_args()

topology     = args.topology
trafficType  = args.trafficType
netProtocol = args.netProtocol


########################################################################################
## Set parameters for this run
########################################################################################
distributions = ['Poisson', 'GE']

#--------------------------------------------------------------------------------
# packetSizes -  ['Uniform', 'Fixed']
#     Fixed   = uses the avg_packet_size = (min_packet_size + max_packet_size)/2
#     Uniform = uses the specified min_packet_size and max_packet_size
#--------------------------------------------------------------------------------
packetSizes   = ['Uniform', 'Fixed']
min_packet_size_specified  = 495
max_packet_size_specified  = 505
avg_packet_size  = int(round((min_packet_size_specified + max_packet_size_specified)/2))

min_packet_size = min_packet_size_specified
max_packet_size = max_packet_size_specified

#----------------------------------------
# sizes for each topology
#----------------------------------------
nodesRadix = {}                # key/value pairs - only used for L2custom

if topology == 'L3' :
    sizes = [4, 8, 12, 16, 20]     # number of pods for L3
else :
    sizes = [16, 128]              # numbers of nodes for L2custom
    nodesRadix[16]=8
    nodesRadix[128]=64

print(sizes)
print(nodesRadix)

#utilizations   = [0.15, 0.30, 0.45, 0.60, 0.75, 0.825, 0.90, 0.975]   # percent utilization of datarate
#---------------------------------------------------------------------------------------
# datarates - instead of 'low', 'medium', 'high'] - switch to percent utilization of maximum linkspeed
#---------------------------------------------------------------------------------------
#datarates      = ['low', 'medium', 'high']
utilizations   = [0.25, 0.5, 0.75]   # percent utilization of max available datarate  (e.g. linkBandwidth)
unitsDataRate  = 'Mbps'              # Gbps, Mbps
linkBandwidth  = 100                 # Maximum bandwidth for the links

# Create list of data rates
# Assume maxDataRate = linkBandwidth
datarates = []
for utilization_index, utilization in enumerate(utilizations) :
    datarates.append(utilization * linkBandwidth)

print("Offered Total DataRates")
print(datarates)
print("")
print("topology= ", topology)

########################################################################################
## Main Loop
########################################################################################
for size_index, size in enumerate(sizes) :
  print('Network Size= %d' %(size))
  for distribution_index, distribution in enumerate(distributions) :
    for packetSize_index, packetSize in enumerate(packetSizes) :
      for datarate_index, datarate in enumerate(datarates) :

        ## Skip running low data rates for large configurations - size is #pods for L3
        if topology == 'L3' and (size == 16 or size == 20) :        
            # if 'low' in datarate or 'medium' in datarate :
            if datarate <= (0.5 * linkBandwidth) :
                continue
            ## if 'low' in datarate or 'medium' in datarate :
        ## if size == 16 or size == 20 :        

        ## Get number of clients
        if topology == 'L3' :
            full_size = int(size**3 / 4)
        else :
            full_size = int(size)
            switchRadix = nodesRadix[size]

        ## Decide p_burst
        if 'Poisson' in distribution :
          p_burst = 0
        else :
          p_burst = 0.1
        ## if 'Poisson' in distribution :

        ## Packet size
        # if 'Uniform' in packetSize :
          min_packet_size = min_packet_size_specified
          max_packet_size = max_packet_size_specified
        # else :
        if 'Fixed' in packetSize :
          min_packet_size = avg_packet_size
          max_packet_size = avg_packet_size
        ## if 'Fixed' in packetSize :

        ## Random seeds
        if datarate_index == len(datarates) - 1 :
          # randomSeed = '[1,2,3,4,5]'
          randomSeed = '[1,2,3]'
        else :
          randomSeed = '[1,2,3]'
        ## if datarate_index == len(datarates) - 1 :

        ## Compute data rate
        # if 'low' in datarate :
        #   dr = round(25 / (full_size - 1), 2)
        # if 'medium' in datarate :
        #   dr = round(50 / (full_size - 1), 2)
        # if 'high' in datarate :
        #   dr = round(75 / (full_size - 1), 2)
        if topology == 'L2custom' and full_size == 16 and trafficType == 'alltoall':
            dr = round(datarate / (8-1), 2)
            print("in L2custom alltoall")
        else :
            dr = round(datarate / (full_size-1), 2)

        if trafficType == 'alltoall' :
          print("per Flow DataRate=", dr)

        ## Enable first pod
        if (size > 8 and topology == 'L3') or ( size > 16 and topology == 'L2custom'):
          enable_first_pod = "true"
        else :
          enable_first_pod = "false"

        ## Reduce simulation times for large sizes
        # if size == 16 or size == 20 :        
        if (size > 15 and topology == 'L3') or ( size > 1024  and topology == 'L2custom'):
            warmup_time = 1000
            window_time = 10000
        else :
            warmup_time = 10000
            window_time = 30000
        ## if size == 16 or size == 20 :        

        ## Traffic size
        if topology == 'L3' :
            traffic_size = full_size
        elif full_size == 16 and trafficType == 'alltoall' :
            traffic_size = 8
        else : 
            traffic_size = full_size

        ## Create the trafficFile string
        if trafficType == 'incast' :
            trafficFile = str(traffic_size-1) + 'x' + str(1)
        elif trafficType == 'broadcast' :
            trafficFile = str(1) + 'x' + str(traffic_size-1)
        elif trafficType == 'alltoall' :
            trafficFile = str(traffic_size) + 'x' + str(traffic_size) + 'alltoall'
        else :
            trafficFile = str(trafficType)

        ## Create programFile
        if topology == 'L3' :
            programFile = 'scratch/dcn_fattree_finite_large_v3'
        else :
            programFile = 'scratch/dcn_fattree_finite_large_L2c_v3'
            # programFile = 'scratch/dcn_fattree_finite_large_l2custom_200G_v3'

        ## Create output file and write
        drname = str(dr).replace('.', 'p')

        ## create separate tags for L3 and L2custom so can differentiate the config files
        ## L2custom comes from the name of the programFile
        if topology == 'L3' :
            topologyTag = 'L3_'
        else :
            topologyTag = 'L2c_'

        # tag = 'size' + str(full_size) + '_alltoall_dis' + distribution + '_packet' + packetSize + '_dr' + str(drname)
        tag = str(topologyTag) + str(netProtocol) + '_size' + str(full_size) + '_' + str(trafficType) \
          + '_dis' + distribution + '_pkt' + packetSize + str(avg_packet_size) \
          + '_lnk' + str(linkBandwidth) + str(unitsDataRate) + '_dr' + str(drname)

        output_file = open('./config_' + tag + '.json', 'w')

        output_file.write('{                                                                              \n') 
        output_file.write('    "DCNStructure": {                                                          \n') 
        output_file.write('            "numPods"          : ' + str(size) + ',                            \n') 
        if topology == 'L2custom' :
            output_file.write('            "switchRadix"      : ' + str(switchRadix) + ',                 \n') 
            output_file.write('            "numNodes"         : ' + str(full_size) + ',                   \n') 
        output_file.write('            "topology"         : "' + str(topology) + '",                      \n') 
        output_file.write('            "ecmpRouting"      : "true"                                        \n') 
        output_file.write('    },                                                                         \n') 
        output_file.write('                                                                               \n') 
        output_file.write('                                                                               \n') 
        output_file.write('    "SimulationParamsInputs": {                                                \n') 
        output_file.write('        "randomSeed"         : ' + str(randomSeed) + ',                        \n') 
        output_file.write('        "simTime_inms"       : ' + str(window_time + warmup_time) + ',         \n') 
        output_file.write('        "warmup_inms"        : ' + str(warmup_time) + ',                       \n') 
        output_file.write('        "windowTime_inms"    : ' + str(window_time) + ',                       \n') 
        output_file.write('        "queueSize"          : 128,                                            \n') 
        output_file.write('        "enableLog"          : "true",                                         \n') 
        output_file.write('        "enable_first_pod"   : "' + str(enable_first_pod) + '",                \n') 
        output_file.write('        "linkBandwidth"      : "' + str(linkBandwidth) + '",                   \n') 
        output_file.write('        "unitsDataRate"      : "' + str(unitsDataRate) + '",                   \n') 
        output_file.write('        "dataRate_OnOff"     : "' + str(dr) + '",                              \n') 
        output_file.write('        "p_burst"            : "' + str(p_burst) + '",                         \n') 
        output_file.write('        "minPacketSize"      : "' + str(min_packet_size) + '",                 \n') 
        output_file.write('        "maxPacketSize"      : "' + str(max_packet_size) + '",                 \n') 
        output_file.write('        "programFile"        : "' + str(programFile) + '",                     \n') 
        output_file.write('        "trafficFile"        : "' + str(trafficFile) + '",                     \n') 
        output_file.write('        "netProtocol"        : "' + str(netProtocol) + '",                     \n') 
        output_file.write('        "tagComments"        : "' + str(tag) + '"                              \n')     
        output_file.write('    }                                                                          \n') 
        output_file.write('}                                                                              \n') 


        output_file.close()
