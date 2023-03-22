#include <cmath>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <numeric>
#include "util.h"
#include "fattree.h"
#include "regression_tree_model.h"

#define DEBUG 0

using namespace std;

void Fattree::mapping_globalFlows()
{
    map_globalFlows.clear();
    for (auto const &current_flow : globalFlows)
    {
        map_globalFlows.insert(make_pair(make_pair(current_flow.clientId, current_flow.serverId), current_flow.flowId));
    }
}

void Fattree::mapping_globalFlowSplits()
{
    map_globalFlowSplits.clear();
    for (auto const &current_flow : globalFlowSplits)
    {
        map_globalFlowSplits.insert(make_pair(make_pair(current_flow.clientId, current_flow.serverId), current_flow.flowSplitId));
    }
}

// TODO: EXPLAIN WHAT THIS FUNCTION DOES
// IS IT CALLED ONCE DURING INITIALIZATION, OR CALLED FOR EVERY WINDOW?
void Fattree::load_traffic_file(string filename)
{
    int src;
    int dest;
    string line;
    std::vector<std::string> arr;
    ifstream myfile2(filename);
    if (myfile2.is_open())
    {
        while (getline(myfile2, line))
        {
            if (!line.empty() && line.find(":") != std::string::npos)
            {
                arr = split(line, ":");
                if ((arr[0][0] == 'C' || arr[0][0] == 'c') && (arr[1][0] == 'S' || arr[1][0] == 's'))
                {
                    src = stoi(arr[0].erase(0, 1));
                    dest = stoi(arr[1].erase(0, 1));

                    auto search = map_globalFlows.find(make_pair(src, dest));
                    if (search != map_globalFlows.end())
                    {
                        globalFlows.at(search->second).is_traffic = true;
                    }

                    auto range = map_globalFlowSplits.equal_range(make_pair(src, dest));
                    for (auto it = range.first; it != range.second; ++it)
                    {
                        int flowSplitId = it->second;
                        globalFlowSplits.at(flowSplitId).is_traffic = true;

                        int queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_0;
                        if (queueId != -1)
                        {
                            globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                            globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        }

                        queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_1;
                        if (queueId != -1)
                        {
                            globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                            globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        }

                        queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_2;
                        if (queueId != -1)
                        {
                            globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                            globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        }

                        queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_3;
                        if (queueId != -1)
                        {
                            globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                            globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        }

                        queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_4;
                        if (queueId != -1)
                        {
                            globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                            globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        }

                        queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_5;
                        if (queueId != -1)
                        {
                            globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                            globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        }
                    }
                }
                else
                    std::cout << "Wrong delimeter format in the file. Delimeter order and usage: c1:s2:d1:b0.0";
            }
        }
        myfile2.close();
    }
    else {
        // TODO: Change all cout statements to logging (else if can affect the wall-time calculations)
        std::cout << "[ERROR] Unable to open file -- " << filename << std::endl;
    }
}

void Fattree::update_flow_datarate(double datarate, double p_burst)
{
    for (auto &flow : globalFlows)
    {
        if (flow.is_traffic)
        {
            flow.datarate = datarate;
            flow.p_burst = p_burst;
        }
    }
}

// This function computes the squared service rate coefficient of variation for all queues
void Fattree::calc_CS_sqr_all(double link_bandwidth, double packet_size, int packet_header, int bits, double min_packet_size, double max_packet_size)
{
	// es::toremove
    cout << link_bandwidth << endl;
    for (auto &queue : globalQueues)
    {
        // QUESTION: IF THE INPUTS TO ALL QUEUES ARE THE SAME, WHY ARE WE CALLING THIS FUNCTION FOR ALL QUEUES?
        // SINCE THE INPUT DOES NOT CHANGE, WE CAN COMPUTE IT ONCE OUTSIDE THE LOOP AND ASSIGN THE OUTPUT TO ALL QUEUES.
        queue.calc_CS_sqr(link_bandwidth, packet_size, packet_header, bits, min_packet_size, max_packet_size);
    }
}

void Fattree::clear_queue_vector()
{
    for (auto &queue : globalQueues)
    {
        queue.clear();
    }
}

// This function computes the squared coefficient of arrivals for Stage 0 queues
// TODO: DESCRIBE WHAT WE MEAN BY STAGE 0
void Fattree::calc_lambda_CA_sqr_stage_0()
{
    for (auto &queue : globalQueues)
    {
        if (queue.type == Queue::host_up)
        {
            for (auto &flowId : queue.v_flowId)
            {
                if (globalFlows.at(flowId).is_traffic)
                {
                    double datarate = globalFlows.at(flowId).datarate;
                    double lambda = datarate * queue.datarate_unit / (queue.packet_size * queue.bits);
                    double rho = lambda / queue.mu;
                    double CA_sqr = (1 + globalFlows.at(flowId).p_burst) / (1 - globalFlows.at(flowId).p_burst);
                    queue.p_burst = globalFlows.at(flowId).p_burst;
                    queue.num_pack_bulk = 1 / (1 - globalFlows.at(flowId).p_burst);
                    queue.v_datarate.push_back(datarate);
                    queue.v_lambda.push_back(lambda);
                    queue.v_rho.push_back(rho);
                    queue.v_CA_sqr.push_back(CA_sqr);

                    for (auto &flowSplitId : globalFlows.at(flowId).vectorFlowSplitId)
                    {
                        globalFlowSplits.at(flowSplitId).datarate = datarate / globalFlows.at(flowId).vectorFlowSplitId.size();
                    }
                }
            }
        }
    }
}

void Fattree::calc_stage_0()
{
    // for (auto &queue : globalQueues)
    // {
    //     if (queue.type == Queue::host_up)
    //     {
    //         double CD_sqr_total = 0;

    //         ME_model(queue.v_lambda, queue.v_CA_sqr, queue.CS_sqr, queue.mu,
    //                  queue.queueSize, queue.num_pack_bulk, queue.p_burst, queue.channel_latency,
    //                  CD_sqr_total, queue.ME_Ana_stage_inf, queue.latency_per_queue_inf,
    //                  queue.ME_Ana_stage_finR, queue.latency_per_queue_finR, queue.ME_Ana_stage_finC, queue.latency_per_queue_finC);

    //         unordered_map<int, double> m_map;
    //         double datarate_total = 0;
    //         for (auto &flowSplitId : queue.v_flowSplitId)
    //         {
    //             int nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_1;
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_2;
    //             }
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_3;
    //             }
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_4;
    //             }
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_5;
    //             }

    //             m_map[nextQueueId] = m_map[nextQueueId] + globalFlowSplits.at(flowSplitId).datarate;
    //             datarate_total += globalFlowSplits.at(flowSplitId).datarate;
    //         }

    //         for (auto &p : m_map)
    //         {
    //             auto current_queueId = p.first;
    //             auto current_dataRate = p.second;
    //             double CD_sqr = 1 + current_dataRate / datarate_total * (CD_sqr_total - 1);

    //             globalQueues.at(current_queueId).v_CA_sqr.push_back(CD_sqr);
    //             globalQueues.at(current_queueId).v_datarate.push_back(current_dataRate);
    //             globalQueues.at(current_queueId).v_lambda.push_back(current_dataRate * queue.datarate_unit / (queue.packet_size * queue.bits));
    //         }
    //     }
    // }
}

void Fattree::calc_stage_1()
{
    // for (auto &queue : globalQueues)
    // {
    //     if (queue.type == Queue::edge_up)
    //     {
    //         double CD_sqr_total = 0;

    //         ME_model(queue.v_lambda, queue.v_CA_sqr, queue.CS_sqr, queue.mu,
    //                  queue.queueSize, queue.num_pack_bulk, queue.p_burst, queue.channel_latency,
    //                  CD_sqr_total, queue.ME_Ana_stage_inf, queue.latency_per_queue_inf,
    //                  queue.ME_Ana_stage_finR, queue.latency_per_queue_finR, queue.ME_Ana_stage_finC, queue.latency_per_queue_finC);

    //         unordered_map<int, double> m_map;
    //         double datarate_total = 0;
    //         for (auto &flowSplitId : queue.v_flowSplitId)
    //         {
    //             int nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_2;
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_3;
    //             }
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_4;
    //             }
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_5;
    //             }
    //             m_map[nextQueueId] = m_map[nextQueueId] + globalFlowSplits.at(flowSplitId).datarate;
    //             datarate_total += globalFlowSplits.at(flowSplitId).datarate;
    //         }

    //         for (auto &p : m_map)
    //         {
    //             auto current_queueId = p.first;
    //             auto current_dataRate = p.second;
    //             double CD_sqr = 1 + current_dataRate / datarate_total * (CD_sqr_total - 1);

    //             globalQueues.at(current_queueId).v_CA_sqr.push_back(CD_sqr);
    //             globalQueues.at(current_queueId).v_datarate.push_back(current_dataRate);
    //             globalQueues.at(current_queueId).v_lambda.push_back(current_dataRate * queue.datarate_unit / (queue.packet_size * queue.bits));
    //         }
    //     }
    // }
}

