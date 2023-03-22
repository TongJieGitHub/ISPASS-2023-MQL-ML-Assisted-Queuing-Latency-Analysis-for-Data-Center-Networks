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
#include <vector>
#include <algorithm>
#include <numeric>

#include "flow-log.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include <fstream>
#include <sstream>
#include "ns3/node.h"
#include "ns3/rng-seed-manager.h"

#define PERIODIC_WINDOW_INTERVAL (MicroSeconds (199998))
#define PERIODIC_ANA_INTERVAL (MicroSeconds (199999))
#define PERIODIC_PRINT_INTERVAL (MicroSeconds (200000))

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FlowLog");

NS_OBJECT_ENSURE_REGISTERED (FlowLog);

TypeId 
FlowLog::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FlowLog")
    .SetParent<Object> ()
    .SetGroupName ("FlowLog")
    .AddConstructor<FlowLog> ()
    .AddAttribute ("TimeWindow", 
                   "TimeWindow",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&FlowLog::m_TimeWindow),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId 
FlowLog::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

FlowLog::FlowLog() 
{
  Simulator::Schedule (PERIODIC_WINDOW_INTERVAL, &FlowLog::PeriodicUpdateWindow, this);
  Simulator::Schedule (PERIODIC_ANA_INTERVAL, &FlowLog::PeriodicDoAnalyticalModel, this);
  Simulator::Schedule (PERIODIC_PRINT_INTERVAL, &FlowLog::PeriodicPrintLog, this);
}

FlowLog::~FlowLog() 
{
    
}

void
FlowLog::CreateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t stage, Address srcAddress, Address desAddress, std::unordered_map<uint32_t, int> address_client_map, std::unordered_map<uint32_t, int> address_server_map)
{
  Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  int client = address_client_map[sourceAddress.Get()];
  Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(std::make_pair(std::make_pair(client, server), (std::make_pair(probe->GetNode()->GetId(), interface))));
  if(search == m_logs.end()) {
    Ptr<FlowLogTracker> tracker = ns3::Create<FlowLogTracker>();
    tracker->stage = stage;
    tracker->nodeId = probe->GetNode()->GetId();
    tracker->interface = interface;
    tracker->srcAddress = srcAddress;
    tracker->desAddress = desAddress;
    tracker->isLastStage = false;
    m_logs.insert(std::make_pair(std::make_pair(std::make_pair(client, server), (std::make_pair(probe->GetNode()->GetId(), interface))), tracker));
    this->address_client_map = address_client_map;
    this->address_server_map = address_server_map;
  }
}

void
FlowLog::UpdateLog (Ptr<FlowProbe> probe, uint32_t interface, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency, bool isLastStage)
{
  Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  int client = address_client_map[sourceAddress.Get()];
  Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(std::make_pair(std::make_pair(client, server), (std::make_pair(probe->GetNode()->GetId(), interface))));
  if(search != m_logs.end()) {
    auto tracker = search->second;
    Time now = Simulator::Now ();
    // std::ofstream of;
    tracker->vectorTime.push_back(now.GetNanoSeconds());
    tracker->vectorPacketSizeTx.push_back(packetSize + 2);
    // tracker->vectorLatency.push_back(latency);
    size_t sz = tracker->vectorTime.size();
    if (sz > 1) {
      tracker->vectorInterarrival.push_back(static_cast<double>(tracker->vectorTime.at(sz - 1) - tracker->vectorTime.at(sz - 2)));
    }
    if (isLastStage) {
      tracker->isLastStage = true;
    }
  }
}

void 
FlowLog::UpdateLatency (Ptr<FlowProbe> probe, uint32_t interface, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency)
{
  Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  int client = address_client_map[sourceAddress.Get()];
  Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(std::make_pair(std::make_pair(client, server), (std::make_pair(probe->GetNode()->GetId(), interface))));
  if(search != m_logs.end()) {
    auto tracker = search->second;
    tracker->vectorPacketSizeRx.push_back(packetSize + 2);
    tracker->vectorLatency.push_back(latency);
  }
}

