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

#include "queue-log.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include <fstream>
#include <sstream>
#include "ns3/node.h"
#include "ns3/rng-seed-manager.h"

#define PERIODIC_WINDOW_INTERVAL (MilliSeconds (498))
#define PERIODIC_ANA_INTERVAL (MilliSeconds (499))
#define PERIODIC_PRINT_INTERVAL (MilliSeconds (500))

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QueueLog");

NS_OBJECT_ENSURE_REGISTERED (QueueLog);

TypeId 
QueueLog::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QueueLog")
    .SetParent<Object> ()
    .SetGroupName ("QueueLog")
    .AddConstructor<QueueLog> ()
    .AddAttribute ("TimeWindow", 
                   "TimeWindow",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&QueueLog::m_TimeWindow),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId 
QueueLog::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

QueueLog::QueueLog() 
{
  Simulator::Schedule (PERIODIC_WINDOW_INTERVAL, &QueueLog::PeriodicUpdateWindow, this);
  Simulator::Schedule (PERIODIC_ANA_INTERVAL, &QueueLog::PeriodicDoAnalyticalModel, this);
  Simulator::Schedule (PERIODIC_PRINT_INTERVAL, &QueueLog::PeriodicPrintLog, this);
}

QueueLog::~QueueLog() 
{
    
}

void
QueueLog::CreateFlowPerQueueLog (Ptr<FlowProbe> probe, uint32_t interface, Ptr<FlowProbe> probeFrom, uint32_t interfaceFrom)
{
  auto search = m_flow_per_queue_logs.find(std::make_pair(std::make_pair(probe->GetNode()->GetId(), interface), (std::make_pair(probeFrom->GetNode()->GetId(), interfaceFrom))));
  if(search == m_flow_per_queue_logs.end()) {
    Ptr<FlowPerQueueLogTracker> tracker = ns3::Create<FlowPerQueueLogTracker>();
    tracker->nodeId = probe->GetNode()->GetId();
    tracker->interface = interface;
    tracker->nodeIdFrom = probeFrom->GetNode()->GetId();
    tracker->interfaceFrom = interfaceFrom;
    m_flow_per_queue_logs.insert(std::make_pair(std::make_pair(std::make_pair(probe->GetNode()->GetId(), interface), (std::make_pair(probeFrom->GetNode()->GetId(), interfaceFrom))), tracker));
  }
}


void
QueueLog::UpdateFlowPerQueueLog (Ptr<FlowProbe> probe, uint32_t interface, Ptr<FlowProbe> probeFrom, uint32_t interfaceFrom, uint32_t packetSize)
{
  auto search = m_flow_per_queue_logs.find(std::make_pair(std::make_pair(probe->GetNode()->GetId(), interface), (std::make_pair(probeFrom->GetNode()->GetId(), interfaceFrom))));
  if(search != m_flow_per_queue_logs.end()) {
    auto tracker = search->second;
    Time now = Simulator::Now ();
    // std::ofstream of;
    tracker->vectorTime.push_back(now.GetNanoSeconds());
    tracker->vectorPacketSize.push_back(packetSize + 2);
    // tracker->vectorLatency.push_back(latency);
    size_t sz = tracker->vectorTime.size();
    if (sz > 1) {
      tracker->vectorInterarrival.push_back(static_cast<double>(tracker->vectorTime.at(sz - 1) - tracker->vectorTime.at(sz - 2)));
    }
  }
}

void
QueueLog::CreateQueueLog(Ptr<FlowProbe> probe, uint32_t interface) 
{
  auto search = m_queue_logs.find(std::make_pair(probe->GetNode()->GetId(), interface));
  if(search == m_queue_logs.end()) {
    Ptr<QueueLogTracker> tracker = ns3::Create<QueueLogTracker>();
    tracker->nodeId = probe->GetNode()->GetId();
    tracker->interface = interface;
    m_queue_logs.insert(std::make_pair(std::make_pair(probe->GetNode()->GetId(), interface), tracker));
  }
}


