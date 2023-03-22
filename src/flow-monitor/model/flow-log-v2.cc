#include <iostream>
#include <string>
#include <cassert>
#include <chrono>
#include <ctime>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <numeric>

#include "flow-log-v2.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include <fstream>
#include <sstream>
#include "ns3/node.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/ipv4-flow-probe.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-socket-state.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/ipv4-interface.h"
#include "ns3/queue-disc.h"

#define PERIODIC_SAMPLING_INTERVAL (MicroSeconds (2000))
// #define PERIODIC_WINDOW_INTERVAL (MicroSeconds   (9999998))
// #define PERIODIC_ANA_INTERVAL (MicroSeconds      (9999999))
// #define PERIODIC_PRINT_INTERVAL (MicroSeconds    (10000000))

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FlowLogV2");

NS_OBJECT_ENSURE_REGISTERED (FlowLogV2);

TypeId 
FlowLogV2::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FlowLogV2")
    .SetParent<Object> ()
    .SetGroupName ("FlowLogV2")
    .AddConstructor<FlowLogV2> ()
    .AddAttribute ("TimeWindow", 
                   "TimeWindow",
                   TimeValue (MicroSeconds(100000)),
                   MakeTimeAccessor (&FlowLogV2::m_TimeWindow),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId 
FlowLogV2::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

FlowLogV2::FlowLogV2() 
{
  Simulator::Schedule (PERIODIC_SAMPLING_INTERVAL, &FlowLogV2::PeriodicSampling, this);
  // Simulator::Schedule (m_TimeWindow - MicroSeconds(2), &FlowLogV2::PeriodicUpdateWindow, this);
  // Simulator::Schedule (m_TimeWindow - MicroSeconds(1), &FlowLogV2::PeriodicDoAnalyticalModel, this);
  // Simulator::Schedule (m_TimeWindow, &FlowLogV2::PeriodicPrintLog, this);
}

FlowLogV2::~FlowLogV2() 
{
    
}

void
FlowLogV2::Schedule() 
{
  Simulator::Schedule (m_TimeWindow + m_TimeWarmUp - MicroSeconds(2), &FlowLogV2::PeriodicUpdateWindow, this);
  Simulator::Schedule (m_TimeWindow + m_TimeWarmUp - MicroSeconds(1), &FlowLogV2::PeriodicDoAnalyticalModel, this);
  Simulator::Schedule (m_TimeWindow + m_TimeWarmUp, &FlowLogV2::PeriodicPrintLog, this);
}

void
FlowLogV2::CreateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t stage, uint32_t flowId, Address srcAddress, Address desAddress, std::unordered_map<uint32_t, int> address_client_map, std::unordered_map<uint32_t, int> address_server_map)
{
  // Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  // int client = address_client_map[sourceAddress.Get()];
  // Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  // int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(std::make_pair(flowId, (std::make_pair(probe->GetNode()->GetId(), interface))));
  if(search == m_logs.end()) {
    Ptr<FlowLogTracker> tracker = ns3::Create<FlowLogTracker>();
    tracker->flowId = flowId;
    tracker->stage = stage;
    tracker->nodeId = probe->GetNode()->GetId();
    tracker->probe = probe;
    tracker->interface = interface;
    tracker->srcAddress = srcAddress;
    tracker->desAddress = desAddress;
    tracker->isLastStage = false;
    m_logs.insert(std::make_pair(std::make_pair(flowId, (std::make_pair(probe->GetNode()->GetId(), interface))), tracker));
    this->address_client_map = address_client_map;
    this->address_server_map = address_server_map;
  }
}

void
FlowLogV2::UpdateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency, bool isLastStage)
{
  // Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  // int client = address_client_map[sourceAddress.Get()];
  // Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  // int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(std::make_pair(flowId, (std::make_pair(probe->GetNode()->GetId(), interface))));
  if(search != m_logs.end()) {
    auto tracker = search->second;
    if (isLastStage) {
      tracker->isLastStage = true;
    }
    Time now = Simulator::Now ();
    tracker->sumPacketSizeArrival += (packetSize + 2);
    tracker->sumPacketSizeArrivalSqr += (packetSize + 2) * (packetSize + 2);
    tracker->countArrival += 1;
    if (tracker->countArrival > 1) {
      tracker->sumInterarrival += now.GetNanoSeconds() - tracker->lastTimeArrival;
      tracker->sumInterarrivalSqr += (now.GetNanoSeconds() - tracker->lastTimeArrival) * (now.GetNanoSeconds() - tracker->lastTimeArrival);
    }
    tracker->lastTimeArrival = now.GetNanoSeconds();
  }
}