void Fattree::calc_stage_2()
{
    // for (auto &queue : globalQueues)
    // {
    //     if (queue.type == Queue::aggr_up)
    //     {
    //         double CD_sqr_total = 0;

    //         ME_model(queue.v_lambda, queue.v_CA_sqr, queue.CS_sqr, queue.mu,
    //                  queue.queueSize, queue.num_pack_bulk, queue.p_burst, queue.channel_latency,
    //                  CD_sqr_total, queue.ME_Ana_stage_inf, queue.latency_per_queue_inf,
    //                  queue.ME_Ana_stage_finR, queue.latency_per_queue_finR, queue.ME_Ana_stage_finC, queue.latency_per_queue_finC);

    //         unordered_map<int, double> m_map;
    //         double datarate_total = 0;
    //         for (auto &flowSplitId : queue.v_flowSplitId)
    //         {
    //             int nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_3;
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_4;
    //             }
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_5;
    //             }
    //             m_map[nextQueueId] = m_map[nextQueueId] + globalFlowSplits.at(flowSplitId).datarate;
    //             datarate_total += globalFlowSplits.at(flowSplitId).datarate;
    //         }

    //         for (auto &p : m_map)
    //         {
    //             auto current_queueId = p.first;
    //             auto current_dataRate = p.second;
    //             double CD_sqr = 1 + current_dataRate / datarate_total * (CD_sqr_total - 1);

    //             globalQueues.at(current_queueId).v_CA_sqr.push_back(CD_sqr);
    //             globalQueues.at(current_queueId).v_datarate.push_back(current_dataRate);
    //             globalQueues.at(current_queueId).v_lambda.push_back(current_dataRate * queue.datarate_unit / (queue.packet_size * queue.bits));
    //         }
    //     }
    // }
}

void Fattree::calc_stage_3()
{
    // for (auto &queue : globalQueues)
    // {
    //     if (queue.type == Queue::core_down)
    //     {
    //         double CD_sqr_total = 0;

    //         ME_model(queue.v_lambda, queue.v_CA_sqr, queue.CS_sqr, queue.mu,
    //                  queue.queueSize, queue.num_pack_bulk, queue.p_burst, queue.channel_latency,
    //                  CD_sqr_total, queue.ME_Ana_stage_inf, queue.latency_per_queue_inf,
    //                  queue.ME_Ana_stage_finR, queue.latency_per_queue_finR, queue.ME_Ana_stage_finC, queue.latency_per_queue_finC);

    //         unordered_map<int, double> m_map;
    //         double datarate_total = 0;
    //         for (auto &flowSplitId : queue.v_flowSplitId)
    //         {
    //             int nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_4;
    //             if (nextQueueId == -1)
    //             {
    //                 nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_5;
    //             }
    //             m_map[nextQueueId] = m_map[nextQueueId] + globalFlowSplits.at(flowSplitId).datarate;
    //             datarate_total += globalFlowSplits.at(flowSplitId).datarate;
    //         }

    //         for (auto &p : m_map)
    //         {
    //             auto current_queueId = p.first;
    //             auto current_dataRate = p.second;
    //             double CD_sqr = 1 + current_dataRate / datarate_total * (CD_sqr_total - 1);

    //             globalQueues.at(current_queueId).v_CA_sqr.push_back(CD_sqr);
    //             globalQueues.at(current_queueId).v_datarate.push_back(current_dataRate);
    //             globalQueues.at(current_queueId).v_lambda.push_back(current_dataRate * queue.datarate_unit / (queue.packet_size * queue.bits));
    //         }
    //     }
    // }
}

void Fattree::calc_stage_4()
{
    // for (auto &queue : globalQueues)
    // {
    //     if (queue.type == Queue::aggr_down)
    //     {
    //         double CD_sqr_total = 0;

    //         ME_model(queue.v_lambda, queue.v_CA_sqr, queue.CS_sqr, queue.mu,
    //                  queue.queueSize, queue.num_pack_bulk, queue.p_burst, queue.channel_latency,
    //                  CD_sqr_total, queue.ME_Ana_stage_inf, queue.latency_per_queue_inf,
    //                  queue.ME_Ana_stage_finR, queue.latency_per_queue_finR, queue.ME_Ana_stage_finC, queue.latency_per_queue_finC);

    //         unordered_map<int, double> m_map;
    //         double datarate_total = 0;
    //         for (auto &flowSplitId : queue.v_flowSplitId)
    //         {
    //             int nextQueueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_5;
    //             m_map[nextQueueId] = m_map[nextQueueId] + globalFlowSplits.at(flowSplitId).datarate;
    //             datarate_total += globalFlowSplits.at(flowSplitId).datarate;
    //         }

    //         for (auto &p : m_map)
    //         {
    //             auto current_queueId = p.first;
    //             auto current_dataRate = p.second;
    //             double CD_sqr = 1 + current_dataRate / datarate_total * (CD_sqr_total - 1);

    //             globalQueues.at(current_queueId).v_CA_sqr.push_back(CD_sqr);
    //             globalQueues.at(current_queueId).v_datarate.push_back(current_dataRate);
    //             globalQueues.at(current_queueId).v_lambda.push_back(current_dataRate * queue.datarate_unit / (queue.packet_size * queue.bits));
    //         }
    //     }
    // }
}

void Fattree::calc_stage_5()
{
    // for (auto &queue : globalQueues)
    // {
    //     if (queue.type == Queue::edge_down)
    //     {
    //         double CD_sqr_total = 0;

    //         ME_model(queue.v_lambda, queue.v_CA_sqr, queue.CS_sqr, queue.mu,
    //                  queue.queueSize, queue.num_pack_bulk, queue.p_burst, queue.channel_latency,
    //                  CD_sqr_total, queue.ME_Ana_stage_inf, queue.latency_per_queue_inf,
    //                  queue.ME_Ana_stage_finR, queue.latency_per_queue_finR, queue.ME_Ana_stage_finC, queue.latency_per_queue_finC);
    //     }
    // }
}

// This function TODO: EXPLAIN WHAT IT DOES
// IS IT CALLED FOR EACH QUEUE?
void Fattree::ME_model(bool is_zero, vector<double> v_datarate, vector<double> v_lambda, vector<double> v_CA_sqr, vector<double> v_CS_sqr,
                         vector<double> v_packet_size, int queueSize, double linkBandwidth,
                         vector<double> &v_rho, double &CD_sqr_total, vector<double> &v_link_latency,
                         vector<double> &v_queue_latency_inf, vector<double> &v_queue_latency_finR, vector<double> &v_queue_latency_finC,
                         vector<double> &v_queue_occupancy_inf, vector<double> &v_queue_occupancy_finR, vector<double> &v_queue_occupancy_finC)
{
    double lambda_total = 0;
    double CA_sqr_total = 0;
    double CS_sqr_total = 0;
    double rho_total = 0;

    for (int i = 0; i < v_lambda.size(); i++)
    {
        lambda_total += v_lambda.at(i);
        // double mu = linkBandwidth / (v_packet_size.at(i) * 8); // TODO: After defining bit = 8, replace 8 with it (see util.h, line 66)
        double mu = linkBandwidth; // TODO: After defining bit = 8, replace 8 with it (see util.h, line 66)
        double rho = v_lambda.at(i) / mu;
        v_rho.push_back(rho);
    }
    for (int i = 0; i < v_lambda.size(); i++)
    {
        CA_sqr_total += v_lambda.at(i) / lambda_total * v_CA_sqr.at(i);
        CS_sqr_total += v_lambda.at(i) / lambda_total * v_CS_sqr.at(i);
    }
    for (int i = 0; i < v_rho.size(); i++)
    {
        rho_total += v_rho.at(i);
    }
    if (rho_total >= 0.9999)
    {
        rho_total = 0.9999;
    }

    double Nq = 0;

    for (int r = 0; r < v_rho.size(); r++)
    {
        double max_packet_size = *max_element(v_packet_size.begin(), v_packet_size.end());

        double Lr = 0.5 * v_rho.at(r) * (v_CA_sqr.at(r) + 1);
        for (int u = 0; u < v_rho.size(); u++)
        {
            Lr += (v_lambda.at(r) / v_lambda.at(u)) * pow(v_rho.at(u), 2) * (v_CS_sqr.at(u) + v_CA_sqr.at(u)) / (2 * (1 - rho_total));
        }

        double link_latency = v_packet_size.at(r) * 8 / linkBandwidth * 1e3; // ms
        Nq += Lr - v_rho.at(r);
        // double queue_latency_inf = is_zero ? ((max_packet_size - v_packet_size.at(r)) * 8 / linkBandwidth * 1e3) : (Lr - v_rho.at(r)) / v_lambda.at(r) * 1e3;
        double queue_latency_inf = (Lr - v_rho.at(r)) / v_lambda.at(r) * 1e3 * (v_packet_size.at(r) * 8);
        if (queue_latency_inf < 0)
            queue_latency_inf = 0;
        v_link_latency.push_back(link_latency);
        v_queue_latency_inf.push_back(queue_latency_inf);

        double n_model_finR;
        double n_model_finC;
        double p_block = 0.0;

        p_block = analytical_GG1Krestricted_GEG1Kcensored(Lr, v_rho.at(r), queueSize, 0, n_model_finR, n_model_finC);
        double lambda_eff = v_lambda.at(r) * (1 - p_block);
        // double lambda_eff = v_lambda.at(r);

        // double queue_latency_finR = is_zero ? ((max_packet_size - v_packet_size.at(r)) * 8 / linkBandwidth * 1e3) : (n_model_finR - v_rho.at(r)) / lambda_eff * 1e3;
        double queue_latency_finR = (n_model_finR - v_rho.at(r)) / lambda_eff * 1e3 * (v_packet_size.at(r));
        if (queue_latency_finR < 0)
            queue_latency_finR = 0;
        // double queue_latency_finC = is_zero ? ((max_packet_size - v_packet_size.at(r)) * 8 / linkBandwidth * 1e3) : (n_model_finC - v_rho.at(r)) / lambda_eff * 1e3;
        double queue_latency_finC = (n_model_finC - v_rho.at(r)) / lambda_eff * 1e3 * (v_packet_size.at(r));
        if (queue_latency_finC < 0)
            queue_latency_finC = 0 ;
        // double queue_latency_finR = n_model_finR / lambda_eff * 1e3;
        // double queue_latency_finC = n_model_finC / lambda_eff * 1e3;
        v_queue_latency_finR.push_back(queue_latency_finR);
        v_queue_latency_finC.push_back(queue_latency_finC);
        v_queue_occupancy_inf.push_back(Lr - v_rho.at(r));
        v_queue_occupancy_finR.push_back(n_model_finR - v_rho.at(r));
        v_queue_occupancy_finC.push_back(n_model_finC - v_rho.at(r));
    }

    // double queue_latency_inf = Nq / lambda_total * 1e3;

    // for (int r = 0; r < v_rho.size(); r++) {
    //     v_queue_latency_inf.push_back(queue_latency_inf);
    // }

    CD_sqr_total = pow(rho_total, 2) * (CS_sqr_total + 1) + (1 - rho_total) * CA_sqr_total + rho_total * (1 - 2 * rho_total);
}

