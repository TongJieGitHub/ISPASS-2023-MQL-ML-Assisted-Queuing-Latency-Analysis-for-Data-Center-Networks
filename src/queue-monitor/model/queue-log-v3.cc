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

#include "queue-log-v3.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include <fstream>
#include <sstream>
#include "ns3/node.h"
#include "ns3/rng-seed-manager.h"

// #define PERIODIC_WINDOW_INTERVAL (MicroSeconds (9999999))
// #define PERIODIC_ANA_INTERVAL (MicroSeconds    (9999999))
// #define PERIODIC_PRINT_INTERVAL (MicroSeconds  (10000000))

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QueueLogV3");

NS_OBJECT_ENSURE_REGISTERED (QueueLogV3);

TypeId 
QueueLogV3::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QueueLogV3")
    .SetParent<Object> ()
    .SetGroupName ("QueueLogV3")
    .AddConstructor<QueueLogV3> ()
    .AddAttribute ("TimeWindow", 
                   "TimeWindow",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&QueueLogV3::m_TimeWindow),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId 
QueueLogV3::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

QueueLogV3::QueueLogV3() 
{
  Simulator::Schedule (m_TimeWindow - MicroSeconds(1), &QueueLogV3::PeriodicUpdateWindow, this);
  Simulator::Schedule (m_TimeWindow - MicroSeconds(1), &QueueLogV3::PeriodicDoAnalyticalModel, this);
  Simulator::Schedule (m_TimeWindow, &QueueLogV3::PeriodicPrintLog, this);
}

QueueLogV3::~QueueLogV3() 
{
    
}


void
QueueLogV3::CreateQueueLog(uint32_t nodeID, uint32_t interface) 
{
  auto search = m_queue_logs.find(std::make_pair(nodeID, interface));
  if(search == m_queue_logs.end()) {
    Ptr<QueueLogTracker> tracker = ns3::Create<QueueLogTracker>();
    tracker->nodeId = nodeID;
    tracker->interface = interface;
    m_queue_logs.insert(std::make_pair(std::make_pair(nodeID, interface), tracker));
  }
}


void 
QueueLogV3::UpdateQueueLogEnqueue(uint32_t nodeID, uint32_t interface, uint32_t packetSize)
{
  auto search = m_queue_logs.find(std::make_pair(nodeID, interface));
  if(search != m_queue_logs.end()) {
    auto tracker = search->second;
    Time now = Simulator::Now ();
    // std::ofstream of;
    tracker->vectorTime_enqueue.push_back(now.GetNanoSeconds());
    tracker->vectorPacketSizeTx_enqueue.push_back(packetSize);
    // tracker->vectorLatency.push_back(latency);
    size_t sz = tracker->vectorTime_enqueue.size();
    if (sz > 1) {
      tracker->vectorInterarrival_enqueue.push_back(static_cast<double>(tracker->vectorTime_enqueue.at(sz - 1) - tracker->vectorTime_enqueue.at(sz - 2)));
    }
  }
}


void 
QueueLogV3::UpdateQueueLogDequeue(uint32_t nodeID, uint32_t interface, uint32_t packetSize)
{
  auto search = m_queue_logs.find(std::make_pair(nodeID, interface));
  if(search != m_queue_logs.end()) {
    auto tracker = search->second;
    Time now = Simulator::Now ();
    // std::ofstream of;
    tracker->vectorTime_dequeue.push_back(now.GetNanoSeconds());
    tracker->vectorPacketSizeTx_dequeue.push_back(packetSize);
    // tracker->vectorLatency.push_back(latency);
    size_t sz = tracker->vectorTime_dequeue.size();
    if (sz > 1) {
      tracker->vectorInterarrival_dequeue.push_back(static_cast<double>(tracker->vectorTime_dequeue.at(sz - 1) - tracker->vectorTime_dequeue.at(sz - 2)));
    }
  }
}



void 
QueueLogV3::UpdateQueueDrop(uint32_t nodeID, uint32_t interface, uint32_t packetSize)
{
  auto search = m_queue_logs.find(std::make_pair(nodeID, interface));
  if(search != m_queue_logs.end()) {
    auto tracker = search->second;
    tracker->vectorPacketSizeDrop.push_back(packetSize);
  }
}


