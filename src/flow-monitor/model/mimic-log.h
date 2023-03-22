#ifndef MIMIC_LOG_H
#define MIMIC_LOG_H

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

class MimicLog : public Object
{
public:

  // --- basic methods ---
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;
  MimicLog ();
  virtual ~MimicLog();
  std::string tag;

  class MimicLogTracker: public Object {
  public:
      uint32_t flowId;
      uint32_t stage;
      uint32_t nodeId;
      Address srcAddress;
      Address desAddress;

      double CA_sqr = 1;
      double CS_sqr = 0;
      double meanDatarate = 0;
      double meanLatencySim = 0;
      double meanInterarrival = 0;
      double meanPacketSize = 0;
      double meanServiceTime = 0;

      uint32_t txPackets = 0;
      uint32_t rxPackets = 0;
      int64_t txBytes = 0;
      int64_t rxBytes = 0;
      
      std::vector<std::pair<int64_t, double>> InjectionLog;
      std::vector<std::pair<int64_t, double>> CaLog;
      std::vector<int64_t> vectorTime;
      std::vector<int64_t> vectorPacketSizeTx;
      std::vector<int64_t> vectorPacketSizeRx;
      std::vector<int64_t> vectorLatency;
      std::vector<double> vectorInterarrival;
      int64_t startTime_sim = 0;
      int64_t startTime_ana = 0;
  };

  void CreateLog (Ptr<FlowProbe> probe, Address srcAddress, Address desAddress, std::unordered_map<uint32_t, int> address_client_map, std::unordered_map<uint32_t, int> address_server_map);
  void UpdateLog (Ptr<FlowProbe> probe, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency);
  void UpdateLatency (Ptr<FlowProbe> probe, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency);

  void UpdateWindow ();
  void PrintLog ();

  void PeriodicUpdateWindow ();
  void PeriodicPrintLog ();

  std::string filename;
  std::unordered_map<uint32_t, int> address_client_map;
  std::unordered_map<uint32_t, int> address_server_map;

protected:
  virtual void NotifyConstructionCompleted ();

private:
  Time m_TimeWindow = MilliSeconds (1000); 
  // key: <client, server>
  std::map<std::pair<int, int>, Ptr<MimicLogTracker>> m_logs;

};


} // namespace ns3

#endif /* MIMIC_LOG_H */