double Fattree::analytical_GG1Krestricted_GEG1Kcensored(double mnl_infinite, double rho, int queue_size, double p_burst,
                                                          double &mnl_finR, double &mnl_finC)
{
    int N = queue_size;

    if (mnl_infinite <= rho)
    {
        mnl_finR = 0;
        mnl_finC = 0;
        return 0.0;
    }

    // lagrange coefficients
    double x = (mnl_infinite - rho) / mnl_infinite;
    x = (x >= 0) ? x : 0;                                                                                               // eqn (2.15, Dem86)
    mnl_finR = (rho / (1 - (pow(rho, 2) * pow(x, N - 1)))) * (((1 - pow(x, N)) / (1 - x)) - ((N)*rho * pow(x, N - 1))); // eqn (3.5, Dem86)

    // yr = y restricted
    double yr = (1 - rho) / (1 - x); // eqn (3.3, Dem86)  (for x <= 1)

    // yc = y censored					        % eqn (4.19, Dem93)
    double yc = yr + p_burst * ((rho / (p_burst - x)) - ((yr + (rho / (p_burst - x))) * pow(p_burst / x, N - 1)));

    double g = (rho * (1 - x)) / (x * (1 - rho)); // eqn (2.14)    (using inifinite queue eqn with x_finite)

    vector<double> P_Nc(N + 1);
    P_Nc[0] = (1 - x) / (1 - x + (g * x * (1 - pow(x, N))) + ((1 - x) * g * yc * pow(x, N + 1))); // (4.9, 4.10 Dem93)

    P_Nc[1] = g * x * P_Nc[0]; // eqn (2.9, Dem86) & (4.9, 4.10 Dem93)

    for (int n = 2; n < N; n++)
    {
        P_Nc[n] = x * P_Nc[n - 1]; // eqn (2.10, Dem86) & (4.9, 4.10 Dem93)
    }

    P_Nc[N] = yc * x * P_Nc[N - 1]; // eqn (2.11, Dem86) & (4.9, 4.10 Dem93)

    mnl_finC = 0;
    for (int n = 1; n < N + 1; n++)
    {
        mnl_finC = mnl_finC + n * P_Nc[n]; // MQL constraint (constraint c in Dem93) (really mean Node length)
    }

    double p_block = 0.0;
    for (int n = 0; n < N; n++)
    {
        p_block = p_block + P_Nc[n] * pow(p_burst, N + 1 - n);
    }

    return p_block;
}

void Fattree::calc_flowSplit_latency()
{
    for (auto &flowSplit : globalFlowSplits)
    {
        if (flowSplit.is_traffic)
        {
            flowSplit.latency_per_flow_split_inf = 0;
            flowSplit.latency_per_flow_split_finR = 0;
            flowSplit.latency_per_flow_split_finC = 0;
            int queueId = flowSplit.stages.globalQueueId_stage_0;
            if (queueId != -1)
            {
                if (use_sim_host_latency)
                {
                    flowSplit.latency_per_flow_split_inf += flowSplit.latency_client;
                    flowSplit.latency_per_flow_split_finR += flowSplit.latency_client;
                    flowSplit.latency_per_flow_split_finC += flowSplit.latency_client;
                }
                else
                {
                    for (int i = 0; i < globalQueues.at(queueId).v_flowSplitId.size(); i++)
                    {
                        if (globalQueues.at(queueId).v_flowSplitId.at(i) == flowSplit.flowSplitId)
                        {
                            flowSplit.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                            flowSplit.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                            flowSplit.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                            break;
                        }
                    }
                }
            }
            queueId = flowSplit.stages.globalQueueId_stage_1;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitId.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitId.at(i) == flowSplit.flowSplitId)
                    {
                        flowSplit.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        break;
                    }
                }
            }
            queueId = flowSplit.stages.globalQueueId_stage_2;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitId.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitId.at(i) == flowSplit.flowSplitId)
                    {
                        flowSplit.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        break;
                    }
                }
            }
            queueId = flowSplit.stages.globalQueueId_stage_3;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitId.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitId.at(i) == flowSplit.flowSplitId)
                    {
                        flowSplit.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        break;
                    }
                }
            }
            queueId = flowSplit.stages.globalQueueId_stage_4;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitId.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitId.at(i) == flowSplit.flowSplitId)
                    {
                        flowSplit.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        break;
                    }
                }
            }
            queueId = flowSplit.stages.globalQueueId_stage_5;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitId.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitId.at(i) == flowSplit.flowSplitId)
                    {
                        flowSplit.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplit.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        break;
                    }
                }
            }
        }
    }
}

void Fattree::calc_flow_latency()
{
    for (auto &flow : globalFlows)
    {
        if (flow.is_traffic)
        {
            double latency_total_inf = 0;
            double latency_total_finR = 0;
            double latency_total_finC = 0;
            for (auto &flowSplitId : flow.vectorFlowSplitId)
            {
                latency_total_inf += globalFlowSplits.at(flowSplitId).latency_per_flow_split_inf;
                latency_total_finR += globalFlowSplits.at(flowSplitId).latency_per_flow_split_finR;
                latency_total_finC += globalFlowSplits.at(flowSplitId).latency_per_flow_split_finC;
            }
            flow.latency_per_flow_inf = latency_total_inf / flow.vectorFlowSplitId.size();
            flow.latency_per_flow_finR = latency_total_finR / flow.vectorFlowSplitId.size();
            flow.latency_per_flow_finC = latency_total_finC / flow.vectorFlowSplitId.size();
        }
    }
}

void Fattree::printRoutingTable(string filename)
{
    std::ofstream of;
    of.open(filename, std::ios::out);
    for (auto const &current_flow : globalFlowSplits)
    {
        of
            // << current_flow.flowId << ","
            // << current_flow.flowSplitId << ","
            << current_flow.clientId << ","
            << current_flow.serverId << ","
            << (current_flow.stages.globalQueueId_stage_0 != -1 ? globalQueues.at(current_flow.stages.globalQueueId_stage_0).queueName : "null") << ","
            << (current_flow.stages.globalQueueId_stage_1 != -1 ? globalQueues.at(current_flow.stages.globalQueueId_stage_1).queueName : "null") << ","
            << (current_flow.stages.globalQueueId_stage_2 != -1 ? globalQueues.at(current_flow.stages.globalQueueId_stage_2).queueName : "null") << ","
            << (current_flow.stages.globalQueueId_stage_3 != -1 ? globalQueues.at(current_flow.stages.globalQueueId_stage_3).queueName : "null") << ","
            << (current_flow.stages.globalQueueId_stage_4 != -1 ? globalQueues.at(current_flow.stages.globalQueueId_stage_4).queueName : "null") << ","
            << (current_flow.stages.globalQueueId_stage_5 != -1 ? globalQueues.at(current_flow.stages.globalQueueId_stage_5).queueName : "null") << "\n";
    }
    of.close();
}

void Fattree::printQueues(string filename)
{
    std::ofstream of;
    of.open(filename, std::ios::out);
    for (auto const &queue : globalQueues)
    {
        of << queue.globalQueueId << "," << queue.queueName << "," << queue.nextDeviceName << "," << queue.podId << "\n";
    }
    of.close();
}