void 
QueueLog::UpdateQueueLog(Ptr<FlowProbe> probe, uint32_t interface, uint32_t packetSize, int64_t latency)
{
  auto search = m_queue_logs.find(std::make_pair(probe->GetNode()->GetId(), interface));
  if(search != m_queue_logs.end()) {
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
  }
}

void 
QueueLog::UpdateQueueLatency(Ptr<FlowProbe> probe, uint32_t interface, uint32_t packetSize, int64_t latency)
{
  auto search = m_queue_logs.find(std::make_pair(probe->GetNode()->GetId(), interface));
  if(search != m_queue_logs.end()) {
    auto tracker = search->second;
    tracker->vectorPacketSizeRx.push_back(packetSize + 2);
    tracker->vectorLatency.push_back(latency);
  }
}


void
QueueLog::UpdateWindow()
{
  for(auto log : m_flow_per_queue_logs) {
    auto tracker = log.second;
    Time now = Simulator::Now ();

    double meanDatarate = static_cast<double>(std::accumulate(tracker->vectorPacketSize.begin(), tracker->vectorPacketSize.end(), 0)) * 8 * 1000 / (tracker->vectorTime.back() - tracker->vectorTime.front());
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

    mean = std::accumulate(tracker->vectorPacketSize.begin(), tracker->vectorPacketSize.end(), 0.0) / tracker->vectorPacketSize.size() * 8 * 10;
    var = 0.0;
    for (size_t i = 0; i < tracker->vectorPacketSize.size(); i++) {
      var += pow(tracker->vectorPacketSize.at(i) * 8 * 10 - mean ,2);
    }
    var = var / tracker->vectorPacketSize.size();
    double CS_sqr= var / (mean * mean);
    tracker->meanServiceTime = mean;
    tracker->CS_sqr = CS_sqr;

    int64_t sumPacketSize = std::accumulate(tracker->vectorPacketSize.begin(), tracker->vectorPacketSize.end(), 0);
    double meanPacketSize = static_cast<double>(sumPacketSize) / tracker->vectorPacketSize.size();
    tracker->txPackets = tracker->vectorPacketSize.size();
    tracker->txBytes = sumPacketSize;
    tracker->meanPacketSize = meanPacketSize;

    tracker->lambda = meanDatarate * 1000000 / (meanPacketSize * 8);
    tracker->mu = 100 * 1000000 / (meanPacketSize * 8);
    tracker->rho = tracker->lambda / tracker->mu;

    tracker->vectorTime.clear();
    tracker->vectorPacketSize.clear();
    tracker->vectorInterarrival.clear();
    tracker->startTime_sim = now.GetNanoSeconds();
  }

  for(auto log : m_queue_logs) {
    auto tracker = log.second;
    Time now = Simulator::Now ();

    double meanDatarate = static_cast<double>(std::accumulate(tracker->vectorPacketSizeTx.begin(), tracker->vectorPacketSizeTx.end(), 0)) * 8 * 1000 / (tracker->vectorTime.back() - tracker->vectorTime.front());
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

    mean = std::accumulate(tracker->vectorPacketSizeTx.begin(), tracker->vectorPacketSizeTx.end(), 0.0) / tracker->vectorPacketSizeTx.size() * 8 * 10;
    var = 0.0;
    for (size_t i = 0; i < tracker->vectorPacketSizeTx.size(); i++) {
      var += pow(tracker->vectorPacketSizeTx.at(i) * 8 * 10 - mean ,2);
    }
    var = var / tracker->vectorPacketSizeTx.size();
    double CS_sqr= var / (mean * mean);
    tracker->meanServiceTime = mean;
    tracker->CS_sqr = CS_sqr;

    int64_t sumPacketSizeTx = std::accumulate(tracker->vectorPacketSizeTx.begin(), tracker->vectorPacketSizeTx.end(), 0);
    double meanPacketSizeTx = static_cast<double>(sumPacketSizeTx) / tracker->vectorPacketSizeTx.size();
    tracker->txPackets = tracker->vectorPacketSizeTx.size();
    tracker->txBytes = sumPacketSizeTx;
    tracker->meanPacketSize = meanPacketSizeTx;
    
    
    // double meanLatency = static_cast<double>(std::accumulate(tracker->vectorLatency.begin(), tracker->vectorLatency.end(), 0) / tracker->vectorLatency.size());
    int128_t latency_total = 0;
    for (auto m : tracker->vectorLatency) {
      latency_total += m;
    }
    if(tracker->vectorLatency.size() != 0) {
      double meanLatency = static_cast<double>(latency_total / (tracker->vectorLatency.size() + 1));
      tracker->meanLatencySim = meanLatency;

      int64_t sumPacketSizeRx = std::accumulate(tracker->vectorPacketSizeRx.begin(), tracker->vectorPacketSizeRx.end(), 0);
      tracker->rxPackets = tracker->vectorPacketSizeRx.size();
      tracker->rxBytes = sumPacketSizeRx;
    }

    tracker->lambda = meanDatarate * 1000000 / (meanPacketSizeTx * 8);
    tracker->mu = 100 * 1000000 / (meanPacketSizeTx * 8);
    tracker->rho = tracker->lambda / tracker->mu;

    tracker->vectorTime.clear();
    tracker->vectorPacketSizeTx.clear();
    tracker->vectorPacketSizeRx.clear();
    tracker->vectorInterarrival.clear();
    tracker->vectorLatency.clear();
    tracker->startTime_sim = now.GetNanoSeconds();
  }
}