void 
FlowLogV2::UpdateLatency (Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency)
{
  // Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  // int client = address_client_map[sourceAddress.Get()];
  // Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  // int server = address_server_map[destinationAddress.Get()];
  Time now = Simulator::Now ();
  auto search = m_logs.find(std::make_pair(flowId, (std::make_pair(probe->GetNode()->GetId(), interface))));
  if(search != m_logs.end()) {
    auto tracker = search->second;
    tracker->sumProductLatencySize += latency * (packetSize + 2);
    tracker->countDeparture += 1;
    tracker->sumPacketSizeDeparture += (packetSize + 2);
    if (tracker->countDeparture > 1) {
      tracker->sumInterDeparture += now.GetNanoSeconds() - tracker->lastTimeDeparture;
      tracker->sumInterDepartureSqr += (now.GetNanoSeconds() - tracker->lastTimeDeparture) * (now.GetNanoSeconds() - tracker->lastTimeDeparture);
    }
    tracker->lastTimeDeparture = now.GetNanoSeconds();
    tracker->sum_latency += static_cast<double>(latency) / 1e6; // ns->ms
    tracker->count_latency ++;
    tracker->sum_rxBytes += (packetSize + 2);
    tracker->sum_rxPackets ++;
  }
}

int FlowLogV2::GetCount(Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId)
{
  // auto search = m_logs.find(std::make_pair(flowId, (std::make_pair(probe->GetNode()->GetId(), interface))));
  // if(search != m_logs.end()) {
  //   auto tracker = search->second;
  //   return tracker->count;
  // } else {
  //   return -1;
  // }
}

void
FlowLogV2::Sampling()
{
  // for(auto log : m_logs) {
  //   auto tracker = log.second;
  //   Time now = Simulator::Now ();

  //   Ptr<Ipv4FlowProbe> probe_ipv4 = tracker->probe->GetObject<Ipv4FlowProbe>();
  //   Ptr<Ipv4Interface> ipv4interface =  probe_ipv4->m_ipv4->GetInterface(tracker->interface);
  //   Ptr<QueueDisc> qdisc = ipv4interface->m_tc->GetRootQueueDiscOnDevice(ipv4interface->m_device);
  //   uint32_t packets = qdisc->GetNPackets();
  //   uint32_t bytes = qdisc->GetNBytes();
  //   tracker->vectorQueuePackets.push_back(packets);
  //   tracker->vectorQueueBytes.push_back(bytes);
  // }
}