void Fattree::print_header(string filename)
{
    std::ofstream of;
    of.open(filename, std::ios::out);
    of << "datarate";
    for (auto const &flow : globalFlows)
    {
        if (flow.is_traffic)
        {
            of << ",c" << flow.clientId << "_s" << flow.serverId;
        }
    }
    of << "\n";
    of.close();
}

void Fattree::print_latency_inf(string filename)
{
    std::ofstream of;
    of.open(filename, std::ios::out | std::ios::app);
    int count = 0;
    for (auto const &flow : globalFlows)
    {
        if (flow.is_traffic)
        {
            if (count == 0)
            {
                of << flow.datarate;
            }
            of << "," << flow.latency_per_flow_inf;
            count++;
        }
    }
    of << "\n";
    of.close();
}

void Fattree::print_latency_finR(string filename)
{
    std::ofstream of;
    of.open(filename, std::ios::out | std::ios::app);
    int count = 0;
    for (auto const &flow : globalFlows)
    {
        if (flow.is_traffic)
        {
            if (count == 0)
            {
                of << flow.datarate;
            }
            of << "," << flow.latency_per_flow_finR;
            count++;
        }
    }
    of << "\n";
    of.close();
}

void Fattree::print_latency_finC(string filename)
{
    std::ofstream of;
    of.open(filename, std::ios::out | std::ios::app);
    int count = 0;
    for (auto const &flow : globalFlows)
    {
        if (flow.is_traffic)
        {
            if (count == 0)
            {
                of << flow.datarate;
            }
            of << "," << flow.latency_per_flow_finC;
            count++;
        }
    }
    of << "\n";
    of.close();
}

std::vector<std::string> Fattree::split(std::string str, std::string sep)
{
    char *cstr = const_cast<char *>(str.c_str());
    char *current;
    std::vector<std::string> arr;
    current = strtok(cstr, sep.c_str());
    while (current != NULL)
    {
        arr.push_back(current);
        current = strtok(NULL, sep.c_str());
    }
    return arr;
}

void Fattree::load_traffic_file_mimic(string filename)
{
    int flowId_sim;
    int src = 0;
    int dest = 0;
    double datarate = 0;
    double packet_size = 0;
    double CA_sqr = 0;
    double CS_sqr = 0;
    double latency_client = 0;
    double CD_sqr = 0;

    istringstream iss;
    string line;
    std::vector<std::string> arr;
    ifstream myfile2(filename);
    if (myfile2.is_open())
    {
        while (getline(myfile2, line))
        {
            if (!line.empty() && line.find(":") != std::string::npos)
            {
                arr = split(line, ":");
                if ((arr[0][0] == 'I' || arr[0][0] == 'i') &&
                    (arr[1][0] == 'C' || arr[1][0] == 'c') &&
                    (arr[2][0] == 'S' || arr[2][0] == 's') &&
                    (arr[3][0] == 'R' || arr[3][0] == 'r') &&
                    (arr[4][0] == 'P' || arr[4][0] == 'p') &&
                    (arr[5][0] == 'A' || arr[5][0] == 'a') &&
                    (arr[6][0] == 'B' || arr[6][0] == 'b') &&
                    (arr[7][0] == 'L' || arr[7][0] == 'l') &&
                    (arr[8][0] == 'D' || arr[8][0] == 'd'))
                {
                    flowId_sim = stoi(arr[0].erase(0, 1));
                    src = stoi(arr[1].erase(0, 1));
                    dest = stoi(arr[2].erase(0, 1));

                    string str = arr[3].erase(0, 1);
                    iss.str(str);
                    iss >> datarate;
                    iss.clear();

                    str = arr[4].erase(0, 1);
                    iss.str(str);
                    iss >> packet_size;
                    iss.clear();

                    str = arr[5].erase(0, 1);
                    iss.str(str);
                    iss >> CA_sqr;
                    iss.clear();

                    str = arr[6].erase(0, 1);
                    iss.str(str);
                    iss >> CS_sqr;
                    iss.clear();

                    str = arr[7].erase(0, 1);
                    iss.str(str);
                    iss >> latency_client;
                    latency_client = latency_client;
                    iss.clear();

                    str = arr[8].erase(0, 1);
                    iss.str(str);
                    iss >> CD_sqr;
                    iss.clear();

                    auto search = map_globalFlows.find(make_pair(src, dest));
                    if (search != map_globalFlows.end())
                    {
                        globalFlows.at(search->second).is_traffic = true;
                        globalFlows.at(search->second).datarate = datarate;
                        globalFlows.at(search->second).packet_size = packet_size;
                        globalFlows.at(search->second).CA_sqr = CA_sqr;
                        globalFlows.at(search->second).CS_sqr = CS_sqr;
                        globalFlows.at(search->second).latency_client = latency_client;
                        globalFlows.at(search->second).CD_sqr = CD_sqr;

                        FlowV2 current_flow;
                        current_flow.clientId = globalFlows.at(search->second).clientId;
                        current_flow.serverId = globalFlows.at(search->second).serverId;
                        current_flow.flowId = globalFlows.at(search->second).flowId;
                        current_flow.vectorFlowSplitId = globalFlows.at(search->second).vectorFlowSplitId;
                        current_flow.is_traffic = true;
                        current_flow.datarate = datarate;
                        current_flow.packet_size = packet_size;
                        current_flow.CA_sqr = CA_sqr;
                        current_flow.CS_sqr = CS_sqr;
                        current_flow.latency_client = latency_client;
                        current_flow.CD_sqr = CD_sqr;

                        current_flow.flowId_sim = flowId_sim;
                        current_flow.flowIdV2 = globalFlowsV2.size();
                        globalFlowsV2.push_back(current_flow);
                    }

                    auto range = map_globalFlowSplits.equal_range(make_pair(src, dest));
                    for (auto it = range.first; it != range.second; ++it)
                    {
                        int flowSplitId = it->second;
                        globalFlowSplits.at(flowSplitId).is_traffic = true;
                        globalFlowSplits.at(flowSplitId).datarate = datarate / globalFlows.at(globalFlowSplits.at(flowSplitId).flowId).vectorFlowSplitId.size();
                        globalFlowSplits.at(flowSplitId).packet_size = packet_size;
                        globalFlowSplits.at(flowSplitId).CA_sqr = CA_sqr;
                        globalFlowSplits.at(flowSplitId).CS_sqr = CS_sqr;
                        globalFlowSplits.at(flowSplitId).latency_client = latency_client;
                        globalFlowSplits.at(flowSplitId).CD_sqr = CD_sqr;

                        FlowSplitV2 current_flowSplitV2;
                        current_flowSplitV2.clientId = globalFlowSplits.at(flowSplitId).clientId;
                        current_flowSplitV2.serverId = globalFlowSplits.at(flowSplitId).serverId;
                        current_flowSplitV2.flowId = globalFlowSplits.at(flowSplitId).flowId;
                        current_flowSplitV2.flowSplitId = globalFlowSplits.at(flowSplitId).flowSplitId;
                        current_flowSplitV2.stages = globalFlowSplits.at(flowSplitId).stages;
                        current_flowSplitV2.is_traffic = true;
                        current_flowSplitV2.datarate = datarate / globalFlows.at(globalFlowSplits.at(flowSplitId).flowId).vectorFlowSplitId.size();
                        current_flowSplitV2.packet_size = packet_size;
                        current_flowSplitV2.CA_sqr = CA_sqr;
                        current_flowSplitV2.CS_sqr = CS_sqr;
                        current_flowSplitV2.latency_client = latency_client;
                        current_flowSplitV2.CD_sqr = CD_sqr;

                        current_flowSplitV2.flowId_sim = flowId_sim;
                        current_flowSplitV2.flowSplitIdV2 = globalFlowSplitsV2.size();
                        current_flowSplitV2.flowIdV2 = globalFlowsV2.back().flowIdV2;
                        globalFlowsV2.back().vectorFlowSplitIdV2.push_back(current_flowSplitV2.flowSplitIdV2);
                        globalFlowSplitsV2.push_back(current_flowSplitV2);

                        int queueId = current_flowSplitV2.stages.globalQueueId_stage_0;
                        if (queueId != -1)
                        {
                            globalQueues.at(queueId).v_flowSplitIdV2.push_back(current_flowSplitV2.flowSplitIdV2);
                            globalQueues.at(queueId).v_flowIdV2.push_back(current_flowSplitV2.flowIdV2);
                        }

                        // int queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_0;
                        // if (queueId != -1)
                        // {
                        //     globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                        //     globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        // }

                        // queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_1;
                        // if (queueId != -1)
                        // {
                        //     globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                        //     globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        // }

                        // queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_2;
                        // if (queueId != -1)
                        // {
                        //     globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                        //     globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        // }

                        // queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_3;
                        // if (queueId != -1)
                        // {
                        //     globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                        //     globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        // }

                        // queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_4;
                        // if (queueId != -1)
                        // {
                        //     globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                        //     globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        // }

                        // queueId = globalFlowSplits.at(flowSplitId).stages.globalQueueId_stage_5;
                        // if (queueId != -1)
                        // {
                        //     globalQueues.at(queueId).v_flowSplitId.push_back(flowSplitId);
                        //     globalQueues.at(queueId).v_flowId.push_back(globalFlowSplits.at(flowSplitId).flowId);
                        // }
                    }
                }
                else
                    std::cout << "Wrong delimeter format in the file. Delimeter order and usage: c1:s2:d1:p1:a1:b1";
            }
        }
        myfile2.close();
    }
    else {
        // TODO: Change all cout statements to logging (else if can affect the wall-time calculations)
        std::cout << "[ERROR] Unable to open file -- " << filename << std::endl;
    }
}

