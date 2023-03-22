#ifndef QUEUE_MONITOR_HELPER_H
#define QUEUE_MONITOR_HELPER_H

#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/queue-monitor.h"
#include <string>

namespace ns3 {

class AttributeValue;


/**
 * \ingroup queue-monitor
 * \brief Helper to enable IP queue monitoring on a set of Nodes
 */
class QueueMonitorHelper
{
public:

  QueueMonitorHelper ();
  ~QueueMonitorHelper ();

  /**
   * \brief Set an attribute for the to-be-created QueueMonitor object
   * \param n1 attribute name
   * \param v1 attribute value
   */
  void SetMonitorAttribute (std::string n1, const AttributeValue &v1);

  /**
   * \brief Enable queue monitoring on a set of nodes
   * \param nodes A NodeContainer holding the set of nodes to work with.
   * \returns a pointer to the QueueMonitor object
   */
  Ptr<QueueMonitor> Install (NodeContainer nodes);
  /**
   * \brief Enable queue monitoring on a single node
   * \param node A Ptr<Node> to the node on which to enable queue monitoring.
   * \returns a pointer to the QueueMonitor object
   */
  Ptr<QueueMonitor> Install (Ptr<Node> node);
  /**
   * \brief Enable queue monitoring on all nodes
   * \returns a pointer to the QueueMonitor object
   */
  Ptr<QueueMonitor> InstallAll ();

  /**
   * \brief Retrieve the QueueMonitor object created by the Install* methods
   * \returns a pointer to the QueueMonitor object
   */
  Ptr<QueueMonitor> GetMonitor ();

private:
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  QueueMonitorHelper (const QueueMonitorHelper&);
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   * \returns
   */
  QueueMonitorHelper& operator= (const QueueMonitorHelper&);

  ObjectFactory m_monitorFactory;        //!< Object factory
  Ptr<QueueMonitor> m_queueMonitor;        //!< the QueueMonitor object
};

} // namespace ns3


#endif /* QUEUE_MONITOR_HELPER_H */
