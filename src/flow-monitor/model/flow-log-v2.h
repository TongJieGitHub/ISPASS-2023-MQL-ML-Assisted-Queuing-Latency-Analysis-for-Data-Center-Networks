#ifndef FLOW_LOG_V2_H
#define FLOW_LOG_V2_H

#include <vector>
#include <map>
#include <unordered_map>

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/flow-probe.h"
#include "ns3/flow-classifier.h"
#include "ns3/histogram.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include "ns3/traced-callback.h"
#include "ns3/queue-monitor.h"

namespace ns3 {

class FlowLogV2 : public Object
{
public:

  // --- basic methods ---
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;
  FlowLogV2 ();
  std::string tag;
  virtual ~FlowLogV2();
  void Schedule();

  class FlowLogTracker: public Object {
  public:
      uint32_t flowId;
      uint32_t stage;
      uint32_t nodeId;
      Ptr<FlowProbe> probe;
      uint32_t interface;
      Address srcAddress;
      Address desAddress;
      bool isLastStage = false;

      double CA_sqr = 1;
      double CD_sqr = 1;
      double CS_sqr = 0;
      double meanDatarateArrival = 0;         // Mbps
      double meanDatarateDeparture = 0;       // Mbps
      double meanDatarateArrival_total = 0;   // Mbps
      double meanDatarateDeparture_total = 0; // Mbps
      double meanLatencySim = 0;              // ms
      double meanInterarrival = 0;            // ms
      double meanPacketSize = 0;              // bytes
      double meanServiceTime = 0;             // ms
      double rho = 0;
      double rho_total = 0;
      uint32_t num_flows = 0;

      uint32_t txPackets = 0;
      uint32_t rxPackets = 0;
      int64_t txBytes = 0;
      int64_t rxBytes = 0;

      // queue occupancy
      double meanQueuePackets = 0;
      double meanQueueBytes = 0;


      int64_t lastTimeArrival = 0;
      double sumInterarrival = 0;
      double sumInterarrivalSqr = 0;
      int64_t countArrival = 0;
      double sumPacketSizeArrival = 0;
      double sumPacketSizeArrivalSqr = 0;

      int64_t lastTimeDeparture = 0;
      double sumInterDeparture = 0;
      double sumInterDepartureSqr = 0;
      int64_t countDeparture = 0;
      double sumPacketSizeDeparture = 0;
      double sumPacketSizeDepartureSqr = 0;
      double sumProductLatencySize = 0;

      double sum_latency = 0;
      int64_t count_latency = 0;
      // int64_t sum_txPackets = 0;
      // int64_t sum_txBytes = 0;
      int64_t sum_rxPackets = 0;
      int64_t sum_rxBytes = 0;

  };

  void CreateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t stage, uint32_t flowId, Address srcAddress, Address desAddress, std::unordered_map<uint32_t, int> address_client_map, std::unordered_map<uint32_t, int> address_server_map);
  void UpdateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency, bool isLastStage);
  void UpdateLatency (Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency);
  int GetCount(Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId);

  void Sampling ();
  void UpdateWindow ();
  void DoAnalyticalModel ();
  void PrintLog ();

  void PeriodicSampling ();
  void PeriodicUpdateWindow ();
  void PeriodicDoAnalyticalModel ();
  void PeriodicPrintLog ();

  std::string sourceDataRate;
  std::string filename;
  Ptr<QueueMonitor> m_queue_monitor;
  Time m_TimeWindow = MicroSeconds(3);
  Time m_TimeWarmUp = MicroSeconds(1);

  std::unordered_map<uint32_t, int> address_client_map;
  std::unordered_map<uint32_t, int> address_server_map;

protected:
  virtual void NotifyConstructionCompleted ();

private:
  // key: <<flowId>, <nodeID, interfaceID>>
  std::map<std::pair<uint32_t, std::pair<uint32_t, uint32_t>>, Ptr<FlowLogTracker>> m_logs;

};


} // namespace ns3

#endif /* FLOW_LOG_V2_H */
