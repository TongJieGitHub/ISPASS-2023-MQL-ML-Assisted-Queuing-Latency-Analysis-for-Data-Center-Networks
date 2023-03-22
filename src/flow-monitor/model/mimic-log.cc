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

#include "mimic-log.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include <fstream>
#include <sstream>
#include "ns3/node.h"
#include "ns3/rng-seed-manager.h"

#define PERIODIC_WINDOW_INTERVAL (MicroSeconds (199999))
#define PERIODIC_PRINT_INTERVAL (MicroSeconds (200000))

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MimicLog");

NS_OBJECT_ENSURE_REGISTERED (MimicLog);

TypeId 
MimicLog::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MimicLog")
    .SetParent<Object> ()
    .SetGroupName ("MimicLog")
    .AddConstructor<MimicLog> ()
    .AddAttribute ("TimeWindow", 
                   "TimeWindow",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&MimicLog::m_TimeWindow),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId 
MimicLog::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

MimicLog::MimicLog() 
{
  Simulator::Schedule (PERIODIC_WINDOW_INTERVAL, &MimicLog::PeriodicUpdateWindow, this);
  Simulator::Schedule (PERIODIC_PRINT_INTERVAL, &MimicLog::PeriodicPrintLog, this);
}

MimicLog::~MimicLog() 
{
    
}

void
MimicLog::CreateLog (Ptr<FlowProbe> probe, Address srcAddress, Address desAddress, std::unordered_map<uint32_t, int> address_client_map, std::unordered_map<uint32_t, int> address_server_map)
{
  Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  int client = address_client_map[sourceAddress.Get()];
  Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(std::make_pair(client, server));
  if(search == m_logs.end()) {
    Ptr<MimicLogTracker> tracker = ns3::Create<MimicLogTracker>();
    tracker->nodeId = probe->GetNode()->GetId();
    tracker->srcAddress = srcAddress;
    tracker->desAddress = desAddress;
    // tracker->isLastStage = false;
    m_logs.insert(std::make_pair(std::make_pair(client, server), tracker));
    this->address_client_map = address_client_map;
    this->address_server_map = address_server_map;
  }
}

void
MimicLog::UpdateLog (Ptr<FlowProbe> probe, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency)
{
  Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  int client = address_client_map[sourceAddress.Get()];
  Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(std::make_pair(client, server));
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
  }
}

void 
MimicLog::UpdateLatency (Ptr<FlowProbe> probe, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency)
{
  Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  int client = address_client_map[sourceAddress.Get()];
  Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(std::make_pair(client, server));
  if(search != m_logs.end()) {
    auto tracker = search->second;
    tracker->vectorPacketSizeRx.push_back(packetSize + 2);
    tracker->vectorLatency.push_back(latency);
  }
}

void
MimicLog::UpdateWindow()
{
  for(auto log : m_logs) {
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
    double latency_total = 0;
    for (auto m : tracker->vectorLatency) {
      latency_total += static_cast<double>(m) / 1e6;
    }
    if(tracker->vectorLatency.size() != 0) {
      double meanLatency = latency_total / tracker->vectorLatency.size() * 1e6;
      tracker->meanLatencySim = meanLatency;

      int64_t sumPacketSizeRx = std::accumulate(tracker->vectorPacketSizeRx.begin(), tracker->vectorPacketSizeRx.end(), 0);
      tracker->rxPackets = tracker->vectorPacketSizeRx.size();
      tracker->rxBytes = sumPacketSizeRx;
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
MimicLog::PrintLog()
{
  for(auto log : m_logs) {
    auto tracker = log.second;
    // skip 
    if (tracker->txPackets == 0 || tracker->txPackets == 1) {
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

    std::string stats_filename;
    stats_filename.append("runs/");
    stats_filename.append(this->tag);
    stats_filename.append("/reports_sim/");
    stats_filename.append(filename);
    of.open(stats_filename, std::ios::out | std::ios::app);
    of << now.GetMilliSeconds() << "," 
    << ns3::RngSeedManager::GetSeed() << "," 
    << client << ","
    << server << ","
    << s_str << ","
    << d_str << ","
    << tracker->stage << "," 
    << tracker->nodeId << "," 
    << tracker->meanDatarate << "," 
    << tracker->CA_sqr << "," 
    << tracker->CS_sqr << "," 
    << tracker->meanPacketSize << "," 
    << tracker->txPackets << "," 
    << tracker->txBytes << ","
    << tracker->rxPackets << ","  
    << tracker->rxBytes << "," 
    << tracker->meanLatencySim << "," 
    << tracker->meanServiceTime << std::endl;
    of.close();

    // of.open("statistics/mimic_" + std::to_string(now.GetMilliSeconds()) + ".txt", std::ios::out | std::ios::app);
    // of << "c" << client << ":"
    // << "s" << server << ":"
    // << "d" << tracker->meanDatarate << ":"
    // << "p" << tracker->meanPacketSize << ":"
    // << "a" << tracker->CA_sqr << ":"
    // << "b" << tracker->CS_sqr << std::endl;
  }  
}

void
MimicLog::PeriodicUpdateWindow ()
{
  UpdateWindow ();
  Simulator::Schedule (PERIODIC_WINDOW_INTERVAL, &MimicLog::PeriodicUpdateWindow, this);
}

void
MimicLog::PeriodicPrintLog ()
{
  PrintLog ();
  Simulator::Schedule (PERIODIC_PRINT_INTERVAL, &MimicLog::PeriodicPrintLog, this);
}


void
MimicLog::NotifyConstructionCompleted ()
{
  Object::NotifyConstructionCompleted ();
}


} // namespace ns3