void
QueueLogV3::UpdateWindow()
{

  for(auto log : m_queue_logs) {
    auto tracker = log.second;
    Time now = Simulator::Now ();

    // double meanDatarate = static_cast<double>(std::accumulate(tracker->vectorPacketSizeTx.begin(), tracker->vectorPacketSizeTx.end(), 0) - tracker->vectorPacketSizeTx.back()) * 8 * 1000 / (tracker->vectorTime.back() - tracker->vectorTime.front());
    double meanDatarate_enqueue = static_cast<double>(std::accumulate(tracker->vectorPacketSizeTx_enqueue.begin(), tracker->vectorPacketSizeTx_enqueue.end(), static_cast<int64_t>(0))) * 8 * 1000 / (m_TimeWindow.GetNanoSeconds());
    tracker->meanDatarate_enqueue = meanDatarate_enqueue;

    double meanDatarate_dequeue = static_cast<double>(std::accumulate(tracker->vectorPacketSizeTx_dequeue.begin(), tracker->vectorPacketSizeTx_dequeue.end(), static_cast<int64_t>(0))) * 8 * 1000 / (m_TimeWindow.GetNanoSeconds());
    tracker->meanDatarate_dequeue = meanDatarate_dequeue;

    double mean = std::accumulate(tracker->vectorInterarrival_enqueue.begin(), tracker->vectorInterarrival_enqueue.end(), 0.0) / tracker->vectorInterarrival_enqueue.size();
    double var = 0.0;
    for (size_t i = 0; i < tracker->vectorInterarrival_enqueue.size(); i++) {
      var += pow(tracker->vectorInterarrival_enqueue.at(i) - mean ,2);
    }
    var = var / tracker->vectorInterarrival_enqueue.size();
    double CA_sqr = var / (mean * mean);
    tracker->meanInterarrival_enqueue = mean;
    tracker->CA_sqr = CA_sqr;

    mean = std::accumulate(tracker->vectorInterarrival_dequeue.begin(), tracker->vectorInterarrival_dequeue.end(), 0.0) / tracker->vectorInterarrival_dequeue.size();
    var = 0.0;
    for (size_t i = 0; i < tracker->vectorInterarrival_dequeue.size(); i++) {
      var += pow(tracker->vectorInterarrival_dequeue.at(i) - mean ,2);
    }
    var = var / tracker->vectorInterarrival_dequeue.size();
    double CD_sqr = var / (mean * mean);
    tracker->meanInterarrival_dequeue = mean;
    tracker->CD_sqr = CD_sqr;


    // mean = std::accumulate(tracker->vectorPacketSizeTx.begin(), tracker->vectorPacketSizeTx.end(), 0.0) / tracker->vectorPacketSizeTx.size() * 8 * 10;
    // var = 0.0;
    // for (size_t i = 0; i < tracker->vectorPacketSizeTx.size(); i++) {
    //   var += pow(tracker->vectorPacketSizeTx.at(i) * 8 * 10 - mean ,2);
    // }
    // var = var / tracker->vectorPacketSizeTx.size();
    // double CS_sqr= var / (mean * mean);
    // tracker->meanServiceTime = mean;
    // tracker->CS_sqr = CS_sqr;

    int64_t sumPacketSizeTx_enqueue = std::accumulate(tracker->vectorPacketSizeTx_enqueue.begin(), tracker->vectorPacketSizeTx_enqueue.end(), static_cast<int64_t>(0));
    double meanPacketSizeTx_enqueue = static_cast<double>(sumPacketSizeTx_enqueue) / tracker->vectorPacketSizeTx_enqueue.size();
    tracker->txPackets_enqueue = tracker->vectorPacketSizeTx_enqueue.size();
    tracker->txBytes_enqueue = sumPacketSizeTx_enqueue;
    tracker->meanPacketSize_enqueue = meanPacketSizeTx_enqueue;

    int64_t sumPacketSizeTx_dequeue = std::accumulate(tracker->vectorPacketSizeTx_dequeue.begin(), tracker->vectorPacketSizeTx_dequeue.end(), static_cast<int64_t>(0));
    double meanPacketSizeTx_dequeue = static_cast<double>(sumPacketSizeTx_dequeue) / tracker->vectorPacketSizeTx_dequeue.size();
    tracker->txPackets_dequeue = tracker->vectorPacketSizeTx_dequeue.size();
    tracker->txBytes_dequeue = sumPacketSizeTx_dequeue;
    tracker->meanPacketSize_dequeue = meanPacketSizeTx_dequeue;
    
    
    tracker->lambda_enqueue = meanDatarate_enqueue * 1000000 / (meanPacketSizeTx_enqueue * 8);
    tracker->mu = 100 * 1000000 / (meanPacketSizeTx_enqueue * 8);
    tracker->rho = tracker->lambda_enqueue / tracker->mu;

    tracker->lambda_dequeue = meanDatarate_dequeue * 1000000 / (meanPacketSizeTx_dequeue * 8);

    int64_t sumPacketSizeDrop = std::accumulate(tracker->vectorPacketSizeDrop.begin(), tracker->vectorPacketSizeDrop.end(), static_cast<int64_t>(0));
    double meanPacketSizeDrop = tracker->vectorPacketSizeDrop.size() == 0 ? 0 : static_cast<double>(sumPacketSizeDrop) / tracker->vectorPacketSizeDrop.size();
    // double meanDatarateEff =  static_cast<double>(sumPacketSizeTx - sumPacketSizeDrop) * 8 * 1000 / (PERIODIC_WINDOW_INTERVAL.GetNanoSeconds());
    // double meanPacketSizeTxEff;
    // if (tracker->vectorPacketSizeTx.size() == tracker->vectorPacketSizeDrop.size())
    // {
    //   meanPacketSizeTxEff = 0;
    // } else
    // {
    //   meanPacketSizeTxEff = (sumPacketSizeTx - sumPacketSizeDrop) / (tracker->vectorPacketSizeTx.size() - tracker->vectorPacketSizeDrop.size());
    // }

    // tracker->meanDatarateEff = meanDatarateEff;
    tracker->dropPackets = tracker->vectorPacketSizeDrop.size();
    tracker->dropBytes = sumPacketSizeDrop;
    tracker->meanDropPacketSize = meanPacketSizeDrop;
    // tracker->lambda_eff = meanDatarateEff * 1000000 / (meanPacketSizeTxEff * 8);
    // tracker->rho_eff = tracker->meanDatarateEff / 100;

    tracker->vectorTime_enqueue.clear();
    tracker->vectorPacketSizeTx_enqueue.clear();
    tracker->vectorInterarrival_enqueue.clear();

    tracker->vectorTime_dequeue.clear();
    tracker->vectorPacketSizeTx_dequeue.clear();
    tracker->vectorInterarrival_dequeue.clear();

    tracker->vectorPacketSizeDrop.clear();
    tracker->startTime_sim = now.GetNanoSeconds();
  }
}