void
FlowLogV2::UpdateWindow()
{
  for(auto log : m_logs) {
    auto tracker = log.second;
    Time now = Simulator::Now ();

    // compute arrival datarate
    double meanDatarateArrival = tracker->sumPacketSizeArrival * 8 * 1000 / (m_TimeWindow.GetNanoSeconds());
    tracker->meanDatarateArrival = meanDatarateArrival;

    // compute departure datarate
    double meanDatarateDeparture = tracker->sumPacketSizeDeparture * 8 * 1000 / (m_TimeWindow.GetNanoSeconds());
    tracker->meanDatarateDeparture = meanDatarateDeparture;

    // compute coefficient of variation of interarrival time 
    int64_t n = tracker->countArrival - 1;
    double mean = tracker->sumInterarrival / n;
    double var = (tracker->sumInterarrivalSqr - tracker->sumInterarrival * tracker->sumInterarrival / n) / (n - 1);
    double CA_sqr = var / (mean * mean);
    tracker->meanInterarrival = mean / 1e6;  // ns -> ms
    tracker->CA_sqr = CA_sqr;

    // compute coefficient of variation of interdeparture times
    n = tracker->countDeparture - 1;
    mean = tracker->sumInterDeparture / n;
    var = (tracker->sumInterDepartureSqr - tracker->sumInterDeparture * tracker->sumInterDeparture / n) / (n - 1);
    double CD_sqr = var / (mean * mean);
    tracker->CD_sqr = CD_sqr;

    // compute coefficient of variation of service time
    n = tracker->countArrival;
    mean = tracker->sumPacketSizeArrival / n;
    var = (tracker->sumPacketSizeArrivalSqr - tracker->sumPacketSizeArrival * tracker->sumPacketSizeArrival / n) / (n - 1);
    double CS_sqr= var / (mean * mean);
    // tracker->meanServiceTime = mean / 1e6;  // ns -> ms
    tracker->CS_sqr = CS_sqr;

    tracker->txPackets = tracker->countArrival;
    tracker->txBytes = tracker->sumPacketSizeArrival;
    
    double latency_total = tracker->sum_latency;
    tracker->meanLatencySim = tracker->sum_latency / tracker->count_latency;
    tracker->rxPackets = tracker->sum_rxPackets;
    tracker->rxBytes = tracker->sum_rxBytes;
    tracker->meanPacketSize = static_cast<double>(tracker->sum_rxBytes) / tracker->sum_rxPackets;
    tracker->meanServiceTime = tracker->meanPacketSize * 8 *10 / 1e6;


    double sumQueuePackets = latency_total / m_TimeWindow.GetMilliSeconds();
    tracker->meanQueuePackets = sumQueuePackets;
    tracker->meanQueueBytes = tracker->sumProductLatencySize / m_TimeWindow.GetNanoSeconds();

    tracker->lastTimeArrival = 0;
    tracker->sumInterarrival = 0;
    tracker->sumInterarrivalSqr = 0;
    tracker->countArrival = 0;
    tracker->sumPacketSizeArrival = 0;
    tracker->sumPacketSizeArrivalSqr = 0;

    tracker->lastTimeDeparture = 0;
    tracker->sumInterDeparture = 0;
    tracker->sumInterDepartureSqr = 0;
    tracker->countDeparture = 0;
    tracker->sumPacketSizeDeparture = 0;
    tracker->sumPacketSizeDepartureSqr = 0;
    tracker->sumProductLatencySize = 0;

    tracker->sum_latency = 0;
    tracker->count_latency = 0;
    tracker->sum_rxBytes = 0;
    tracker->sum_rxPackets = 0;
  }
}

