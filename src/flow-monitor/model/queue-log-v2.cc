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

#include "queue-log-v2.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include <fstream>
#include <sstream>
#include "ns3/node.h"
#include "ns3/rng-seed-manager.h"

#define PERIODIC_WINDOW_INTERVAL (MicroSeconds (199999))
#define PERIODIC_ANA_INTERVAL (MicroSeconds (199999))
#define PERIODIC_PRINT_INTERVAL (MicroSeconds (200000))

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QueueLogV2");

NS_OBJECT_ENSURE_REGISTERED (QueueLogV2);

TypeId 
QueueLogV2::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QueueLogV2")
    .SetParent<Object> ()
    .SetGroupName ("QueueLogV2")
    .AddConstructor<QueueLogV2> ()
    .AddAttribute ("TimeWindow", 
                   "TimeWindow",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&QueueLogV2::m_TimeWindow),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId 
QueueLogV2::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

QueueLogV2::QueueLogV2() 
{
  Simulator::Schedule (PERIODIC_WINDOW_INTERVAL, &QueueLogV2::PeriodicUpdateWindow, this);
  Simulator::Schedule (PERIODIC_ANA_INTERVAL, &QueueLogV2::PeriodicDoAnalyticalModel, this);
  Simulator::Schedule (PERIODIC_PRINT_INTERVAL, &QueueLogV2::PeriodicPrintLog, this);
}

QueueLogV2::~QueueLogV2() 
{
    
}


void
QueueLogV2::CreateQueueLog(Ptr<FlowProbe> probe, uint32_t interface) 
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
QueueLogV2::UpdateQueueLog(Ptr<FlowProbe> probe, uint32_t interface, uint32_t packetSize, int64_t latency)
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
QueueLogV2::UpdateQueueLatency(Ptr<FlowProbe> probe, uint32_t interface, uint32_t packetSize, int64_t latency)
{
  auto search = m_queue_logs.find(std::make_pair(probe->GetNode()->GetId(), interface));
  if(search != m_queue_logs.end()) {
    auto tracker = search->second;
    tracker->vectorPacketSizeRx.push_back(packetSize + 2);
    tracker->vectorLatency.push_back(latency);
  }
}

void 
QueueLogV2::UpdateQueueDrop(Ptr<FlowProbe> probe, uint32_t interface, uint32_t packetSize)
{
  auto search = m_queue_logs.find(std::make_pair(probe->GetNode()->GetId(), interface));
  if(search != m_queue_logs.end()) {
    auto tracker = search->second;
    tracker->vectorPacketSizeDrop.push_back(packetSize + 2);
  }
}