void
FlowLog::UpdateWindow()
{
  for(auto log : m_logs) {
    auto tracker = log.second;
    Time now = Simulator::Now ();

    // double meanDatarate = static_cast<double>(std::accumulate(tracker->vectorPacketSizeTx.begin(), tracker->vectorPacketSizeTx.end(), 0)) * 8 * 1000 / (tracker->vectorTime.back() - tracker->vectorTime.front());
    double meanDatarate = static_cast<double>(std::accumulate(tracker->vectorPacketSizeTx.begin(), tracker->vectorPacketSizeTx.end(), 0)) * 8 * 1000 / (PERIODIC_WINDOW_INTERVAL.GetNanoSeconds());
    tracker->meanDatarate = meanDatarate;

    double mean = std::accumulate(tracker->vectorInterarrival.begin(), tracker->vectorInterarrival.end(), 0.0) / tracker->vectorInterarrival.size();
    double var = 0.0;
    for (size_t i = 0; i < tracker->vectorInterarrival.size(); i++) {
      var += pow(tracker->vectorInterarrival.at(i) - mean ,2);
    }
    var = var / tracker->vectorInterarrival.size();
    double CA_sqr = var / (mean * mean);
    tracker->meanInterarrival = mean;
    tracker->CA_sqr = CA_sqr;

    mean = std::accumulate(tracker->vectorPacketSizeRx.begin(), tracker->vectorPacketSizeRx.end(), 0.0) / tracker->vectorPacketSizeRx.size() * 8 * 10;
    var = 0.0;
    for (size_t i = 0; i < tracker->vectorPacketSizeRx.size(); i++) {
      var += pow(tracker->vectorPacketSizeRx.at(i) * 8 * 10 - mean ,2);
    }
    var = var / tracker->vectorPacketSizeRx.size();
    double CS_sqr= var / (mean * mean);
    tracker->meanServiceTime = mean;
    tracker->CS_sqr = CS_sqr;

    int64_t sumPacketSizeTx = std::accumulate(tracker->vectorPacketSizeTx.begin(), tracker->vectorPacketSizeTx.end(), 0);
    double meanPacketSizeTx = static_cast<double>(sumPacketSizeTx) / tracker->vectorPacketSizeTx.size();
    tracker->txPackets = tracker->vectorPacketSizeTx.size();
    tracker->txBytes = sumPacketSizeTx;
    
    
    // double meanLatency = static_cast<double>(std::accumulate(tracker->vectorLatency.begin(), tracker->vectorLatency.end(), 0) / tracker->vectorLatency.size());
    double latency_total = 0;
    for (auto m : tracker->vectorLatency) {
      latency_total += static_cast<double>(m) / 1e6;
    }
    if(tracker->vectorLatency.size() != 0) {
      double meanLatency = latency_total / tracker->vectorLatency.size() * 1e6;
      tracker->meanLatencySim = meanLatency;

      int64_t sumPacketSizeRx = std::accumulate(tracker->vectorPacketSizeRx.begin(), tracker->vectorPacketSizeRx.end(), 0);
      double meanPacketSizeRx = static_cast<double>(sumPacketSizeRx) / tracker->vectorPacketSizeRx.size();
      tracker->rxPackets = tracker->vectorPacketSizeRx.size();
      tracker->rxBytes = sumPacketSizeRx;
      tracker->meanPacketSize = meanPacketSizeRx;
    }



    tracker->vectorTime.clear();
    tracker->vectorPacketSizeTx.clear();
    tracker->vectorPacketSizeRx.clear();
    tracker->vectorInterarrival.clear();
    tracker->vectorLatency.clear();
    tracker->startTime_sim = now.GetNanoSeconds();
  }
}

