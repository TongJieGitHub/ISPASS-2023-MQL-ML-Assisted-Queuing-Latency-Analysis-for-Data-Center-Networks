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

#include "mimic-log-v2.h"
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

#define PERIODIC_SAMPLING_INTERVAL (MicroSeconds (2000))
// #define PERIODIC_WINDOW_INTERVAL (MicroSeconds   (9999999))
// #define PERIODIC_PRINT_INTERVAL (MicroSeconds    (10000000))

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MimicLogV2");

NS_OBJECT_ENSURE_REGISTERED (MimicLogV2);

TypeId 
MimicLogV2::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MimicLogV2")
    .SetParent<Object> ()
    .SetGroupName ("MimicLogV2")
    .AddConstructor<MimicLogV2> ()
    .AddAttribute ("TimeWindow", 
                   "TimeWindow",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&MimicLogV2::m_TimeWindow),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId 
MimicLogV2::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

MimicLogV2::MimicLogV2() 
{
  Simulator::Schedule (PERIODIC_SAMPLING_INTERVAL, &MimicLogV2::PeriodicSampling, this);
  // Simulator::Schedule (m_TimeWindow - MicroSeconds(1), &MimicLogV2::PeriodicUpdateWindow, this);
  // Simulator::Schedule (m_TimeWindow, &MimicLogV2::PeriodicPrintLog, this);
}

MimicLogV2::~MimicLogV2() 
{
    
}

void
MimicLogV2::Schedule() 
{
  Simulator::Schedule (m_TimeWindow + m_TimeWarmUp - MicroSeconds(1), &MimicLogV2::PeriodicUpdateWindow, this);
  Simulator::Schedule (m_TimeWindow + m_TimeWarmUp, &MimicLogV2::PeriodicPrintLog, this);
}

void
MimicLogV2::CreateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint16_t srcPort, std::unordered_map<uint32_t, int> address_client_map, std::unordered_map<uint32_t, int> address_server_map)
{
  // Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  // int client = address_client_map[sourceAddress.Get()];
  // Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  // int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(flowId);
  if(search == m_logs.end()) {
    Ptr<MimicLogTracker> tracker = ns3::Create<MimicLogTracker>();
    tracker->flowId = flowId;
    tracker->nodeId = probe->GetNode()->GetId();
    tracker->probe = probe;
    tracker->srcAddress = srcAddress;
    tracker->desAddress = desAddress;
    tracker->srcPort = srcPort;
    m_logs.insert(std::make_pair(flowId, tracker));
    this->address_client_map = address_client_map;
    this->address_server_map = address_server_map;
  }
}