void 
FlowLogV2::DoAnalyticalModel ()
{
  for(auto log : m_logs) {
    auto tracker = log.second;

    // skip the last stage
    if(tracker->isLastStage) {
      continue;
    }
    
    Time now = Simulator::Now ();
    // analytical models
    std::vector<double>  v_lambda_arrival, v_lambda_departure, v_rho, v_CA_sqr, v_CS_sqr;
    std::map<uint32_t, int> m_flowId;
    // uint32_t current_flowId = tracker->flowId;
    // uint32_t current_stage = tracker->stage;
    double current_lambda = tracker->meanDatarateArrival;
    double current_rho = tracker->meanDatarateArrival / 100;
    // double current_CA_sqr = tracker->CA_sqr;
    // double current_minInterarrival = tracker->minInterarrival;
    // double current_CS_sqr = tracker->CS_sqr;
    uint32_t current_nodeId = tracker->nodeId;
    uint32_t current_interface = tracker->interface;
    double total_rho = 0;
    double total_datarate_arrival = 0;
    double total_datarate_departure = 0;
    double mnl_inf = 0;
    //double mql_inf = 0;
    double ME_lat_ana_inf_GEG1 = 0;
    double ME_lat_ana_inf_GED1 = 0;
    
    // for(auto m: m_logs) {
    //   // skip the last stage
    //   if(m.second->isLastStage) {
    //   continue;
    //   }

    //   if (m.second->nodeId == current_nodeId && m.second->interface == current_interface) {
    //     v_lambda_arrival.push_back(m.second->meanDatarateArrival);
    //     v_lambda_departure.push_back(m.second->meanDatarateDeparture);
    //     v_rho.push_back(m.second->meanDatarateArrival / 100); // TODO: 
    //     // v_CA_sqr.push_back(m.second->CA_sqr);
    //     // v_CS_sqr.push_back(m.second->CS_sqr);
    //     // m_flowId[m.second->flowId]++;
    //     //std::cout <<  "(" << m.first.first << "," << m.first.second << "), "; 
    //   }
    // }
    //std::cout << std::endl;
    //std::cout << current_nodeId << ", " << current_interface << ": " << v_lambda.size() << " " << v_rho.size() << " " << v_CA_sqr.size() << " " << std::endl;

    total_rho = std::accumulate(v_rho.begin(), v_rho.end(), 0.0);
    total_datarate_arrival = std::accumulate(v_lambda_arrival.begin(), v_lambda_arrival.end(), 0.0);
    total_datarate_departure = std::accumulate(v_lambda_departure.begin(), v_lambda_departure.end(), 0.0);
    // mnl_inf = 0.5 * current_rho * (1 + 1);
    // for (size_t i = 0; i < v_rho.size(); i++) {
    //   mnl_inf += (current_lambda / v_lambda.at(i)) * v_rho.at(i) * v_rho.at(i) * (v_CA_sqr.at(i) + v_CS_sqr.at(i)) / (1 - total_rho) * 0.5;
    // }
    // ME_lat_ana_inf_GEG1 = (8 * tracker->meanPacketSize * 1000) * mnl_inf / current_lambda;
    // tracker->ME_lat_ana_inf_GEG1 = ME_lat_ana_inf_GEG1;

    // mnl_inf = 0.5 * current_rho * (1 + 1);
    // for (size_t i = 0; i < v_rho.size(); i++) {
    //   mnl_inf += (current_lambda / v_lambda.at(i)) * v_rho.at(i) * v_rho.at(i) * (v_CA_sqr.at(i) + 0) / (1 - total_rho) * 0.5;
    // }
    // ME_lat_ana_inf_GED1 = (8 * tracker->meanPacketSize * 1000) * mnl_inf / current_lambda;
    // tracker->ME_lat_ana_inf_GED1 = ME_lat_ana_inf_GED1;

    tracker->meanDatarateArrival_total = total_datarate_arrival;
    tracker->meanDatarateDeparture_total = total_datarate_departure;
    tracker->rho = current_rho;
    tracker->rho_total = total_rho;
    tracker->num_flows = v_rho.size();
    
    // tracker->vectorErrorDiff.clear();
    // tracker->vectorErrorDiffAbs.clear();
    // tracker->vectorErrorPct.clear();
    // tracker->vectorErrorPctAbs.clear();
    // tracker->vectorErrorRatio.clear();

    // tracker->vectorErrorDiff.push_back(tracker->ME_lat_ana_inf_GEG1 - tracker->meanLatencySim);
    // tracker->vectorErrorDiffAbs.push_back(abs(tracker->ME_lat_ana_inf_GEG1 - tracker->meanLatencySim));
    // tracker->vectorErrorPct.push_back((tracker->ME_lat_ana_inf_GEG1 - tracker->meanLatencySim) / tracker->meanLatencySim * 100);
    // tracker->vectorErrorPctAbs.push_back(abs((tracker->ME_lat_ana_inf_GEG1 - tracker->meanLatencySim)) / tracker->meanLatencySim * 100);
    // tracker->vectorErrorRatio.push_back((tracker->ME_lat_ana_inf_GEG1 - tracker->meanLatencySim) ? tracker->ME_lat_ana_inf_GEG1 / tracker->meanLatencySim : tracker->meanLatencySim / tracker->ME_lat_ana_inf_GEG1);

    // tracker->vectorErrorDiff.push_back(tracker->ME_lat_ana_inf_GED1 - tracker->meanLatencySim);
    // tracker->vectorErrorDiffAbs.push_back(abs(tracker->ME_lat_ana_inf_GED1 - tracker->meanLatencySim));
    // tracker->vectorErrorPct.push_back((tracker->ME_lat_ana_inf_GED1 - tracker->meanLatencySim) / tracker->meanLatencySim * 100);
    // tracker->vectorErrorPctAbs.push_back(abs((tracker->ME_lat_ana_inf_GED1 - tracker->meanLatencySim)) / tracker->meanLatencySim * 100);
    // tracker->vectorErrorRatio.push_back((tracker->ME_lat_ana_inf_GED1 - tracker->meanLatencySim) ? tracker->ME_lat_ana_inf_GED1 / tracker->meanLatencySim : tracker->meanLatencySim / tracker->ME_lat_ana_inf_GED1);

    // tracker->vectorErrorDiff.push_back(tracker->meanServiceTime - tracker->meanLatencySim);
    // tracker->vectorErrorDiffAbs.push_back(abs(tracker->meanServiceTime - tracker->meanLatencySim));
    // tracker->vectorErrorPct.push_back((tracker->meanServiceTime - tracker->meanLatencySim) / tracker->meanLatencySim * 100);
    // tracker->vectorErrorPctAbs.push_back(abs((tracker->meanServiceTime - tracker->meanLatencySim)) / tracker->meanLatencySim * 100);
    // tracker->vectorErrorRatio.push_back((tracker->meanServiceTime - tracker->meanLatencySim) ? tracker->meanServiceTime / tracker->meanLatencySim : tracker->meanLatencySim / tracker->meanServiceTime);

    // tracker->minErrorDiff = *std::min_element(tracker->vectorErrorDiffAbs.begin(), tracker->vectorErrorDiffAbs.end());
    // tracker->minErrorPct = *std::min_element(tracker->vectorErrorPctAbs.begin(), tracker->vectorErrorPctAbs.end());
    // tracker->bestModel = std::min_element(tracker->vectorErrorPctAbs.begin(), tracker->vectorErrorPctAbs.end()) - tracker->vectorErrorPctAbs.begin();
    // tracker->bestModelRatio = std::min_element(tracker->vectorErrorRatio.begin(), tracker->vectorErrorRatio.end()) - tracker->vectorErrorRatio.begin();

    m_flowId.clear();
    v_lambda_arrival.clear();
    v_lambda_departure.clear();
    v_rho.clear();
    v_CA_sqr.clear();
    v_CS_sqr.clear();
  }
}

