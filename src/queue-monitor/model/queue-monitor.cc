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

#include "queue-monitor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include <fstream>
#include <sstream>
#include "ns3/node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QueueMonitor");

NS_OBJECT_ENSURE_REGISTERED (QueueMonitor);

TypeId 
QueueMonitor::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QueueMonitor")
    .SetParent<Object> ()
    .SetGroupName ("QueueMonitor")
    .AddConstructor<QueueMonitor> ()
    .AddAttribute ("StartTime", ("The time when the monitoring starts."),
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&QueueMonitor::Start),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId 
QueueMonitor::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

QueueMonitor::QueueMonitor ()
  : m_enabled (false)
{
  NS_LOG_FUNCTION (this);
}

void
QueueMonitor::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_startEvent);
  Simulator::Cancel (m_stopEvent);
  for (uint32_t i = 0; i < m_queueProbes.size (); i++)
    {
      m_queueProbes[i]->Dispose ();
      m_queueProbes[i] = 0;
    }
  Object::DoDispose ();
}

void
QueueMonitor::AddProbe (Ptr<QueueProbe> probe)
{
  m_queueProbes.push_back (probe);
}


const QueueMonitor::QueueProbeContainer&
QueueMonitor::GetAllProbes () const
{
  return m_queueProbes;
}


void 
QueueMonitor::ReportTxEnqueue (Ptr<QueueProbe> probe, uint32_t packetSize)
{
  // std::cout << "TxEnqueue: " 
  // << probe->GetNode()->GetId() << "," 
  // << probe->GetDevice()->GetIfIndex() << "," 
  // << packetSize << std::endl;
  if(enable_log) 
  {
    uint32_t nodeId = probe->GetNode()->GetId();
    uint32_t interface = probe->GetDevice()->GetIfIndex();
    m_queuelogv3.CreateQueueLog(nodeId, interface);
    m_queuelogv3.UpdateQueueLogEnqueue(nodeId, interface, packetSize);
    m_queuelogv3.sourceDataRate = this->sourceDataRate;
    m_queuelogv3.filename = this->filename;
    m_queuelogv3.m_TimeWindow = this->m_TimeWindow;
  }
}

void 
QueueMonitor::ReportTxDequeue (Ptr<QueueProbe> probe, uint32_t packetSize)
{
  // std::cout << "TxDequeue: " 
  // << probe->GetNode()->GetId() << "," 
  // << probe->GetDevice()->GetIfIndex() << "," 
  // << packetSize << std::endl;
  if(enable_log) 
  {
    uint32_t nodeId = probe->GetNode()->GetId();
    uint32_t interface = probe->GetDevice()->GetIfIndex();
    m_queuelogv3.CreateQueueLog(nodeId, interface);
    m_queuelogv3.UpdateQueueLogDequeue(nodeId, interface, packetSize);
    m_queuelogv3.sourceDataRate = this->sourceDataRate;
    m_queuelogv3.filename = this->filename;
    m_queuelogv3.m_TimeWindow = this->m_TimeWindow;
  }
}

void 
QueueMonitor::ReportTxDrop (Ptr<QueueProbe> probe, uint32_t packetSize)
{
  // std::cout << "TxDrop: " 
  // << probe->GetNode()->GetId() << "," 
  // << probe->GetDevice()->GetIfIndex() << "," 
  // << packetSize << std::endl;
  if(enable_log) 
  {
    uint32_t nodeId = probe->GetNode()->GetId();
    uint32_t interface = probe->GetDevice()->GetIfIndex();
    m_queuelogv3.CreateQueueLog(nodeId, interface);
    m_queuelogv3.UpdateQueueDrop(nodeId, interface, packetSize);
    m_queuelogv3.sourceDataRate = this->sourceDataRate;
    m_queuelogv3.filename = this->filename;
    m_queuelogv3.m_TimeWindow = this->m_TimeWindow;
  }
}

void
QueueMonitor::Start (const Time &time)
{
  NS_LOG_FUNCTION (this << time.As (Time::S));
  if (m_enabled)
    {
      NS_LOG_DEBUG ("QueueMonitor already enabled; returning");
      return;
    }
  Simulator::Cancel (m_startEvent);
  NS_LOG_DEBUG ("Scheduling start at " << time.As (Time::S));
  m_startEvent = Simulator::Schedule (time, &QueueMonitor::StartRightNow, this);
}

void
QueueMonitor::Stop (const Time &time)
{
  NS_LOG_FUNCTION (this << time.As (Time::S));
  Simulator::Cancel (m_stopEvent);
  NS_LOG_DEBUG ("Scheduling stop at " << time.As (Time::S));
  m_stopEvent = Simulator::Schedule (time, &QueueMonitor::StopRightNow, this);
}


void
QueueMonitor::StartRightNow ()
{
  NS_LOG_FUNCTION (this);
  if (m_enabled)
    {
      NS_LOG_DEBUG ("QueueMonitor already enabled; returning");
      return;
    }
  m_enabled = true;
}


void
QueueMonitor::StopRightNow ()
{
  NS_LOG_FUNCTION (this);
  if (!m_enabled)
    {
      NS_LOG_DEBUG ("QueueMonitor not enabled; returning");
      return;
    }
  m_enabled = false;
}



} // namespace ns3