void 
QueueLog::DoAnalyticalModel ()
{
  for(auto log : m_flow_per_queue_logs) {
    auto tracker = log.second;

    Time now = Simulator::Now ();
    // analytical models
    std::vector<double> v_lambda, v_rho, v_CA_sqr, v_CS_sqr;
    // uint32_t current_flowId = tracker->flowId;
    // uint32_t current_stage = tracker->stage;
    double current_lambda = tracker->lambda;
    double current_rho = tracker->rho;
    double current_CA_sqr = tracker->CA_sqr;
    // double current_minInterarrival = tracker->minInterarrival;
    // double current_CS_sqr = tracker->CS_sqr;
    uint32_t current_nodeId = tracker->nodeId;
    uint32_t current_interface = tracker->interface;
    double total_rho = 0;
    double total_lambda = 0;
    double mnl_inf = 0;
    double mnl_inf_total = 0;
    double ME_lat_ana_inf_GEG1 = 0;
    double ME_lat_ana_inf_GEG1_sys = 0;
    
    for(auto m: m_flow_per_queue_logs) {
      if (m.second->nodeId == current_nodeId && m.second->interface == current_interface) {
        v_lambda.push_back(m.second->lambda);
        v_rho.push_back(m.second->rho);
        v_CA_sqr.push_back(m.second->CA_sqr);
        v_CS_sqr.push_back(m.second->CS_sqr);
        //std::cout <<  "(" << m.first.first << "," << m.first.second << "), "; 
      }
    }
    //std::cout << std::endl;
    //std::cout << current_nodeId << ", " << current_interface << ": " << v_lambda.size() << " " << v_rho.size() << " " << v_CA_sqr.size() << " " << std::endl;

    total_rho = std::accumulate(v_rho.begin(), v_rho.end(), 0.0);
    total_lambda = std::accumulate(v_lambda.begin(), v_lambda.end(), 0.0);

    for (size_t i = 0; i < v_rho.size(); i++) {
      current_lambda = v_lambda.at(i);
      current_CA_sqr = v_CA_sqr.at(i);
      // current_CS_sqr = v_CS_sqr.at(i);
      mnl_inf = 0.5 * current_rho * (current_CA_sqr + 1);
      for (size_t i = 0; i < v_rho.size(); i++) {
        mnl_inf += (current_lambda / v_lambda.at(i)) * v_rho.at(i) * v_rho.at(i) * (v_CA_sqr.at(i) + v_CS_sqr.at(i)) / (1 - total_rho) * 0.5;
      }
      mnl_inf_total += mnl_inf;
    }
    if(mnl_inf_total - total_rho < 0) {
      ME_lat_ana_inf_GEG1 = 0;
      ME_lat_ana_inf_GEG1_sys = ME_lat_ana_inf_GEG1 + tracker->meanServiceTime;
    }
    else {
      ME_lat_ana_inf_GEG1 = (mnl_inf_total - total_rho) / total_lambda * 1000000000;
      ME_lat_ana_inf_GEG1_sys = ME_lat_ana_inf_GEG1 + tracker->meanServiceTime;
    }

    tracker->ME_lat_ana_inf_GEG1 = ME_lat_ana_inf_GEG1;
    tracker->ME_lat_ana_inf_GEG1_sys = ME_lat_ana_inf_GEG1_sys;

    tracker->lambda_total = total_lambda;
    tracker->rho_total = total_rho;
    tracker->num_flows = v_rho.size();

    v_lambda.clear();
    v_rho.clear();
    v_CA_sqr.clear();
    v_CS_sqr.clear();
    tracker->startTime_ana = now.GetNanoSeconds();
  }
}


