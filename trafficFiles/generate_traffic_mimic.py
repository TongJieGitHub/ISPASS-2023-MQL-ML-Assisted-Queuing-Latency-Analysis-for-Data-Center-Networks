import argparse
import numpy as np
import sys

"""
python generate_traffic.py ${SEED} dcn-traffic_mimic_nodes_load_.txt --load ${LOAD} --numClusters ${CLUSTERS} --numCores ${CORES} --numToRs ${DEGREE} --numServers ${SERVERS} --length ${SIMU_LEN} --linkSpeed ${LINK_SPEED}

python generate_traffic_mimic.py 1 dcn-traffic_mimic_nodes_16_load_07_rand_1.txt --load 0.7 --numClusters 4 --numCores 4 --numToRs 2 --numServers 2
"""

class custom_distribution:
    def __init__(self, rng, xp, fp):
        """takes x, y points of cdf"""
        np.all(np.diff(xp) > 0)
        self.rng = rng
        self.xp = xp
        self.fp = fp
    def sample(self, size=1):
        sampled_prob = self.rng.uniform(0, 1, size)
        # use interp func to find x given y
        sampled_x = [np.interp(prob, self.fp, self.xp) for prob in sampled_prob]
        return sampled_x

def generate_traffic_matrix(rng, load, linkSpeed, numOfServersPerRack,
                            numOfToRsPerCluster, numOfClusters, numOfCores,
                            emulatedRacks, end_time = 10.0):
    # bisection bandwidth = numSpine * linkRate
    # float from 0-1 specifying percentage of bisection bandwidth to use

    #NOTE: This could be substituted with a different distribution
    xp = [0, 10000, 20000, 30000, 50000, 80000,
          200000, 1e+06, 2e+06, 5e+06, 1e+07, 3e+07]
    fp = [0, 0.15, 0.2, 0.3, 0.4, 0.53, 0.6, 0.7, 0.8, 0.9, 0.97, 1]
    mean_flow_size = 1709062 * 8 # 1709062B from 1M samples of DCTCP
    dctcp_dist = custom_distribution(rng, xp, fp)

    numOfServers = numOfServersPerRack * numOfToRsPerCluster * numOfClusters
    start_time = 0.0

    # 100Mbps * number of spine links
    bisection_bandwidth = linkSpeed * numOfCores * numOfClusters;

    lambda_rate = bisection_bandwidth * load / mean_flow_size
    mean_interarrival_time = 1.0 / lambda_rate

    print("mean_interarrival_time =", mean_interarrival_time)
    print("estimated num of flows =", (end_time - start_time) / mean_interarrival_time)

    curr_time = start_time
    traffic_matrix = dict()
    while curr_time < end_time:
        interval = rng.exponential(mean_interarrival_time)
        curr_time += interval

        flow_size_in_bytes = int(dctcp_dist.sample(size=1)[0])

        # pick a random set of racks
        srcRack = -1
        dstRack = -1
        while srcRack == dstRack:
            srcRack = rng.choice(np.arange(0, numOfClusters*numOfToRsPerCluster))
            dstRack = rng.choice(np.arange(0, numOfClusters*numOfToRsPerCluster))

        src = rng.choice(np.arange(0, numOfServersPerRack)) + srcRack*numOfServersPerRack
        dst = rng.choice(np.arange(0, numOfServersPerRack)) + dstRack*numOfServersPerRack

        if src not in traffic_matrix:
            traffic_matrix[src] = dict()
        if dst not in traffic_matrix[src]:
            traffic_matrix[src][dst] = []
        traffic_matrix[src][dst].append([curr_time, flow_size_in_bytes])

    return traffic_matrix