void
FlowLogV2::PrintLog()
{
  for(auto log : m_logs) {
    auto tracker = log.second;
    // skip the last stage
    if(tracker->isLastStage || tracker->rxPackets == 0) {
      continue;
    }

    Time now = Simulator::Now ();

    Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(tracker->srcAddress);      
    Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(tracker->desAddress); 
		char s[100];
		sprintf(s, "%d.%d.%d.%d", ((sourceAddress.Get() >> 24) & 0xff), ((sourceAddress.Get() >> 16) & 0xff), ((sourceAddress.Get() >> 8) & 0xff), ((sourceAddress.Get() >> 0) & 0xff));
    std::string s_str(s);
		char d[100];
		sprintf(d, "%d.%d.%d.%d", ((destinationAddress.Get() >> 24) & 0xff), ((destinationAddress.Get() >> 16) & 0xff), ((destinationAddress.Get() >> 8) & 0xff), ((destinationAddress.Get() >> 0) & 0xff));
    std::string d_str(d);

    int client = address_client_map[sourceAddress.Get()];
    int server = address_server_map[destinationAddress.Get()];

    std::ofstream of;

    std::string queue_flow_log_filename;
    queue_flow_log_filename.append("runs/" + tag + "/reports_sim/flow_queue_log.csv");
    of.open(queue_flow_log_filename, std::ios::out | std::ios::app);
    of << now.GetMilliSeconds()/1000*1000 << "," 
    << ns3::RngSeedManager::GetSeed() << ","
    // << sourceDataRate << "," 
    << tracker->flowId << ","
    << client << ","
    << server << ","
    // << s_str << ","
    // << d_str << ","
    << tracker->stage << "," 
    << tracker->nodeId << "," 
    << tracker->interface << "," 
    << tracker->meanDatarateArrival << "," 
    // << tracker->meanDatarateArrival_total << "," 
    << tracker->meanDatarateDeparture << "," 
    // << tracker->meanDatarateDeparture_total << "," 
    << tracker->rho << "," 
    // << tracker->rho_total << "," 
    << tracker->CA_sqr << "," 
    << tracker->CD_sqr << "," 
    << tracker->CS_sqr << "," 
    << tracker->meanPacketSize << "," 
    << tracker->txPackets << "," 
    << tracker->txBytes << ","
    << tracker->rxPackets << ","  
    << tracker->rxBytes << "," 
    << tracker->meanLatencySim << "," 
    << tracker->meanServiceTime << ","
    << ((fabs(tracker->meanLatencySim - tracker->meanServiceTime) >= 1e-5) ? tracker->meanLatencySim - tracker->meanServiceTime : 0) << ","
    << tracker->meanQueuePackets << ","
    << tracker->meanQueueBytes << std::endl;
    of.close();

    // double CD_sqr = 0;
    // auto find = m_queue_monitor->m_queuelogv3.m_queue_logs.find(std::make_pair(tracker->nodeId, tracker->interface));
    // if (find != m_queue_monitor->m_queuelogv3.m_queue_logs.end())
    // {
    //   CD_sqr = find->second->CD_sqr;
    // }

    if (tracker->stage == 0)
    {
      std::string time_window_filename;
      time_window_filename.append("runs/" + tag + "/outputs_sim/mimic_" + std::to_string(int(now.GetMilliSeconds()/1000) * 1000) + ".txt");
      of.open(time_window_filename, std::ios::out | std::ios::app);
      // time_window_filename.append(tag);
      // time_window_filename.append("/outputs_sim/");
      of 
      << "i" << tracker->flowId << ":"
      << "c" << client << ":"
      << "s" << server << ":"
      << "r" << tracker->meanDatarateArrival << ":"
      << "p" << tracker->meanPacketSize << ":"
      << "a" << tracker->CA_sqr << ":"
      << "b" << tracker->CS_sqr << ":"
      << "l" << tracker->meanLatencySim << ":"
      << "d" << tracker->CD_sqr << std::endl;
      of.close();
    }

    // of.open("statistics/verification_" + std::to_string(now.GetMilliSeconds()) + "_" + std::to_string(tracker->nodeId) + "_" + std::to_string(tracker->interface) + ".txt", std::ios::out | std::ios::app);
    //   of << "c" << client << ":"
    //   << "s" << server << ":"
    //   << "r" << tracker->meanDatarate << ":"
    //   << "p" << tracker->meanPacketSize << ":"
    //   << "a" << tracker->CA_sqr << ":"
    //   << "b" << tracker->CS_sqr << ":"
    //   << "l" << tracker->meanLatencySim << std::endl;
    // of.close();

  }  
}

