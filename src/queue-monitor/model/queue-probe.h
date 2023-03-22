#ifndef QUEUE_PROBE_H
#define QUEUE_PROBE_H

#include <map>
#include <vector>

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/point-to-point-net-device.h"

namespace ns3 {

class QueueMonitor;

/// The QueueProbe class is responsible for listening for packet events
/// in a specific point of the simulated space, report those events to
/// the global FlowMonitor, and collect its own flow statistics
/// regarding only the packets that pass through that probe.
class QueueProbe : public Object
{
private:
  /// Defined and not implemented to avoid misuse
  //   QueueProbe (QueueProbe const &);
  /// Defined and not implemented to avoid misuse
  /// \returns
  QueueProbe& operator= (QueueProbe const &);

protected:
  virtual void DoDispose (void);

public:
  /// Constructor
  /// \param flowMonitor the FlowMonitor this probe is associated with
  QueueProbe (Ptr<QueueMonitor> queueMonitor);
  QueueProbe (Ptr<QueueMonitor> monitor, Ptr<Node> node, Ptr<PointToPointNetDevice> device);
  virtual ~QueueProbe ();

  /// Register this type.
  /// \return The TypeId.
  static TypeId GetTypeId (void);

  virtual Ptr<Node> GetNode (void) const;
  virtual Ptr<PointToPointNetDevice> GetDevice (void) const;

protected:
  Ptr<QueueMonitor> m_queueMonitor; //!< the QueueMonitor instance

private:
  void TxEnqueueLogger (Ptr<const Packet> packet);
  void TxDequeueLogger (Ptr<const Packet> packet);
  void TxDropLogger (Ptr<const Packet> packet);

  Ptr<Node> m_node;
  Ptr<PointToPointNetDevice> m_device;
};


} // namespace ns3

#endif /* QUEUE_PROBE_H */