void
MimicLogV2::UpdateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint16_t srcPort, uint32_t packetId, uint32_t packetSize, int64_t latency)
{
  // Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  // int client = address_client_map[sourceAddress.Get()];
  // Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  // int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(flowId);
  if(search != m_logs.end()) {
    auto tracker = search->second;
    Time now = Simulator::Now ();
    // std::ofstream of;
    // tracker->vectorPacketSizeTx.push_back(packetSize + 2);
    // tracker->vectorLatency.push_back(latency);
    tracker->sumPacketSizeTx += (packetSize + 2);
    tracker->sumPacketSizeTxSqr += (packetSize + 2) * (packetSize + 2);
    tracker->countArrival += 1;
    if (tracker->countArrival > 1) {
      tracker->sumInterarrival += now.GetNanoSeconds() - tracker->lastTimeArrival;
      tracker->sumInterarrivalSqr += (now.GetNanoSeconds() - tracker->lastTimeArrival) * (now.GetNanoSeconds() - tracker->lastTimeArrival);
    }
    tracker->lastTimeArrival = now.GetNanoSeconds();
    // Ptr<Ipv4FlowProbe> probe_ipv4 = probe->GetObject<Ipv4FlowProbe>();
    // if (probe_ipv4->m_ipv4->m_protocols.find(std::make_pair(6, -1)) != probe_ipv4->m_ipv4->m_protocols.end())
    // {
    //   Ptr<TcpL4Protocol> tcpl4 = probe_ipv4->m_ipv4->m_protocols[std::make_pair(6, -1)]->GetObject<TcpL4Protocol>();
    //   for(auto socket: tcpl4->m_sockets) {
    //     Address addr;
    //     socket->GetSockName(addr);
    //     InetSocketAddress addrnew = InetSocketAddress::ConvertFrom(addr);
    //     // std::cout << addrnew << ", " << addrnew.GetIpv4() << ", " << addrnew.GetPort() << std::endl;
    //     if (addrnew.GetIpv4() == srcAddress && addrnew.GetPort() == srcPort) {
    //       if (now.GetNanoSeconds() % PERIODIC_PRINT_INTERVAL.GetNanoSeconds() >= PERIODIC_PRINT_INTERVAL.GetNanoSeconds() / 100 * tracker->vectorCwnd.size()) {
    //         uint32_t cwnd = socket->m_tcb->m_cWnd.Get();
    //         double rtt = socket->m_tcb->m_lastRtt.Get().GetNanoSeconds() / 1e6;
    //         tracker->vectorCwnd.push_back(cwnd);
    //         tracker->vectorRtt.push_back(rtt);
    //       }
    //     }
    //   }
    // }
  }
}

void 
MimicLogV2::UpdateLatency (Ptr<FlowProbe> probe,uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency)
{
  // Ipv4Address sourceAddress = Ipv4Address::ConvertFrom(srcAddress);
  // int client = address_client_map[sourceAddress.Get()];
  // Ipv4Address destinationAddress = Ipv4Address::ConvertFrom(desAddress); 
  // int server = address_server_map[destinationAddress.Get()];

  auto search = m_logs.find(flowId);
  if(search != m_logs.end()) {
    auto tracker = search->second;
      tracker->sum_latency += static_cast<double>(latency) / 1e6; // ns->ms
      tracker->count_latency ++;
      tracker->sum_rxBytes += (packetSize + 2);
      tracker->sum_rxPackets ++;
  }
}

void
MimicLogV2::Sampling()
{
  for(auto log : m_logs) {
    auto tracker = log.second;
    Time now = Simulator::Now ();

    Ptr<Ipv4FlowProbe> probe_ipv4 = tracker->probe->GetObject<Ipv4FlowProbe>();
    if (probe_ipv4->m_ipv4->m_protocols.find(std::make_pair(6, -1)) != probe_ipv4->m_ipv4->m_protocols.end())
    {
      Ptr<TcpL4Protocol> tcpl4 = probe_ipv4->m_ipv4->m_protocols[std::make_pair(6, -1)]->GetObject<TcpL4Protocol>();
      for(auto socket: tcpl4->m_sockets) {
        Address addr;
        socket->GetSockName(addr);
        InetSocketAddress addrnew = InetSocketAddress::ConvertFrom(addr);
        // std::cout << addrnew << ", " << addrnew.GetIpv4() << ", " << addrnew.GetPort() << std::endl;
        if (addrnew.GetIpv4() == tracker->srcAddress && addrnew.GetPort() == tracker->srcPort) {
          uint32_t cwnd = socket->m_tcb->m_cWnd.Get();
          double rtt = socket->m_tcb->m_lastRtt.Get().GetNanoSeconds() / 1e6;
          tracker->sumCwnd += static_cast<double>(cwnd);
          tracker->sumRtt += rtt;
          tracker->countSampling += 1;
        }
      }
    }
  }
}