void
QueueLogV2::UpdateWindow()
{

  for(auto log : m_queue_logs) {
    auto tracker = log.second;
    Time now = Simulator::Now ();

    // double meanDatarate = static_cast<double>(std::accumulate(tracker->vectorPacketSizeTx.begin(), tracker->vectorPacketSizeTx.end(), 0) - tracker->vectorPacketSizeTx.back()) * 8 * 1000 / (tracker->vectorTime.back() - tracker->vectorTime.front());
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
      double meanLatency = static_cast<double>(latency_total / (tracker->vectorLatency.size()));
      tracker->meanLatencySim = meanLatency;

      int64_t sumPacketSizeRx = std::accumulate(tracker->vectorPacketSizeRx.begin(), tracker->vectorPacketSizeRx.end(), 0);
      tracker->rxPackets = tracker->vectorPacketSizeRx.size();
      tracker->rxBytes = sumPacketSizeRx;
    }

    tracker->lambda = meanDatarate * 1000000 / (meanPacketSizeTx * 8);
    tracker->mu = 100 * 1000000 / (meanPacketSizeTx * 8);
    tracker->rho = tracker->lambda / tracker->mu;

    int64_t sumPacketSizeDrop = std::accumulate(tracker->vectorPacketSizeDrop.begin(), tracker->vectorPacketSizeDrop.end(), 0);
    double meanPacketSizeDrop = tracker->vectorPacketSizeDrop.size() == 0 ? 0 : static_cast<double>(sumPacketSizeDrop) / tracker->vectorPacketSizeDrop.size();
    double meanDatarateEff =  static_cast<double>(sumPacketSizeTx - sumPacketSizeDrop) * 8 * 1000 / (PERIODIC_WINDOW_INTERVAL.GetNanoSeconds());
    double meanPacketSizeTxEff;
    if (tracker->vectorPacketSizeTx.size() == tracker->vectorPacketSizeDrop.size())
    {
      meanPacketSizeTxEff = 0;
    } else
    {
      meanPacketSizeTxEff = (sumPacketSizeTx - sumPacketSizeDrop) / (tracker->vectorPacketSizeTx.size() - tracker->vectorPacketSizeDrop.size());
    }

    tracker->meanDatarateEff = meanDatarateEff;
    tracker->dropPackets = tracker->vectorPacketSizeDrop.size();
    tracker->dropBytes = sumPacketSizeDrop;
    tracker->meanDropPacketSize = meanPacketSizeDrop;
    tracker->lambda_eff = meanDatarateEff * 1000000 / (meanPacketSizeTxEff * 8);
    tracker->rho_eff = tracker->meanDatarateEff / 100;

    tracker->vectorTime.clear();
    tracker->vectorPacketSizeTx.clear();
    tracker->vectorPacketSizeRx.clear();
    tracker->vectorInterarrival.clear();
    tracker->vectorLatency.clear();
    tracker->vectorPacketSizeDrop.clear();
    tracker->startTime_sim = now.GetNanoSeconds();
  }
}


void
QueueLogV2::PrintLog()
{
  for(auto log : m_queue_logs) {
    auto tracker = log.second;

    if (tracker->txPackets == 0) {
      continue;
    }

    Time now = Simulator::Now ();

    std::ofstream of;

    of.open("statistics/queuelogv2_" + filename, std::ios::out | std::ios::app);
    of << now.GetMilliSeconds() << "," 
    << ns3::RngSeedManager::GetSeed() << ","
    // << sourceDataRate << "," 
    << tracker->nodeId << "," 
    << tracker->interface << "," 
    << tracker->meanDatarate << "," 
    << tracker->lambda << "," 
    << tracker->mu << "," 
    << tracker->rho << "," 
    << tracker->CA_sqr << "," 
    << tracker->CS_sqr << "," 
    << tracker->meanPacketSize << "," 
    << tracker->txPackets << "," 
    << tracker->txBytes << ","
    << tracker->meanDatarateEff << ","
    << tracker->lambda_eff << ","
    << tracker->rho_eff << ","
    << tracker->dropPackets << ","
    << tracker->dropBytes << ","
    << tracker->meanDropPacketSize << ","
    << tracker->meanLatencySim << std::endl; 
    // << tracker->meanServiceTime << std::endl;
    of.close();
  }  
}


void
QueueLogV2::PeriodicUpdateWindow ()
{
  UpdateWindow ();
  Simulator::Schedule (PERIODIC_WINDOW_INTERVAL, &QueueLogV2::PeriodicUpdateWindow, this);
}

void
QueueLogV2::PeriodicDoAnalyticalModel ()
{
  // DoAnalyticalModel ();
  Simulator::Schedule (PERIODIC_ANA_INTERVAL, &QueueLogV2::PeriodicDoAnalyticalModel, this);
}


void
QueueLogV2::PeriodicPrintLog ()
{
  PrintLog ();
  Simulator::Schedule (PERIODIC_PRINT_INTERVAL, &QueueLogV2::PeriodicPrintLog, this);
}

void
QueueLogV2::NotifyConstructionCompleted ()
{
  Object::NotifyConstructionCompleted ();
}


} // namespace ns3