void
QueueLogV3::PrintLog()
{
  for(auto log : m_queue_logs) {
    auto tracker = log.second;

    if (tracker->txPackets_enqueue == 0) {
      continue;
    }

    Time now = Simulator::Now ();

    std::ofstream of;

    of.open("statistics/queuelogv3_" + filename, std::ios::out | std::ios::app);
    of << now.GetMilliSeconds() << "," 
    << ns3::RngSeedManager::GetSeed() << ","
    // << sourceDataRate << "," 
    << tracker->nodeId << "," 
    << tracker->interface << "," 
    << tracker->meanDatarate_enqueue << "," 
    << tracker->lambda_enqueue << "," 
    << tracker->mu << "," 
    << tracker->rho << "," 
    << tracker->CA_sqr << "," 
    << tracker->meanPacketSize_enqueue << "," 
    << tracker->txPackets_enqueue << "," 
    << tracker->txBytes_enqueue << ","
    << tracker->meanDatarate_dequeue << "," 
    << tracker->lambda_dequeue << "," 
    << tracker->CD_sqr << "," 
    << tracker->meanPacketSize_dequeue << "," 
    << tracker->txPackets_dequeue << "," 
    << tracker->txBytes_dequeue << ","
    << tracker->meanDropPacketSize << ","
    << tracker->dropPackets << ","
    << tracker->dropBytes << std::endl;
    // << tracker->meanServiceTime << std::endl;
    of.close();
  }  
}


void
QueueLogV3::PeriodicUpdateWindow ()
{
  UpdateWindow ();
  Simulator::Schedule (m_TimeWindow - MicroSeconds(1), &QueueLogV3::PeriodicUpdateWindow, this);
}

void
QueueLogV3::PeriodicDoAnalyticalModel ()
{
  // DoAnalyticalModel ();
  Simulator::Schedule (m_TimeWindow - MicroSeconds(1), &QueueLogV3::PeriodicDoAnalyticalModel, this);
}


void
QueueLogV3::PeriodicPrintLog ()
{
  // PrintLog ();
  Simulator::Schedule (m_TimeWindow, &QueueLogV3::PeriodicPrintLog, this);
}

void
QueueLogV3::NotifyConstructionCompleted ()
{
  Object::NotifyConstructionCompleted ();
}


} // namespace ns3