void
FlowLogV2::PeriodicSampling ()
{
  Sampling ();
  if (Simulator::Now () <= Seconds(100))
    Simulator::Schedule (PERIODIC_SAMPLING_INTERVAL, &FlowLogV2::PeriodicSampling, this);
}

void
FlowLogV2::PeriodicUpdateWindow ()
{
  // auto start = std::chrono::system_clock::now();
  // std::time_t start_time = std::chrono::system_clock::to_time_t(start);
  // std::cout << "UpdateWindow start: " << std::ctime(&start_time) << std::endl;
  UpdateWindow ();
  Simulator::Schedule (m_TimeWindow - MicroSeconds(2), &FlowLogV2::PeriodicUpdateWindow, this);
  // auto end = std::chrono::system_clock::now();
  // std::time_t end_time = std::chrono::system_clock::to_time_t(end);
  // std::cout << "UpdateWindow finsih: " << std::ctime(&end_time) << std::endl;
}

void
FlowLogV2::PeriodicDoAnalyticalModel ()
{
  // auto start = std::chrono::system_clock::now();
  // std::time_t start_time = std::chrono::system_clock::to_time_t(start);
  // std::cout << "DoAnalyticalModel start: " << std::ctime(&start_time) << std::endl;
  DoAnalyticalModel ();
  Simulator::Schedule (m_TimeWindow - MicroSeconds(1), &FlowLogV2::PeriodicDoAnalyticalModel, this);
  // auto end = std::chrono::system_clock::now();
  // std::time_t end_time = std::chrono::system_clock::to_time_t(end);
  // std::cout << "DoAnalyticalModel finsih: " << std::ctime(&end_time) << std::endl;
}


void
FlowLogV2::PeriodicPrintLog ()
{
  // auto start = std::chrono::system_clock::now();
  // std::time_t start_time = std::chrono::system_clock::to_time_t(start);
  // std::cout << "PrintLog start: " << std::ctime(&start_time) << std::endl;
  PrintLog ();
  Simulator::Schedule (m_TimeWindow, &FlowLogV2::PeriodicPrintLog, this);
  // auto end = std::chrono::system_clock::now();
  // std::time_t end_time = std::chrono::system_clock::to_time_t(end);
  // std::cout << "PrintLog finish: " << std::ctime(&end_time) << std::endl;
}


void
FlowLogV2::NotifyConstructionCompleted ()
{
  Object::NotifyConstructionCompleted ();
}


} // namespace ns3