void 
FlowLog::DoAnalyticalModel ()
{
  for(auto log : m_logs) {
    auto tracker = log.second;

    // skip the last stage
    if(tracker->isLastStage) {
      continue;
    }
    
    Time now = Simulator::Now ();
    // analytical models
    std::vector<double> v_lambda, v_rho, v_CA_sqr, v_CS_sqr;
    std::map<uint32_t, int> m_flowId;
    // uint32_t current_flowId = tracker->flowId;
    // uint32_t current_stage = tracker->stage;
    double current_lambda = tracker->meanDatarate;
    double current_rho = tracker->meanDatarate / 100;
    // double current_CA_sqr = tracker->CA_sqr;
    // double current_minInterarrival = tracker->minInterarrival;
    // double current_CS_sqr = tracker->CS_sqr;
    uint32_t current_nodeId = tracker->nodeId;
    uint32_t current_interface = tracker->interface;
    double total_rho = 0;
    double total_datarate = 0;
    double mnl_inf = 0;
    //double mql_inf = 0;
    double ME_lat_ana_inf_GEG1 = 0;
    double ME_lat_ana_inf_GED1 = 0;
    
    for(auto m: m_logs) {
      // skip the last stage
      if(m.second->isLastStage) {
      continue;
      }

      if (m.second->nodeId == current_nodeId && m.second->interface == current_interface) {
        v_lambda.push_back(m.second->meanDatarate);
        v_rho.push_back(m.second->meanDatarate / 100);
        v_CA_sqr.push_back(m.second->CA_sqr);
        v_CS_sqr.push_back(m.second->CS_sqr);
        m_flowId[m.second->flowId]++;
        //std::cout <<  "(" << m.first.first << "," << m.first.second << "), "; 
      }
    }
    //std::cout << std::endl;
    //std::cout << current_nodeId << ", " << current_interface << ": " << v_lambda.size() << " " << v_rho.size() << " " << v_CA_sqr.size() << " " << std::endl;

    total_rho = std::accumulate(v_rho.begin(), v_rho.end(), 0.0);
    total_datarate = std::accumulate(v_lambda.begin(), v_lambda.end(), 0.0);
    mnl_inf = 0.5 * current_rho * (1 + 1);
    for (size_t i = 0; i < v_rho.size(); i++) {
      mnl_inf += (current_lambda / v_lambda.at(i)) * v_rho.at(i) * v_rho.at(i) * (v_CA_sqr.at(i) + v_CS_sqr.at(i)) / (1 - total_rho) * 0.5;
    }
    ME_lat_ana_inf_GEG1 = (8 * tracker->meanPacketSize * 1000) * mnl_inf / current_lambda;
    tracker->ME_lat_ana_inf_GEG1 = ME_lat_ana_inf_GEG1;

    mnl_inf = 0.5 * current_rho * (1 + 1);
    for (size_t i = 0; i < v_rho.size(); i++) {
      mnl_inf += (current_lambda / v_lambda.at(i)) * v_rho.at(i) * v_rho.at(i) * (v_CA_sqr.at(i) + 0) / (1 - total_rho) * 0.5;
    }
    ME_lat_ana_inf_GED1 = (8 * tracker->meanPacketSize * 1000) * mnl_inf / current_lambda;
    tracker->ME_lat_ana_inf_GED1 = ME_lat_ana_inf_GED1;

    tracker->meanDatarate_total = total_datarate;
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
    v_lambda.clear();
    v_rho.clear();
    v_CA_sqr.clear();
    v_CS_sqr.clear();
    tracker->startTime_ana = now.GetNanoSeconds();
  }
}

