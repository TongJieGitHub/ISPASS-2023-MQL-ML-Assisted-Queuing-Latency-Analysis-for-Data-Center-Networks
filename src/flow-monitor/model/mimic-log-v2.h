#ifndef MIMIC_LOG_V2_H
#define MIMIC_LOG_V2_H

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

namespace ns3 {

class MimicLogV2 : public Object
{
public:

  // --- basic methods ---
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;
  MimicLogV2 ();
  virtual ~MimicLogV2();
  std::string tag;
  void Schedule();

  class MimicLogTracker: public Object {
  public:
      uint32_t flowId;
      uint32_t stage;
      uint32_t nodeId;
      Ptr<FlowProbe> probe;
      Address srcAddress;
      Address desAddress;
      uint16_t srcPort;

      double CA_sqr = 1;
      double CS_sqr = 0;
      double meanDatarate = 0;     // source data rate, Mbps
      double meanLatencySim = 0;   // end-to-end latency, ms
      double meanInterarrival = 0; // ms
      double meanPacketSize = 0;   // bytes
      double meanServiceTime = 0;  // ms

      uint32_t txPackets = 0;
      uint32_t rxPackets = 0;
      int64_t txBytes = 0;
      int64_t rxBytes = 0;
      
      double meanCwnd = 0; // TCP congestion window
      double meanRtt = 0;  // TCP round trip time

      int64_t lastTimeArrival = 0;
      double sumInterarrival = 0;
      double sumInterarrivalSqr = 0;
      int64_t countArrival = 0;
      double sumPacketSizeTx = 0;
      double sumPacketSizeTxSqr = 0;

      int64_t sum_rxPackets = 0;
      int64_t sum_rxBytes = 0;
      double sum_latency = 0;
      int64_t count_latency = 0;

      double sumCwnd = 0;
      double sumRtt = 0;
      int64_t countSampling = 0;
  };

  void CreateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint16_t srcPort, std::unordered_map<uint32_t, int> address_client_map, std::unordered_map<uint32_t, int> address_server_map);
  void UpdateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint16_t srcPort, uint32_t packetId, uint32_t packetSize, int64_t latency);
  void UpdateLatency (Ptr<FlowProbe> probe, uint32_t interface, uint32_t flowId, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency);

  void Sampling ();
  void UpdateWindow ();
  void PrintLog ();

  void PeriodicSampling ();
  void PeriodicUpdateWindow ();
  void PeriodicPrintLog ();

  std::string filename;
  std::unordered_map<uint32_t, int> address_client_map;
  std::unordered_map<uint32_t, int> address_server_map;
  Time m_TimeWindow = MicroSeconds(3);
  Time m_TimeWarmUp = MicroSeconds(1);

protected:
  virtual void NotifyConstructionCompleted ();

private:
  // key: flowId
  std::map<uint32_t, Ptr<MimicLogTracker>> m_logs;

};


} // namespace ns3

#endif /* MIMIC_LOG_V2_H */