void
MimicLogV2::UpdateWindow()
{
  for(auto log : m_logs) {
    auto tracker = log.second;
    Time now = Simulator::Now ();

    // compute arrival datarate (Mbps)
    double meanDatarate = tracker->sumPacketSizeTx * 8 * 1000 / (m_TimeWindow.GetNanoSeconds());
    tracker->meanDatarate = meanDatarate;

    // compute coefficient of variation of interarrival time 
    int64_t n = tracker->countArrival - 1;
    double mean = tracker->sumInterarrival / n;
    double var = (tracker->sumInterarrivalSqr - tracker->sumInterarrival * tracker->sumInterarrival / n) / (n - 1);
    double CA_sqr = var / (mean * mean);
    tracker->meanInterarrival = mean;
    tracker->CA_sqr = CA_sqr;

   // compute coefficient of variation of service time
    n = tracker->countArrival;
    mean = tracker->sumPacketSizeTx / n;
    var = (tracker->sumPacketSizeTxSqr - tracker->sumPacketSizeTx * tracker->sumPacketSizeTx / n) / (n - 1);
    double CS_sqr= var / (mean * mean);
    tracker->CS_sqr = CS_sqr;

    tracker->txPackets = tracker->countArrival;
    tracker->txBytes = tracker->sumPacketSizeTx;
    tracker->meanPacketSize = tracker->sumPacketSizeTx / tracker->countArrival;
    tracker->meanServiceTime = tracker->meanPacketSize * 8 * 10 / 1e6;
    
    tracker->meanLatencySim = tracker->sum_latency / tracker->count_latency;
    tracker->rxPackets = tracker->sum_rxPackets;
    tracker->rxBytes = tracker->sum_rxBytes;
    
    tracker->meanCwnd = tracker->sumCwnd / tracker->countSampling;
    tracker->meanRtt = tracker->sumRtt / tracker->countSampling;

    tracker->lastTimeArrival = 0;
    tracker->sumInterarrival = 0;
    tracker->sumInterarrivalSqr = 0;
    tracker->countArrival = 0;
    tracker->sumPacketSizeTx = 0;
    tracker->sumPacketSizeTxSqr = 0;

    tracker->sum_rxPackets = 0;
    tracker->sum_rxBytes = 0;
    tracker->sum_latency = 0;
    tracker->count_latency = 0;

    tracker->sumCwnd = 0;
    tracker->sumRtt = 0;
    tracker->countSampling = 0;
  }
}

void
MimicLogV2::PrintLog()
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
    stats_filename.append("end_to_end_latency.csv");
    of.open(stats_filename, std::ios::out | std::ios::app);
    of << now.GetMilliSeconds()/1000*1000 << "," 
    << ns3::RngSeedManager::GetSeed() << "," 
    << tracker->flowId << ","
    << client << ","
    << server << ","
    << s_str << ","
    << d_str << ","
    << tracker->meanDatarate << "," 
    << tracker->CA_sqr << "," 
    << tracker->CS_sqr << "," 
    << tracker->meanPacketSize << "," 
    << tracker->txPackets << "," 
    << tracker->txBytes << ","
    << tracker->rxPackets << ","  
    << tracker->rxBytes << "," 
    << tracker->meanCwnd << "," 
    << tracker->meanRtt << "," 
    << tracker->meanLatencySim << std::endl;
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
MimicLogV2::PeriodicSampling ()
{
  Sampling ();
  if (Simulator::Now () <= Seconds(100000))
    Simulator::Schedule (PERIODIC_SAMPLING_INTERVAL, &MimicLogV2::PeriodicSampling, this);
}

void
MimicLogV2::PeriodicUpdateWindow ()
{
  UpdateWindow ();
  Simulator::Schedule (m_TimeWindow - MicroSeconds(1), &MimicLogV2::PeriodicUpdateWindow, this);
}

void
MimicLogV2::PeriodicPrintLog ()
{
  PrintLog ();
  Simulator::Schedule (m_TimeWindow, &MimicLogV2::PeriodicPrintLog, this);
}


void
MimicLogV2::NotifyConstructionCompleted ()
{
  Object::NotifyConstructionCompleted ();
}


} // namespace ns3