void
QueueLog::PrintLog()
{
  for(auto log : m_flow_per_queue_logs) {
    auto tracker = log.second;

    if (tracker->txPackets == 0) {
      continue;
    }

    double meanLatencySim = 0;
    auto search = m_queue_logs.find(std::make_pair(tracker->nodeId, tracker->interface));
    if(search != m_queue_logs.end()) {
      meanLatencySim = search->second->meanLatencySim;
    }

    Time now = Simulator::Now ();

    std::ofstream of;

    of.open("statistics/queuelog_" + filename, std::ios::out | std::ios::app);
    of << now.GetMilliSeconds() << "," 
    << ns3::RngSeedManager::GetSeed() << ","
    << sourceDataRate << "," 
    << tracker->nodeId << "," 
    << tracker->interface << "," 
    << tracker->nodeIdFrom << "," 
    << tracker->interfaceFrom << "," 
    << tracker->meanDatarate << "," 
    << tracker->lambda << "," 
    << tracker->lambda_total << "," 
    << tracker->mu << "," 
    << tracker->rho << "," 
    << tracker->rho_total << "," 
    << tracker->num_flows << "," 
    << tracker->CA_sqr << "," 
    << tracker->CS_sqr << "," 
    << tracker->meanPacketSize << "," 
    << tracker->txPackets << "," 
    << tracker->txBytes << ","
    // << tracker->ME_lat_ana_inf_GEG1 << "," 
    << meanLatencySim << std::endl; 
    // << tracker->ME_lat_ana_inf_GEG1_sys << "," 
    // << tracker->meanServiceTime << std::endl;
    of.close();
  }  
}


void
QueueLog::PeriodicUpdateWindow ()
{
  UpdateWindow ();
  Simulator::Schedule (PERIODIC_WINDOW_INTERVAL, &QueueLog::PeriodicUpdateWindow, this);
}

void
QueueLog::PeriodicDoAnalyticalModel ()
{
  DoAnalyticalModel ();
  Simulator::Schedule (PERIODIC_ANA_INTERVAL, &QueueLog::PeriodicDoAnalyticalModel, this);
}


void
QueueLog::PeriodicPrintLog ()
{
  PrintLog ();
  Simulator::Schedule (PERIODIC_PRINT_INTERVAL, &QueueLog::PeriodicPrintLog, this);
}

void
QueueLog::NotifyConstructionCompleted ()
{
  Object::NotifyConstructionCompleted ();
}


} // namespace ns3