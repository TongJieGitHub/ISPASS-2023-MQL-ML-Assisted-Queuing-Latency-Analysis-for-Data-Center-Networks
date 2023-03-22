#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>

using namespace std;

class Device
{
public:
    int globalDeviceId;
    string deviceName;

    int podId;
    int localDeviceId;

    int numQueues;
    vector<int> vectorQueueId;

    enum device_type
    {
        core,
        aggr,
        edge,
        host
    };
    device_type type;
};

class Queue
{
public:
    int globalDeviceId;
    int globalQueueId;
    string deviceName;
    string queueName;

    int localQueueId;
    int podId;
    int localDeviceId;

    int nextdeviceId = -1;
    string nextDeviceName;

    enum queue_type
    {
        core_down,
        aggr_up,
        aggr_down,
        edge_up,
        edge_down,
        host_up
    };
    queue_type type;

    // unordered_set<int> set_flowId;
    // unordered_set<int> set_flowSplitId;

    int queueSize = 128;
    double linkBandwidth;     //  passed in commandline
    double datarate_unit;     // passed in commandline

    double packet_size = 500;           // bytes  --> overwritten by trace
    int packet_header = 30;             // bytes  --> overwritten by??
    int bits = 8;                       // Byte --> bits 
                                        // TODO: Give a more meaningful name and use #define
    double min_packet_size = 500;
    double max_packet_size = 500;

    double mu = 0;
    double t_serv = 0;
    double CS_sqr = 0;
	//    double channel_latency = ((packet_size + packet_header) * bits / linkBandwidth) * 1000;
    double channel_latency;

    double p_burst = 0;
    double num_pack_bulk = 1;

    vector<int> v_flowId;
    vector<int> v_flowSplitId;
    vector<int> v_flowIdV2;
    vector<int> v_flowSplitIdV2;

    vector<double> v_datarate;
    vector<double> v_lambda;
    vector<double> v_CA_sqr;
    vector<double> v_rho;
    vector<double> v_CS_sqr;
    vector<double> v_packet_size;

    vector<double> v_queue_latency_inf;
    vector<double> v_queue_latency_finR;
    vector<double> v_queue_latency_finC;
    vector<double> v_queue_occupancy_inf;
    vector<double> v_queue_occupancy_finR;
    vector<double> v_queue_occupancy_finC;

    vector<double> v_link_latency;

    vector<double> v_queue_correction_inf;


    // double ME_Ana_stage_inf = 0;
    // double ME_Ana_stage_finR = 0;
    // double ME_Ana_stage_finC = 0;
    // double latency_per_queue_inf = 0;
    // double latency_per_queue_finR = 0;
    // double latency_per_queue_finC = 0;

    void clear();
    void calc_CS_sqr(double link_bandwidth, double packet_size, int packet_header, int bits, double min_packet_size, double max_packet_size);
};

class Stages
{
public:
    int globalQueueId_stage_0 = -1;
    int globalQueueId_stage_1 = -1;
    int globalQueueId_stage_2 = -1;
    int globalQueueId_stage_3 = -1;
    int globalQueueId_stage_4 = -1;
    int globalQueueId_stage_5 = -1;
};

class Flow
{
public:
    int clientId;
    int serverId;
    int flowId;
    vector<int> vectorFlowSplitId;
    bool is_traffic = false;

    double datarate = 0;
    double p_burst = 0;
    double packet_size = 0;
    double CA_sqr = 0;
    double CS_sqr = 0;
    double CD_sqr = 0;

    double latency_per_flow_inf = 0;
    double latency_per_flow_finR = 0;
    double latency_per_flow_finC = 0;

    double latency_client = 0;

    double correction_inf = 0;
    double latency_per_flow_split_correction_inf = 0;
};

// a flow split by ECMP routing
class FlowSplit
{
public:
    int clientId;
    int serverId;
    int flowId;
    int flowSplitId;
    Stages stages;
    bool is_traffic = false;

    double datarate = 0;
    double p_burst = 0;
    double packet_size = 0;
    double CA_sqr = 0;
    double CS_sqr = 0;
    double CD_sqr = 0;

    double latency_per_flow_split_inf = 0;
    double latency_per_flow_split_finR = 0;
    double latency_per_flow_split_finC = 0;

    double latency_client = 0;

    double correction_inf = 0;
    double latency_per_flow_split_correction_inf = 0;
};


class FlowV2: public Flow
{
public:
    int flowId_sim;
    int flowIdV2;
    vector<int> vectorFlowSplitIdV2;
};

class FlowSplitV2: public FlowSplit
{
public:
    int flowId_sim;
    int flowIdV2;
    int flowSplitIdV2;
};

