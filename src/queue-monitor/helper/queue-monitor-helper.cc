#include "queue-monitor-helper.h"

#include "ns3/queue-monitor.h"
#include "ns3/queue-probe.h"
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/net-device.h"
#include "ns3/point-to-point-net-device.h"

namespace ns3 {

QueueMonitorHelper::QueueMonitorHelper ()
{
  m_monitorFactory.SetTypeId ("ns3::QueueMonitor");
}

QueueMonitorHelper::~QueueMonitorHelper ()
{
  if (m_queueMonitor)
    {
      m_queueMonitor->Dispose ();
      m_queueMonitor = 0;
    }
}

void 
QueueMonitorHelper::SetMonitorAttribute (std::string n1, const AttributeValue &v1)
{
  m_monitorFactory.Set (n1, v1);
}


Ptr<QueueMonitor>
QueueMonitorHelper::GetMonitor ()
{
  if (!m_queueMonitor)
    {
      m_queueMonitor = m_monitorFactory.Create<QueueMonitor> ();
    }
  return m_queueMonitor;
}



Ptr<QueueMonitor>
QueueMonitorHelper::Install (Ptr<Node> node)
{
  Ptr<QueueMonitor> monitor = GetMonitor ();
  for (uint32_t i = 0; i < node->GetNDevices(); i++)
  {
    Ptr<PointToPointNetDevice> device = DynamicCast<PointToPointNetDevice>(node->GetDevice(i));
    if (device)
    {
      Ptr<QueueProbe> probe = Create<QueueProbe> (monitor, node, device);
    }
  }
  return m_queueMonitor;
}


Ptr<QueueMonitor>
QueueMonitorHelper::Install (NodeContainer nodes)
{
  for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); ++i)
    {
      Ptr<Node> node = *i;
      if (node->GetNDevices() > 0)
        {
          Install (node);
        }
    }
  return m_queueMonitor;
}

Ptr<QueueMonitor>
QueueMonitorHelper::InstallAll ()
{
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      Ptr<Node> node = *i;
      if (node->GetNDevices() > 0)
        {
          Install (node);
        }
    }
  return m_queueMonitor;
}


} // namespace ns3
