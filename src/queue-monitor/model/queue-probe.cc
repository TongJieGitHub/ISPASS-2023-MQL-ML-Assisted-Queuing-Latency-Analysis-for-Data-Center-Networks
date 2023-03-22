#include "ns3/queue-probe.h"
#include "ns3/queue-monitor.h"

namespace ns3 {

/* static */
TypeId QueueProbe::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QueueProbe")
    .SetParent<Object> ()
    .SetGroupName ("QueueMonitor")
    // No AddConstructor because this class has no default constructor.
    ;

  return tid;
}

QueueProbe::~QueueProbe ()
{
}


QueueProbe::QueueProbe (Ptr<QueueMonitor> queueMonitor)
  : m_queueMonitor (queueMonitor)
{
  m_queueMonitor->AddProbe (this);
}

QueueProbe::QueueProbe (Ptr<QueueMonitor> monitor,
                        Ptr<Node> node,
                        Ptr<PointToPointNetDevice> device)
  : QueueProbe (monitor)
{
  m_node = node;
  m_device = device;

  if (!m_device->TraceConnectWithoutContext ("MacTxEnqueue",
                                           MakeCallback (&QueueProbe::TxEnqueueLogger, Ptr<QueueProbe> (this))))
    {
      NS_FATAL_ERROR ("trace fail");
    }
  if (!m_device->TraceConnectWithoutContext ("MacTxDequeue",
                                           MakeCallback (&QueueProbe::TxDequeueLogger, Ptr<QueueProbe> (this))))
    {
      NS_FATAL_ERROR ("trace fail");
    }
  if (!m_device->TraceConnectWithoutContext ("MacTxDrop",
                                           MakeCallback (&QueueProbe::TxDropLogger, Ptr<QueueProbe> (this))))
    {
      NS_FATAL_ERROR ("trace fail");
    }        

}

void 
QueueProbe::TxEnqueueLogger (Ptr<const Packet> packet)
{
  m_queueMonitor->ReportTxEnqueue(this, packet->GetSize());
}

void 
QueueProbe::TxDequeueLogger (Ptr<const Packet> packet)
{
  m_queueMonitor->ReportTxDequeue(this, packet->GetSize());
}

void 
QueueProbe::TxDropLogger (Ptr<const Packet> packet)
{
  m_queueMonitor->ReportTxDrop(this, packet->GetSize());
}

void
QueueProbe::DoDispose (void)
{
  m_queueMonitor = 0;
  Object::DoDispose ();
}


Ptr<Node> 
QueueProbe::GetNode() const
{
  return m_node;
}

Ptr<PointToPointNetDevice> 
QueueProbe::GetDevice() const
{
  return m_device;
}

} // namespace ns3
