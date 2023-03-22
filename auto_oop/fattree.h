#ifndef FATTREE_H
#define FATTREE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <iostream>


using namespace std;

class Fattree
{
public:
    bool use_sim_host_latency = false;
	//    double linkBandwidth = 100000000; // 100 Mbps
    int bits = 8;

    int No_of_ports = 4;

    int total_no_of_core; // number of cores in the fattree
    int total_no_of_aggr; // number of aggr switch in the fattree
    int total_no_of_edge; // number of edge switch in the fattree
    int total_no_of_host; // number of nodes in the fattree

    int No_of_core_per_group;
    int No_of_aggr_per_pod;
    int No_of_edge_per_pod;
    int No_of_host_per_pod;

    int No_aggr_per_core; // the number of aggr switch each core is connected to
    int No_edge_per_aggr; // the number of edge switch each aggr is connected to
    int No_host_per_edge; // the number of nodes each edge is connected to

    int total_no_of_devices;
    int total_no_of_queues;

    int No_of_core_groups;
    int No_of_pods;

    // k / 2 core groups
    vector<vector<int>> coreGroup;
    // k pods
    vector<vector<int>> aggrPod;
    vector<vector<int>> edgePod;
    vector<vector<int>> hostPod;

    vector<Device> globalDevices;
    vector<Queue> globalQueues;

    vector<Flow> globalFlows;
    vector<FlowSplit> globalFlowSplits;

    vector<FlowV2> globalFlowsV2;
    vector<FlowSplitV2> globalFlowSplitsV2;

    struct pair_hash
    {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &p) const
        {
            auto a = std::hash<T1>{}(p.first);
            auto b = std::hash<T2>{}(p.second);
            // https://stackoverflow.com/questions/32685540/why-cant-i-compile-an-unordered-map-with-a-pair-as-key
            // https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
            return a >= b ? a * a + a + b : a + b * b;
        }
    };

    //void create(int K, double linkBandwidth, double datarate_unit);   // moved to derived class as the arugments passed are different between L3 and L2custom
    virtual void link() {};    // different implementation between L3 and L2custom
    // build all-to-all routing table
    virtual void route() {};   // different code between L3 and L2custom

    unordered_map<pair<int, int>, int, pair_hash> map_globalFlows;
    unordered_multimap<pair<int, int>, int, pair_hash> map_globalFlowSplits;
    // map (clientId, serverId) to flowId
    void mapping_globalFlows();
    // map (clientId, serverId) to flowSplitId
    void mapping_globalFlowSplits();

    void load_traffic_file(string filename);
    void update_flow_datarate(double datarate, double p_burst);
    void calc_CS_sqr_all(double link_bandwidth, double packet_size, int packet_header, int bits, double min_packet_size, double max_packet_size);

    void clear_queue_vector();
    void calc_lambda_CA_sqr_stage_0();
    void calc_stage_0();
    void calc_stage_1();
    void calc_stage_2();
    void calc_stage_3();
    void calc_stage_4();
    void calc_stage_5();
    void ME_model(bool is_zero, vector<double> v_datarate, vector<double> v_lambda, vector<double> v_CA_sqr, vector<double> v_CS_sqr, 
                  vector<double> v_packet_size, int queueSize, double linkBandwidth,
                  vector<double> &v_rho, double &CD_sqr_total, vector<double> &v_link_latency,
                  vector<double> &v_queue_latency_inf, vector<double> &v_queue_latency_finR, vector<double> &v_queue_latency_finC,
                  vector<double> &v_queue_occupancy_inf, vector<double> &v_queue_occupancy_finR, vector<double> &v_queue_occupancy_finC);
    double analytical_GG1Krestricted_GEG1Kcensored(double mnl_infinite, double rho, int queue_size, double p_burst,
                                                   double &mnl_finR, double &mnl_finC);

    void calc_flowSplit_latency();
    void calc_flow_latency();

    void printRoutingTable(string filename);
    void printQueues(string filename);
    void print_header(string filename);
    void print_latency_inf(string filename);
    void print_latency_finR(string filename);
    void print_latency_finC(string filename);

    // Splits a string by a delimiter
    std::vector<std::string> split(std::string str, std::string sep);

    // mimic traffics
    void load_traffic_file_mimic(string filename);
    void calc_lambda_CA_sqr_stage_0_mimic();
    void calc_stage_0_mimic();
    void calc_stage_1_mimic();
    void calc_stage_2_mimic();
    void calc_stage_3_mimic();
    void calc_stage_4_mimic();
    void calc_stage_5_mimic();
    void print_header_mimic(string filename);
    void print_header_all_mimic(string filename);
    void print_latency_inf_mimic(string filename, int timestamp);
    void print_latency_finR_mimic(string filename, int timestamp);
    void print_latency_finC_mimic(string filename, int timestamp);
    void print_latency_all_mimic(string filename, int timestamp);
    void reset_flow();
    void print_queue_header(string filename);
    void print_queue_info(string filename, int timestamp);

    void calc_flowSplit_latency_v2();
    void calc_flow_latency_v2();

    void regression_tree_model();
};

#endif // FATTREE_H