void Fattree::calc_lambda_CA_sqr_stage_0_mimic()
{
    for (auto &queue : globalQueues)
    {
        if (queue.type == Queue::host_up)
        {
            for (auto &flowSplitIdV2 : queue.v_flowSplitIdV2)
            {
                if (globalFlowSplitsV2.at(flowSplitIdV2).is_traffic)
                {
                    double datarate = globalFlowSplitsV2.at(flowSplitIdV2).datarate;
                    // TODO: Check the following calculation (Can we keep lambda units always bits per second?)
                    // double lambda = datarate * queue.datarate_unit / (globalFlowSplitsV2.at(flowSplitIdV2).packet_size * queue.bits);
                    double lambda = datarate * queue.datarate_unit;
                    double CA_sqr = globalFlowSplitsV2.at(flowSplitIdV2).CA_sqr;
                    double CS_sqr = globalFlowSplitsV2.at(flowSplitIdV2).CS_sqr;
                    double packet_size = globalFlowSplitsV2.at(flowSplitIdV2).packet_size;
                    queue.p_burst = globalFlowSplitsV2.at(flowSplitIdV2).p_burst;
                    queue.num_pack_bulk = 1 / (1 - globalFlowSplitsV2.at(flowSplitIdV2).p_burst);
                    queue.v_datarate.push_back(datarate);
                    queue.v_lambda.push_back(lambda);
                    queue.v_CA_sqr.push_back(CA_sqr);
                    queue.v_CS_sqr.push_back(CS_sqr);
                    queue.v_packet_size.push_back(packet_size);
                }
            }
        }
    }
}

void Fattree::calc_stage_0_mimic()
{
    for (auto &queue : globalQueues)
    {
        if (queue.type == Queue::host_up)
        {
            double CD_sqr_total = 0;

            ME_model(false, queue.v_datarate, queue.v_lambda, queue.v_CA_sqr, queue.v_CS_sqr,
                     queue.v_packet_size, queue.queueSize, queue.linkBandwidth,
                     queue.v_rho, CD_sqr_total, queue.v_link_latency,
                     queue.v_queue_latency_inf, queue.v_queue_latency_finR, queue.v_queue_latency_finC,
                     queue.v_queue_occupancy_inf, queue.v_queue_occupancy_finR, queue.v_queue_occupancy_finC);

            double lambda_total = std::accumulate(queue.v_lambda.begin(), queue.v_lambda.end(), 0.0);

            for (int i = 0; i < queue.v_flowSplitIdV2.size(); i++)
            {   
                int flowSplitIdV2 = queue.v_flowSplitIdV2.at(i);
                // find the first next stage that participates in this flow
                int nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_1;
                if (nextQueueId == -1)
                {
                    nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_2;
					if (nextQueueId == -1)
					{
						nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_3;
						if (nextQueueId == -1)
						{
							nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_4;
							if (nextQueueId == -1)
							{
								nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_5;
							}
						}
					}
				}

                // use CD_sqr_total from simualtion
                CD_sqr_total = globalFlowSplitsV2.at(flowSplitIdV2).CD_sqr;
                double CD_sqr = 1 + queue.v_lambda.at(i) / lambda_total * (CD_sqr_total - 1);
                double datarate = globalFlowSplitsV2.at(flowSplitIdV2).datarate;
                // double lambda = datarate * queue.datarate_unit / (globalFlowSplitsV2.at(flowSplitIdV2).packet_size * queue.bits);
                double lambda = datarate * queue.datarate_unit;
                double CS_sqr = globalFlowSplitsV2.at(flowSplitIdV2).CS_sqr;
                double packet_size = globalFlowSplitsV2.at(flowSplitIdV2).packet_size;

                globalQueues.at(nextQueueId).v_flowIdV2.push_back(globalFlowSplitsV2.at(flowSplitIdV2).flowIdV2);
                globalQueues.at(nextQueueId).v_flowSplitIdV2.push_back(flowSplitIdV2);
                globalQueues.at(nextQueueId).v_datarate.push_back(datarate);
                globalQueues.at(nextQueueId).v_lambda.push_back(lambda);
                globalQueues.at(nextQueueId).v_CA_sqr.push_back(CD_sqr);
                globalQueues.at(nextQueueId).v_CS_sqr.push_back(CS_sqr);
                globalQueues.at(nextQueueId).v_packet_size.push_back(packet_size);
            }
        }
    }
}

void Fattree::calc_stage_1_mimic()
{
    for (auto &queue : globalQueues)
    {
        if (queue.type == Queue::edge_up)
        {
            double CD_sqr_total = 0;

            // map<int, int> count1, count2;
            bool is_zero = false;
            // for (auto &flowSplitIdV2: queue.v_flowSplitIdV2) {
            //     if (globalFlowSplitsV2.at(flowSplitIdV2).is_traffic) {
            //         count1[globalFlowSplitsV2.at(flowSplitIdV2).flowSplitId]++;
            //         count2[globalFlowSplitsV2.at(flowSplitIdV2).flowId]++;
            //     }
            // }
            // if (count1.size() == 1 || count2.size() == 1) {
            //     is_zero = true;
            // }
            // is_zero = false;

            ME_model(is_zero, queue.v_datarate, queue.v_lambda, queue.v_CA_sqr, queue.v_CS_sqr,
                     queue.v_packet_size, queue.queueSize, queue.linkBandwidth,
                     queue.v_rho, CD_sqr_total, queue.v_link_latency,
                     queue.v_queue_latency_inf, queue.v_queue_latency_finR, queue.v_queue_latency_finC,
                     queue.v_queue_occupancy_inf, queue.v_queue_occupancy_finR, queue.v_queue_occupancy_finC);

            double lambda_total = std::accumulate(queue.v_lambda.begin(), queue.v_lambda.end(), 0.0);

            for (int i = 0; i < queue.v_flowSplitIdV2.size(); i++)
            {   
                int flowSplitIdV2 = queue.v_flowSplitIdV2.at(i);

                // find the first next stage that participates in this flow
                int nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_2;
                if (nextQueueId == -1)
                {
                    nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_3;
					if (nextQueueId == -1)
					{
						nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_4;
						if (nextQueueId == -1)
						{
							nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_5;
						}
					}
				}

                double CD_sqr = 1 + queue.v_lambda.at(i) / lambda_total * (CD_sqr_total - 1);
                double datarate = globalFlowSplitsV2.at(flowSplitIdV2).datarate;
                // double lambda = datarate * queue.datarate_unit / (globalFlowSplitsV2.at(flowSplitIdV2).packet_size * queue.bits);
                double lambda = datarate * queue.datarate_unit;
                double CS_sqr = globalFlowSplitsV2.at(flowSplitIdV2).CS_sqr;
                double packet_size = globalFlowSplitsV2.at(flowSplitIdV2).packet_size;

                globalQueues.at(nextQueueId).v_flowIdV2.push_back(globalFlowSplitsV2.at(flowSplitIdV2).flowIdV2);
                globalQueues.at(nextQueueId).v_flowSplitIdV2.push_back(flowSplitIdV2);
                globalQueues.at(nextQueueId).v_datarate.push_back(datarate);
                globalQueues.at(nextQueueId).v_lambda.push_back(lambda);
                globalQueues.at(nextQueueId).v_CA_sqr.push_back(CD_sqr);
                globalQueues.at(nextQueueId).v_CS_sqr.push_back(CS_sqr);
                globalQueues.at(nextQueueId).v_packet_size.push_back(packet_size);
            }
        }
    }
}

