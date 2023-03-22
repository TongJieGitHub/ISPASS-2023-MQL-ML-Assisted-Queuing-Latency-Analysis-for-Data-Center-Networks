#ifndef QUEUE_MONITOR_H
#define QUEUE_MONITOR_H

#include <vector>
#include <map>

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/histogram.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include "ns3/traced-callback.h"
#include "ns3/queue-probe.h"
#include "ns3/queue-log-v3.h"

namespace ns3 {

/**
 * \defgroup queue-monitor Queue Monitor
 * \brief  Collect and store performance data from a simulation
 */

/**
 * \ingroup queue-monitor
 * \brief An object that monitors and reports back packet queues observed during a simulation
 *
 * The QueueMonitor class is responsible for coordinating efforts
 * regarding probes, and collects queue statistics.
 *
 */
class QueueMonitor : public Object
{
public:

  // --- basic methods ---
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;
  QueueMonitor ();


  /// Set the time, counting from the current time, from which to start monitoring flows.
  /// This method overwrites any previous calls to Start()
  /// \param time delta time to start
  void Start (const Time &time);
  /// Set the time, counting from the current time, from which to stop monitoring flows.
  /// This method overwrites any previous calls to Stop()
  /// \param time delta time to stop
  void Stop (const Time &time);
  /// Begin monitoring flows *right now*
  void StartRightNow ();
  /// End monitoring flows *right now*
  void StopRightNow ();

  void ReportTxEnqueue (Ptr<QueueProbe> probe, uint32_t packetSize);

  void ReportTxDequeue (Ptr<QueueProbe> probe, uint32_t packetSize);

  void ReportTxDrop (Ptr<QueueProbe> probe, uint32_t packetSize);

  // --- methods to be used by the QueueMonitorProbe's only ---
  /// Register a new QueueProbe that will begin monitoring and report
  /// events to this monitor.  This method is normally only used by
  /// QueueProbe implementations.
  /// \param probe the probe to add
  void AddProbe (Ptr<QueueProbe> probe);


  // --- methods to get the results ---

  /// Container: QueueProbe
  typedef std::vector< Ptr<QueueProbe> > QueueProbeContainer;
  /// Container Iterator: QueueProbe
  typedef std::vector< Ptr<QueueProbe> >::iterator QueueProbeContainerI;
  /// Container Const Iterator: QueueProbe
  typedef std::vector< Ptr<QueueProbe> >::const_iterator QueueProbeContainerCI;


  /// Get a list of all QueueProbe's associated with this QueueMonitor
  /// \returns a list of all the probes
  const QueueProbeContainer& GetAllProbes () const;

  std::string sourceDataRate;
  std::string filename;
  bool enable_log = false;
  bool enable_mimic_log = false;
  QueueLogV3 m_queuelogv3;
  Time m_TimeWindow = MicroSeconds(100000);
protected:

//   virtual void NotifyConstructionCompleted ();
  virtual void DoDispose (void);

private:

  QueueProbeContainer m_queueProbes; //!< all the FlowProbes

  EventId m_startEvent;     //!< Start event
  EventId m_stopEvent;      //!< Stop event
  bool m_enabled;           //!< QueueMon is enabled

  std::vector<int64_t> vectorTime;
  std::vector<int64_t> vectorPacketSize;
  std::vector<double> vectorInterarrival;
  size_t windowSize = 1000;
  int64_t startTime = 0;


  TracedCallback<Time> m_tracePacketDelay;

};


} // namespace ns3

#endif /* QUEUE_MONITOR_H */