if __name__=="__main__":

    load = 0.70
    numOfCores = 4
    numOfClusters = 2
    numOfToRsPerCluster = 2
    numOfServersPerRack = 2
    variant = "TCP"
    linkSpeed = 100e6
    
    parser = argparse.ArgumentParser()
    parser.add_argument("seed", type=int, help="RNG seed required.")
    parser.add_argument("outfile", type=str, help="File to write traffic to.")
    parser.add_argument("--load", type=float,
                        help="Portion of bisection bandwidth utilized.")
    parser.add_argument("--numCores", type=int,
                        help="Number of core switches (determines bisection " \
                             "bandwidth).")
    parser.add_argument("--numClusters", type=int,
                        help="Number clusters to generate traffic for.")
    parser.add_argument("--numToRs", type=int,
                        help="Number of ToR switches/racks per cluster.")
    parser.add_argument("--numServers", type=int,
                        help="Number of servers per rack.")
    parser.add_argument("--variant", type=str,
                        help="{TCP, Homa} Default is TCP.")
    parser.add_argument("--length", type=float,
                        help="The length of the trace in seconds.")
    parser.add_argument("--linkSpeed", type=float,
                        help="Link speed")
    args = parser.parse_args()

    seed = args.seed
    outfile = args.outfile
    length = 10.0
    if args.load:
        load = args.load
    if args.numCores:
        numOfCores = args.numCores
    if args.numClusters:
        numOfClusters = args.numClusters
    if args.numToRs:
        numOfToRsPerCluster = args.numToRs
    if args.numServers:
        numOfServersPerRack = args.numServers
    if args.variant:
        variant = args.variant
    if args.length:
        length = args.length
    if args.linkSpeed:
        linkSpeed = args.linkSpeed

    if variant == "TCP":
        variant_lower = "tcp"
        variant_camel = "Tcp"
        setupListeners = True
    else:
        assert(False)

    rng = np.random.RandomState(seed=seed)
    numOfServers = numOfServersPerRack * numOfToRsPerCluster * numOfClusters
    numOfServersPerCluster = numOfServersPerRack * numOfToRsPerCluster
    emulatedRacks = range(numOfToRsPerCluster, numOfClusters*numOfToRsPerCluster)
    emulatedClusters = range(1, numOfClusters)

    traffic_matrix = generate_traffic_matrix(rng, load, linkSpeed,
                                             numOfServersPerRack,
                                             numOfToRsPerCluster, numOfClusters,
                                             numOfCores, emulatedRacks,
                                             length)

    num_flows = 0
    print(outfile)
    with open(outfile, "w") as outf:
        # set number of TCP apps
        # Each one gets a sender and listener to each other server. (n-1)*2 apps per server
        numRealSenders = numOfServers - numOfServersPerRack
        numApproxSenders = numRealSenders

        # set up senders
        for local_cluster in range(numOfClusters):
            for local_rack in range(numOfToRsPerCluster):
                for local_server in range(numOfServersPerRack):
                    local_server_index = (local_cluster * numOfServersPerCluster) \
                                    + (local_rack * numOfServersPerRack) \
                                    + local_server
                    if not setupListeners:
                        local_app = 0
                    elif local_cluster in emulatedClusters:
                        local_app = numApproxSenders
                    else:
                        local_app = numRealSenders

                    for remote_cluster in range(numOfClusters):
                        for remote_rack in range(numOfToRsPerCluster):
                            if (local_cluster == remote_cluster) \
                                    and (local_rack == remote_rack):
                                continue

                            for remote_server in range(numOfServersPerRack):
                                remote_server_index = (remote_cluster * numOfServersPerCluster) \
                                    + (remote_rack * numOfServersPerRack) \
                                    + remote_server

                                outf.write("\n")
                                host_index = (local_rack * numOfServersPerRack) + local_server

                                if variant == "TCP":
                                    app_name = "**.cluster[%d].host[%d].tcpApp[%d]" \
                                                % (local_cluster, host_index, local_app)
                                local_app += 1

                                if (local_server_index in traffic_matrix.keys()) \
                                        and (remote_server_index in traffic_matrix[local_server_index].keys()):
                                    flow_array = traffic_matrix[local_server_index][remote_server_index]

                                    # if variant == "TCP":
                                    #     outf.write("%s.typename = \"TCPSessionApp\"\n" % (app_name))
                                    #     outf.write("%s.localPort = %d\n" % (app_name, 65535 - remote_server_index))
                                    #     outf.write("%s.connectPort = %d\n" % (app_name, 1000 + local_server_index))
                                    #     outf.write("%s.tOpen = %.9fs\n" % (app_name, flow_array[0][0]))
                                    #     outf.write("%s.tClose = %.9fs\n" % (app_name, flow_array[-1][0]+1))
                                    #     outf.write("%s.active = true\n" % (app_name))

                                    #     outf.write("%s.localAddress = \"1.%d.%d.%d\"\n" \
                                    #                 % (app_name, local_cluster, local_rack, local_server*4))
                                    #     outf.write("%s.connectAddress = \"1.%d.%d.%d\"\n" \
                                    #                 % (app_name, remote_cluster, remote_rack, remote_server*4))

                                    # script = ""
                                    # for d in flow_array:
                                    #     if script != "":
                                    #         script = script + "; "
                                    #     script = script + "%.9f %d" % (d[0], d[1])

                                    #     num_flows += 1
                                    #     if num_flows % 10000 == 0:
                                    #         outf.flush()
                                    # outf.write("%s.sendScript = \"%s\"\n" % (app_name, script))

                                    for d in flow_array:
                                        # outf.write("C%d:S%d:D%d:F%d:A%.9f:B%.2f\n" % (local_server_index, remote_server_index, 50, d[1], d[0], 10))
                                        outf.write("C%d:S%d:F%d:A%.9f:B%.2f\n" % (local_server_index, remote_server_index, d[1], d[0], length))
                                        num_flows += 1



    print("number of flows =", num_flows)