void Fattree::calc_stage_2_mimic()
{
    for (auto &queue : globalQueues)
    {
        if (queue.type == Queue::aggr_up)
        {
            double CD_sqr_total = 0;

            // map<int, int> count1, count2;
            bool is_zero = false;
            // for (auto &flowSplitIdV2: queue.v_flowSplitIdV2) {
            //     if (globalFlowSplitsV2.at(flowSplitIdV2).is_traffic) {
            //         count1[globalFlowSplitsV2.at(flowSplitIdV2).flowSplitId]++;
            //         count2[globalFlowSplitsV2.at(flowSplitIdV2).flowId]++;
            //     }
            // }
            // if (count1.size() == 1 || count2.size() == 1) {
            //     is_zero = true;
            // }
            // is_zero = false;

            ME_model(is_zero, queue.v_datarate, queue.v_lambda, queue.v_CA_sqr, queue.v_CS_sqr,
                     queue.v_packet_size, queue.queueSize, queue.linkBandwidth,
                     queue.v_rho, CD_sqr_total, queue.v_link_latency,
                     queue.v_queue_latency_inf, queue.v_queue_latency_finR, queue.v_queue_latency_finC,
                     queue.v_queue_occupancy_inf, queue.v_queue_occupancy_finR, queue.v_queue_occupancy_finC);

            double lambda_total = std::accumulate(queue.v_lambda.begin(), queue.v_lambda.end(), 0.0);

            for (int i = 0; i < queue.v_flowSplitIdV2.size(); i++)
            {   
                int flowSplitIdV2 = queue.v_flowSplitIdV2.at(i);
				// find the first next stage that participates in this flow
                int nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_3;
                if (nextQueueId == -1)
                {
                    nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_4;
					if (nextQueueId == -1)
					{
						nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_5;
					}
				}
				
                double CD_sqr = 1 + queue.v_lambda.at(i) / lambda_total * (CD_sqr_total - 1);
                double datarate = globalFlowSplitsV2.at(flowSplitIdV2).datarate;
                // double lambda = datarate * queue.datarate_unit / (globalFlowSplitsV2.at(flowSplitIdV2).packet_size * queue.bits);
                double lambda = datarate * queue.datarate_unit;
                double CS_sqr = globalFlowSplitsV2.at(flowSplitIdV2).CS_sqr;
                double packet_size = globalFlowSplitsV2.at(flowSplitIdV2).packet_size;

                globalQueues.at(nextQueueId).v_flowIdV2.push_back(globalFlowSplitsV2.at(flowSplitIdV2).flowIdV2);
                globalQueues.at(nextQueueId).v_flowSplitIdV2.push_back(flowSplitIdV2);
                globalQueues.at(nextQueueId).v_datarate.push_back(datarate);
                globalQueues.at(nextQueueId).v_lambda.push_back(lambda);
                globalQueues.at(nextQueueId).v_CA_sqr.push_back(CD_sqr);
                globalQueues.at(nextQueueId).v_CS_sqr.push_back(CS_sqr);
                globalQueues.at(nextQueueId).v_packet_size.push_back(packet_size);
            }
        }
    }
}

void Fattree::calc_stage_3_mimic()
{
    for (auto &queue : globalQueues)
    {
        if (queue.type == Queue::core_down)
        {
            double CD_sqr_total = 0;

            // map<int, int> count1, count2;
            bool is_zero = false;
            // for (auto &flowSplitIdV2: queue.v_flowSplitIdV2) {
            //     if (globalFlowSplitsV2.at(flowSplitIdV2).is_traffic) {
            //         count1[globalFlowSplitsV2.at(flowSplitIdV2).flowSplitId]++;
            //         count2[globalFlowSplitsV2.at(flowSplitIdV2).flowId]++;
            //     }
            // }
            // if (count1.size() == 1 || count2.size() == 1) {
            //     is_zero = true;
            // }
            // is_zero = false;
			// ES: a bug - the above totally negates the previous if
			//            is_zero = false;  

            ME_model(is_zero, queue.v_datarate, queue.v_lambda, queue.v_CA_sqr, queue.v_CS_sqr,
                     queue.v_packet_size, queue.queueSize, queue.linkBandwidth,
                     queue.v_rho, CD_sqr_total, queue.v_link_latency,
                     queue.v_queue_latency_inf, queue.v_queue_latency_finR, queue.v_queue_latency_finC,
                     queue.v_queue_occupancy_inf, queue.v_queue_occupancy_finR, queue.v_queue_occupancy_finC);

            double lambda_total = std::accumulate(queue.v_lambda.begin(), queue.v_lambda.end(), 0.0);

            for (int i = 0; i < queue.v_flowSplitIdV2.size(); i++)
            {   
                int flowSplitIdV2 = queue.v_flowSplitIdV2.at(i);
                // find the first next stage that participates in this flow
                int nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_4;
                if (nextQueueId == -1)
                {
                    nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_5;
                }

                double CD_sqr = 1 + queue.v_lambda.at(i) / lambda_total * (CD_sqr_total - 1);
                double datarate = globalFlowSplitsV2.at(flowSplitIdV2).datarate;
                // double lambda = datarate * queue.datarate_unit / (globalFlowSplitsV2.at(flowSplitIdV2).packet_size * queue.bits);
                double lambda = datarate * queue.datarate_unit;
                double CS_sqr = globalFlowSplitsV2.at(flowSplitIdV2).CS_sqr;
                double packet_size = globalFlowSplitsV2.at(flowSplitIdV2).packet_size;

                globalQueues.at(nextQueueId).v_flowIdV2.push_back(globalFlowSplitsV2.at(flowSplitIdV2).flowIdV2);
                globalQueues.at(nextQueueId).v_flowSplitIdV2.push_back(flowSplitIdV2);
                globalQueues.at(nextQueueId).v_datarate.push_back(datarate);
                globalQueues.at(nextQueueId).v_lambda.push_back(lambda);
                globalQueues.at(nextQueueId).v_CA_sqr.push_back(CD_sqr);
                globalQueues.at(nextQueueId).v_CS_sqr.push_back(CS_sqr);
                globalQueues.at(nextQueueId).v_packet_size.push_back(packet_size);
            }
        }
    }
}

void Fattree::calc_stage_4_mimic()
{
    for (auto &queue : globalQueues)
    {
        if (queue.type == Queue::aggr_down)
        {
            double CD_sqr_total = 0;

            // map<int, int> count1, count2;
            bool is_zero = false;
            // for (auto &flowSplitIdV2: queue.v_flowSplitIdV2) {
            //     if (globalFlowSplitsV2.at(flowSplitIdV2).is_traffic) {
            //         count1[globalFlowSplitsV2.at(flowSplitIdV2).flowSplitId]++;
            //         count2[globalFlowSplitsV2.at(flowSplitIdV2).flowId]++;
            //     }
            // }
            // if (count1.size() == 1 || count2.size() == 1) {
            //     is_zero = true;
            // }
            // is_zero = false;

            ME_model(is_zero, queue.v_datarate, queue.v_lambda, queue.v_CA_sqr, queue.v_CS_sqr,
                     queue.v_packet_size, queue.queueSize, queue.linkBandwidth,
                     queue.v_rho, CD_sqr_total, queue.v_link_latency,
                     queue.v_queue_latency_inf, queue.v_queue_latency_finR, queue.v_queue_latency_finC,
                     queue.v_queue_occupancy_inf, queue.v_queue_occupancy_finR, queue.v_queue_occupancy_finC);

            double lambda_total = std::accumulate(queue.v_lambda.begin(), queue.v_lambda.end(), 0.0);

            for (int i = 0; i < queue.v_flowSplitIdV2.size(); i++)
            {   
                int flowSplitIdV2 = queue.v_flowSplitIdV2.at(i);
                int nextQueueId = globalFlowSplitsV2.at(flowSplitIdV2).stages.globalQueueId_stage_5;

                double CD_sqr = 1 + queue.v_lambda.at(i) / lambda_total * (CD_sqr_total - 1);
                double datarate = globalFlowSplitsV2.at(flowSplitIdV2).datarate;
                // double lambda = datarate * queue.datarate_unit / (globalFlowSplitsV2.at(flowSplitIdV2).packet_size * queue.bits);
                double lambda = datarate * queue.datarate_unit;
                double CS_sqr = globalFlowSplitsV2.at(flowSplitIdV2).CS_sqr;
                double packet_size = globalFlowSplitsV2.at(flowSplitIdV2).packet_size;

                globalQueues.at(nextQueueId).v_flowIdV2.push_back(globalFlowSplitsV2.at(flowSplitIdV2).flowIdV2);
                globalQueues.at(nextQueueId).v_flowSplitIdV2.push_back(flowSplitIdV2);
                globalQueues.at(nextQueueId).v_datarate.push_back(datarate);
                globalQueues.at(nextQueueId).v_lambda.push_back(lambda);
                globalQueues.at(nextQueueId).v_CA_sqr.push_back(CD_sqr);
                globalQueues.at(nextQueueId).v_CS_sqr.push_back(CS_sqr);
                globalQueues.at(nextQueueId).v_packet_size.push_back(packet_size);
            }
        }
    }
}

void Fattree::calc_stage_5_mimic()
{
    for (auto &queue : globalQueues)
    {
        if (queue.type == Queue::edge_down)
        {
            double CD_sqr_total = 0;

            // map<int, int> count1, count2;
            bool is_zero = false;
            // for (auto &flowSplitIdV2: queue.v_flowSplitIdV2) {
            //     if (globalFlowSplitsV2.at(flowSplitIdV2).is_traffic) {
            //         count1[globalFlowSplitsV2.at(flowSplitIdV2).flowSplitId]++;
            //         count2[globalFlowSplitsV2.at(flowSplitIdV2).flowId]++;
            //     }
            // }
            // if (count1.size() == 1 || count2.size() == 1) {
            //     is_zero = true;
            // }
            // is_zero = false;

            ME_model(is_zero, queue.v_datarate, queue.v_lambda, queue.v_CA_sqr, queue.v_CS_sqr,
                     queue.v_packet_size, queue.queueSize, queue.linkBandwidth,
                     queue.v_rho, CD_sqr_total, queue.v_link_latency,
                     queue.v_queue_latency_inf, queue.v_queue_latency_finR, queue.v_queue_latency_finC,
                     queue.v_queue_occupancy_inf, queue.v_queue_occupancy_finR, queue.v_queue_occupancy_finC);
        }
    }
}