void
FlowLog::PrintLog()
{
  for(auto log : m_logs) {
    auto tracker = log.second;
    // skip the last stage
    if(tracker->isLastStage || tracker->txPackets == 0 || tracker->txPackets == 1 || tracker->rxPackets == 0) {
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
    // of.open("flowlog_" + s_str + "_" + d_str +  "_stage"+ std::to_string(tracker->stage) + ".csv", std::ios::out | std::ios::app);
    // of << now.GetMilliSeconds() << "," 
    // << ns3::RngSeedManager::GetSeed() << ","
    // << (now.GetMilliSeconds() / 1000 - 1) / 10 + 1 << "," 
    // << tracker->nodeId << "," 
    // << tracker->interface << "," 
    // << tracker->meanDatarate << "," 
    // << tracker->CA_sqr << "," 
    // << tracker->CS_sqr << "," 
    // << tracker->rho << "," 
    // << tracker->rho_total << "," 
    // << tracker->num_flows << "," 
    // << tracker->meanPacketSize << "," 
    // << tracker->txPackets << "," 
    // << tracker->txBytes << ","
    // << tracker->rxPackets << ","  
    // << tracker->rxBytes << "," 
    // << tracker->meanLatencySim << "," 
    // << tracker->ME_lat_ana_inf_GEG1 << "," 
    // << tracker->ME_lat_ana_inf_GED1 << "," 
    // << tracker->meanServiceTime << ","
    // << tracker->vectorErrorDiff.at(0) << ","
    // << tracker->vectorErrorDiff.at(1) << ","
    // << tracker->vectorErrorDiff.at(2) << ","
    // << tracker->vectorErrorPct.at(0) << ","
    // << tracker->vectorErrorPct.at(1) << ","
    // << tracker->vectorErrorPct.at(2) << ","
    // << tracker->minErrorDiff << ","
    // << tracker->minErrorPct << ","
    // << tracker->bestModel << std::endl;
    // of.close();

    of.open("statistics/flowlog_" + filename, std::ios::out | std::ios::app);
    of << now.GetMilliSeconds() << "," 
    << ns3::RngSeedManager::GetSeed() << ","
    // << sourceDataRate << "," 
    << client << ","
    << server << ","
    << s_str << ","
    << d_str << ","
    << tracker->stage << "," 
    << tracker->nodeId << "," 
    << tracker->interface << "," 
    << tracker->meanDatarate << "," 
    << tracker->meanDatarate_total << "," 
    << tracker->rho << "," 
    << tracker->rho_total << "," 
    // << tracker->num_flows << "," 
    << tracker->CA_sqr << "," 
    << tracker->CS_sqr << "," 
    << tracker->meanPacketSize << "," 
    << tracker->txPackets << "," 
    << tracker->txBytes << ","
    << tracker->rxPackets << ","  
    << tracker->rxBytes << "," 
    << tracker->meanLatencySim << "," 
    // << tracker->ME_lat_ana_inf_GEG1 << "," 
    // << tracker->ME_lat_ana_inf_GED1 << "," 
    << tracker->meanServiceTime << std::endl;
    // << tracker->vectorErrorDiff.at(0) << ","
    // << tracker->vectorErrorDiff.at(1) << ","
    // << tracker->vectorErrorDiff.at(2) << ","
    // << tracker->vectorErrorPct.at(0) << ","
    // << tracker->vectorErrorPct.at(1) << ","
    // << tracker->vectorErrorPct.at(2) << ","
    // << tracker->minErrorDiff << ","
    // << tracker->minErrorPct << ","
    // << tracker->bestModel << std::endl;
    of.close();

    double CD_sqr = 0;
    auto find = m_queue_monitor->m_queuelogv3.m_queue_logs.find(std::make_pair(tracker->nodeId, tracker->interface));
    if (find != m_queue_monitor->m_queuelogv3.m_queue_logs.end())
    {
      CD_sqr = find->second->CD_sqr;
    }

    if (tracker->stage == 0)
    {
      std::string time_window_filename;
      time_window_filename.append("runs/" + tag + "/outputs_sim/mimic_");
      // time_window_filename.append(tag);
      // time_window_filename.append("/outputs_sim/");
      // time_window_filename.append("/outputs_sim/");
      of.open(time_window_filename + std::to_string(now.GetMilliSeconds()) + ".txt", std::ios::out | std::ios::app);
      of << "c" << client << ":"
      << "s" << server << ":"
      << "r" << tracker->meanDatarate << ":"
      << "p" << tracker->meanPacketSize << ":"
      << "a" << tracker->CA_sqr << ":"
      << "b" << tracker->CS_sqr << ":"
      << "l" << tracker->meanLatencySim << ":"
      << "d" << CD_sqr << std::endl;
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
FlowLog::PeriodicUpdateWindow ()
{
  UpdateWindow ();
  Simulator::Schedule (PERIODIC_WINDOW_INTERVAL, &FlowLog::PeriodicUpdateWindow, this);
}

void
FlowLog::PeriodicDoAnalyticalModel ()
{
  DoAnalyticalModel ();
  Simulator::Schedule (PERIODIC_ANA_INTERVAL, &FlowLog::PeriodicDoAnalyticalModel, this);
}


void
FlowLog::PeriodicPrintLog ()
{
  PrintLog ();
  Simulator::Schedule (PERIODIC_PRINT_INTERVAL, &FlowLog::PeriodicPrintLog, this);
}


void
FlowLog::NotifyConstructionCompleted ()
{
  Object::NotifyConstructionCompleted ();
}


} // namespace ns3