void Fattree::print_header_mimic(string filename)
{
    std::ofstream of;
    of.open(filename, std::ios::out);
    of << "timestamp"
       << ","
       << "flowId"
       << ","
       << "client"
       << ","
       << "server"
       << ","
       << "latency"
       << std::endl;
    of.close();
}

void Fattree::print_header_all_mimic(string filename)
{
    std::ofstream of;
    of.open(filename, std::ios::out);
    of << "timestamp"
       << ","
       << "flowId"
       << ","
       << "client"
       << ","
       << "server"
       << ","
       << "latency_inf"
       << ","
       << "latency_finC"
       << ","
       << "latency_finR"
       << ","
       << "correction_inf"
       << ","
       << "latency_correction_inf"
       << std::endl;
    of.close();
}

void Fattree::print_latency_inf_mimic(string filename, int timestamp)
{
    std::ofstream of;
    of.open(filename, std::ios::out | std::ios::app);
    for (auto const &flowV2 : globalFlowsV2)
    {
        if (flowV2.is_traffic)
        {
            of << timestamp << "," << flowV2.flowId_sim << "," << flowV2.clientId << "," << flowV2.serverId << "," << flowV2.latency_per_flow_inf << std::endl;
        }
    }
    of.close();
}

void Fattree::print_latency_finR_mimic(string filename, int timestamp)
{
    std::ofstream of;
    of.open(filename, std::ios::out | std::ios::app);
    for (auto const &flowV2 : globalFlowsV2)
    {
        if (flowV2.is_traffic)
        {
            of << timestamp << "," << flowV2.flowId_sim << "," << flowV2.clientId << "," << flowV2.serverId << "," << flowV2.latency_per_flow_finR << std::endl;
        }
    }
    of.close();
}

void Fattree::print_latency_finC_mimic(string filename, int timestamp)
{
    std::ofstream of;
    of.open(filename, std::ios::out | std::ios::app);
    for (auto const &flowV2 : globalFlowsV2)
    {
        if (flowV2.is_traffic)
        {
            of << timestamp << "," << flowV2.flowId_sim << "," << flowV2.clientId << "," << flowV2.serverId << "," << flowV2.latency_per_flow_finC << std::endl;
        }
    }
    of.close();
}

void Fattree::print_latency_all_mimic(string filename, int timestamp)
{
    std::ofstream of;
    of.open(filename, std::ios::out | std::ios::app);
    for (auto const &flowV2 : globalFlowsV2)
    {
        if (flowV2.is_traffic)
        {
            of << timestamp << "," << flowV2.flowId_sim << "," << flowV2.clientId << "," << flowV2.serverId << ","
               << flowV2.latency_per_flow_inf << ","
               << flowV2.latency_per_flow_finC << ","
               << flowV2.latency_per_flow_finR << ","
               << flowV2.correction_inf << ","
               << flowV2.latency_per_flow_split_correction_inf << std::endl;
        }
    }
    of.close();
}

void Fattree::reset_flow()
{
    for (auto &flow : globalFlows)
    {
        flow.is_traffic = false;
    }

    for (auto &flow : globalFlowSplits)
    {
        flow.is_traffic = false;
    }

    globalFlowsV2.clear();
    globalFlowSplitsV2.clear();
}

void Fattree::print_queue_header(string filename)
{
    std::ofstream of;
    of.open(filename, std::ios::out);
    of << "timestamp,"
       << "flowId,"
       << "client,"
       << "server,"
       << "device,"
       << "queue,"
       << "nodeId,"
       << "queueId,"
       << "datarate_ana,"
       << "datarate_total_ana,"
       << "lambda_ana,"
       << "lambda_total_ana,"
       << "rho_ana,"
       << "rho_total_ana,"
       << "CA_sqr_ana,"
       << "CS_sqr_ana,"
       << "packet_size_ana,"
       << "queue_occupancy_inf,"
       << "queue_occupancy_inf_total,"
       << "queue_occupancy_finC,"
       << "queue_occupancy_finC_total,"
       << "queue_occupancy_finR,"
       << "queue_occupancy_finR_total,"
       << "latency_inf,"
       << "latency_finC,"
       << "latency_finR,"
       << "link_delay_ana,"
       << "correction_inf,"
       << "latency_correction_inf"
       << std::endl;
    of.close();
}

void Fattree::print_queue_info(string filename, int timestamp)
{
    std::ofstream of;
    of.open(filename, std::ios::out | std::ios::app);
    for (auto const &queue : globalQueues)
    {
        if (queue.v_datarate.size() != 0)
        {
            unordered_set<int> s_flowIdV2;
            for (auto &flowIdV2 : queue.v_flowIdV2)
            {
                s_flowIdV2.insert(flowIdV2);
            }
            for (auto &flowIdV2 : s_flowIdV2)
            {
                double lambda_total = 0;
                double datarate_total = 0;
                double rho_total = 0;
                double queue_occupancy_inf_total = 0;
                double queue_occupancy_finC_total = 0;
                double queue_occupancy_finR_total = 0;
                for (int i = 0; i < queue.v_lambda.size(); i++)
                {
                    lambda_total += queue.v_lambda.at(i);
                    datarate_total += queue.v_datarate.at(i);
                    rho_total += queue.v_rho.at(i);
                    queue_occupancy_inf_total += queue.v_queue_occupancy_inf.at(i);
                    queue_occupancy_finC_total += queue.v_queue_occupancy_finC.at(i);
                    queue_occupancy_finR_total += queue.v_queue_occupancy_finR.at(i);
                }

                double datarate = 0, lambda = 0, CA_sqr = 0, CS_sqr = 0, rho = 0, packet_size = 0;
                double link_latency = 0;
                double latency_inf = 0, latency_finC = 0, latency_finR = 0;
                double queue_occupancy_inf = 0, queue_occupancy_finC = 0, queue_occupancy_finR = 0;
                double correction_inf = 0, latency_correction_inf = 0;
                for (int i = 0; i < queue.v_flowSplitIdV2.size(); i++)
                {
                    if(queue.v_flowIdV2.at(i) == flowIdV2)
                    {
                        datarate += queue.v_datarate.at(i);
                        lambda += queue.v_lambda.at(i);
                        rho += queue.v_rho.at(i);
                        CA_sqr = queue.v_CA_sqr.at(i);
                        CS_sqr = queue.v_CS_sqr.at(i);
                        packet_size = queue.v_packet_size.at(i);
                        link_latency = queue.v_link_latency.at(i);
                        latency_inf = queue.v_queue_latency_inf.at(i) + queue.v_link_latency.at(i);
                        latency_finC = queue.v_queue_latency_finC.at(i) + queue.v_link_latency.at(i);
                        latency_finR = queue.v_queue_latency_finR.at(i) + queue.v_link_latency.at(i);
                        queue_occupancy_inf += queue.v_queue_occupancy_inf.at(i);
                        queue_occupancy_finC += queue.v_queue_occupancy_finC.at(i);
                        queue_occupancy_finR += queue.v_queue_occupancy_finR.at(i);
                        correction_inf = queue.v_queue_correction_inf.at(i);
                        latency_correction_inf = latency_inf + correction_inf;
                    }
                }

                // if (queue.type == Queue::host_up)
                // {
                //     latency_inf = globalFlowsV2.at(flowIdV2).latency_client;
                //     latency_finC = globalFlowsV2.at(flowIdV2).latency_client;
                //     latency_finR = globalFlowsV2.at(flowIdV2).latency_client;
                // }

                of << timestamp << ","
                   << globalFlowsV2.at(flowIdV2).flowId_sim << ","
                   << globalFlowsV2.at(flowIdV2).clientId << ","
                   << globalFlowsV2.at(flowIdV2).serverId << ","
                   << queue.deviceName << ","
                   << queue.queueName << ","
                   << queue.globalDeviceId << ","
                   << (queue.localQueueId + 1) << ","
                   << datarate << ","
                   << datarate_total << ","
                   << lambda << ","
                   << lambda_total << ","
                   << rho << ","
                   << rho_total << ","
                   << CA_sqr << ","
                   << CS_sqr << ","
                   << packet_size << ","
                   << queue_occupancy_inf << ","
                   << queue_occupancy_inf_total << ","
                   << queue_occupancy_finC << ","
                   << queue_occupancy_finC_total << ","
                   << queue_occupancy_finR << ","
                   << queue_occupancy_finR_total << ","
                   << latency_inf << ","
                   << latency_finC << ","
                   << latency_finR << ","
                   << link_latency << ","
                   << correction_inf << ","
                   << latency_correction_inf << std::endl;
                // of << "[";
                // for (auto e : queue.v_flowId)
                // {
                //     of << e << ",";
                // }
                // of << "],";
                // of << "[";
                // for (auto e : queue.v_flowSplitId)
                // {
                //     of << e << ",";
                // }
                // of << "],";
                // of << "[";
                // for (auto e : queue.v_datarate)
                // {
                //     of << e << ",";
                // }
                // of << "],";
                // of << "[";
                // for (auto e : queue.v_packet_size)
                // {
                //     of << e << ",";
                // }
                // of << "]";
                // of << std::endl;
            }
        }
    }
    of.close();
}


void Fattree::calc_flowSplit_latency_v2()
{
    for (auto &flowSplitV2 : globalFlowSplitsV2)
    {
        if (flowSplitV2.is_traffic)
        {
            flowSplitV2.latency_per_flow_split_inf = 0;
            flowSplitV2.latency_per_flow_split_finR = 0;
            flowSplitV2.latency_per_flow_split_finC = 0;
            flowSplitV2.correction_inf = 0;
            flowSplitV2.latency_per_flow_split_correction_inf = 0;
            int queueId = flowSplitV2.stages.globalQueueId_stage_0;
            if (queueId != -1)
            {
                flowSplitV2.latency_per_flow_split_inf += flowSplitV2.latency_client;
                flowSplitV2.latency_per_flow_split_finR += flowSplitV2.latency_client;
                flowSplitV2.latency_per_flow_split_finC += flowSplitV2.latency_client;
                flowSplitV2.latency_per_flow_split_correction_inf += flowSplitV2.latency_client;
            }
            queueId = flowSplitV2.stages.globalQueueId_stage_1;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitIdV2.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitIdV2.at(i) == flowSplitV2.flowSplitIdV2)
                    {
                        flowSplitV2.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.correction_inf += globalQueues.at(queueId).v_queue_correction_inf.at(i);
                        flowSplitV2.latency_per_flow_split_correction_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i) + globalQueues.at(queueId).v_queue_correction_inf.at(i);
                        break;
                    }
                }
            }
            queueId = flowSplitV2.stages.globalQueueId_stage_2;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitIdV2.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitIdV2.at(i) == flowSplitV2.flowSplitIdV2)
                    {
                        flowSplitV2.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.correction_inf += globalQueues.at(queueId).v_queue_correction_inf.at(i);
                        flowSplitV2.latency_per_flow_split_correction_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i) + globalQueues.at(queueId).v_queue_correction_inf.at(i);
                        break;
                    }
                }
            }
            queueId = flowSplitV2.stages.globalQueueId_stage_3;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitIdV2.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitIdV2.at(i) == flowSplitV2.flowSplitIdV2)
                    {
                        flowSplitV2.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.correction_inf += globalQueues.at(queueId).v_queue_correction_inf.at(i);
                        flowSplitV2.latency_per_flow_split_correction_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i) + globalQueues.at(queueId).v_queue_correction_inf.at(i);                        
                        break;
                    }
                }
            }
            queueId = flowSplitV2.stages.globalQueueId_stage_4;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitIdV2.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitIdV2.at(i) == flowSplitV2.flowSplitIdV2)
                    {
                        flowSplitV2.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.correction_inf += globalQueues.at(queueId).v_queue_correction_inf.at(i);
                        flowSplitV2.latency_per_flow_split_correction_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i) + globalQueues.at(queueId).v_queue_correction_inf.at(i);            
                        break;
                    }
                }
            }
            queueId = flowSplitV2.stages.globalQueueId_stage_5;
            if (queueId != -1)
            {
                for (int i = 0; i < globalQueues.at(queueId).v_flowSplitIdV2.size(); i++)
                {
                    if(globalQueues.at(queueId).v_flowSplitIdV2.at(i) == flowSplitV2.flowSplitIdV2)
                    {
                        flowSplitV2.latency_per_flow_split_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finR += globalQueues.at(queueId).v_queue_latency_finR.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.latency_per_flow_split_finC += globalQueues.at(queueId).v_queue_latency_finC.at(i) + globalQueues.at(queueId).v_link_latency.at(i);
                        flowSplitV2.correction_inf += globalQueues.at(queueId).v_queue_correction_inf.at(i);
                        flowSplitV2.latency_per_flow_split_correction_inf += globalQueues.at(queueId).v_queue_latency_inf.at(i) + globalQueues.at(queueId).v_link_latency.at(i) + globalQueues.at(queueId).v_queue_correction_inf.at(i);                       
                        break;
                    }
                }
            }
        }
    }
}


void Fattree::calc_flow_latency_v2()
{
    for (auto &flowV2 : globalFlowsV2)
    {
        if (flowV2.is_traffic)
        {
            double latency_total_inf = 0;
            double latency_total_finR = 0;
            double latency_total_finC = 0;
            double correction_inf_total = 0;
            double latency_total_correction_inf = 0;
            for (auto &flowSplitIdV2 : flowV2.vectorFlowSplitIdV2)
            {
                latency_total_inf += globalFlowSplitsV2.at(flowSplitIdV2).latency_per_flow_split_inf;
                latency_total_finR += globalFlowSplitsV2.at(flowSplitIdV2).latency_per_flow_split_finR;
                latency_total_finC += globalFlowSplitsV2.at(flowSplitIdV2).latency_per_flow_split_finC;
                correction_inf_total += globalFlowSplitsV2.at(flowSplitIdV2).correction_inf;
                latency_total_correction_inf += globalFlowSplitsV2.at(flowSplitIdV2).latency_per_flow_split_correction_inf;
            }
            flowV2.latency_per_flow_inf = latency_total_inf / flowV2.vectorFlowSplitId.size();
            flowV2.latency_per_flow_finR = latency_total_finR / flowV2.vectorFlowSplitId.size();
            flowV2.latency_per_flow_finC = latency_total_finC / flowV2.vectorFlowSplitId.size();
            flowV2.correction_inf = correction_inf_total / flowV2.vectorFlowSplitId.size();
            flowV2.latency_per_flow_split_correction_inf = latency_total_correction_inf / flowV2.vectorFlowSplitId.size();
        }
    }
}

void Fattree::regression_tree_model()
{
    for (auto &queue : globalQueues)
    {
        if (queue.v_datarate.size() != 0)
        {
            for (auto &flowIdV2 : queue.v_flowIdV2)
            {
                double lambda_total = 0;
                double datarate_total = 0;
                double rho_total = 0;

                for (int i = 0; i < queue.v_lambda.size(); i++)
                {
                    lambda_total += queue.v_lambda.at(i);
                    datarate_total += queue.v_datarate.at(i);
                    rho_total += queue.v_rho.at(i);
                }

                double datarate = 0, lambda = 0, CA_sqr = 0, CS_sqr = 0, rho = 0, packet_size = 0;
                double link_latency = 0;
                double latency_inf = 0, latency_finC = 0, latency_finR = 0;
                for (int i = 0; i < queue.v_flowSplitIdV2.size(); i++)
                {
                    if(queue.v_flowIdV2.at(i) == flowIdV2)
                    {
                        datarate += queue.v_datarate.at(i);
                        lambda += queue.v_lambda.at(i);
                        rho += queue.v_rho.at(i);
                        CA_sqr = queue.v_CA_sqr.at(i);
                        CS_sqr = queue.v_CS_sqr.at(i);
                        packet_size = queue.v_packet_size.at(i);
                        link_latency = queue.v_link_latency.at(i);
                        latency_inf = queue.v_queue_latency_inf.at(i) + queue.v_link_latency.at(i);
                        latency_finC = queue.v_queue_latency_finC.at(i) + queue.v_link_latency.at(i);
                        latency_finR = queue.v_queue_latency_finR.at(i) + queue.v_link_latency.at(i);
                    }
                }

                double feature_1_rho_ana_i = 1 / (1 - rho);
                double feature_1_rhoT_ana_i = 1 / (1 - rho_total);
                double feature_dr_1_rho_ana_i = datarate / (1 - rho);
                double feature_dr_1_rhoT_ana_i = datarate / (1 - rho_total);
                double feature_CA_sqr_CS_sqr_1_rho_i = (CA_sqr + CS_sqr) / (1 - rho_total);

                double feature_array[] = {datarate, rho, rho_total, CA_sqr, CS_sqr, packet_size,
                                            feature_1_rho_ana_i, feature_1_rhoT_ana_i, feature_dr_1_rho_ana_i, feature_dr_1_rhoT_ana_i, feature_CA_sqr_CS_sqr_1_rho_i};
                double correction = 0.0;

                if (packet_size < 200)
                {
                    if (queue.type == Queue::edge_up)
                    {
                        correction = edge_up_small(feature_array);
                    }
                    else if (queue.type == Queue::aggr_up)
                    {
                        correction = aggr_up_small(feature_array);
                    }
                    else if (queue.type == Queue::core_down)
                    {
                        correction = core_down_small(feature_array);
                    }
                    else if (queue.type == Queue::aggr_down)
                    {
                        correction = aggr_down_small(feature_array);
                    }
                    else if (queue.type == Queue::edge_down)
                    {
                        correction = edge_down_small(feature_array);
                    }
                }
                else
                {
                    if (queue.type == Queue::edge_up)
                    {
                        correction = edge_up_large(feature_array);
                    }
                    else if (queue.type == Queue::aggr_up)
                    {
                        correction = aggr_up_large(feature_array);
                    }
                    else if (queue.type == Queue::core_down)
                    {
                        correction = core_down_large(feature_array);
                    }
                    else if (queue.type == Queue::aggr_down)
                    {
                        correction = aggr_down_large(feature_array);
                    }
                    else if (queue.type == Queue::edge_down)
                    {
                        correction = edge_down_large(feature_array);
                    }
                }

                queue.v_queue_correction_inf.push_back(correction);

            }
        }
    } 
}
