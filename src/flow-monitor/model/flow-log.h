#ifndef FLOW_LOG_H
#define FLOW_LOG_H

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

class FlowLog : public Object
{
public:

  // --- basic methods ---
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;
  FlowLog ();
  std::string tag;
  virtual ~FlowLog();

  class FlowLogTracker: public Object {
  public:
      uint32_t flowId;
      uint32_t stage;
      uint32_t nodeId;
      uint32_t interface;
      Address srcAddress;
      Address desAddress;
      bool isLastStage = false;

      double CA_sqr = 1;
      double CS_sqr = 0;
      double meanDatarate = 0;
      double meanDatarate_total = 0;
      double meanLatencySim = 0;
      double meanInterarrival = 0;
      double meanPacketSize = 0;
      double meanServiceTime = 0;
      double rho = 0;
      double rho_total = 0;
      uint32_t num_flows = 0;

      double ME_lat_ana_inf_GEG1 = 0;
      double ME_lat_ana_inf_GED1 = 0;

      // std::vector<double> vectorErrorDiff;
      // std::vector<double> vectorErrorDiffAbs;
      // std::vector<double> vectorErrorPct;
      // std::vector<double> vectorErrorPctAbs;
      // std::vector<double> vectorErrorRatio;
      // double minErrorDiff;
      // double minErrorPct;
      // size_t bestModel = 0;
      // size_t bestModelRatio = 0;

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

  void CreateLog (Ptr<FlowProbe> probe, uint32_t interface, uint32_t stage, Address srcAddress, Address desAddress, std::unordered_map<uint32_t, int> address_client_map, std::unordered_map<uint32_t, int> address_server_map);
  void UpdateLog (Ptr<FlowProbe> probe, uint32_t interface, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency, bool isLastStage);
  void UpdateLatency (Ptr<FlowProbe> probe, uint32_t interface, Address srcAddress, Address desAddress, uint32_t packetId, uint32_t packetSize, int64_t latency);

  void UpdateWindow ();
  void DoAnalyticalModel ();
  void PrintLog ();

  void PeriodicUpdateWindow ();
  void PeriodicDoAnalyticalModel ();
  void PeriodicPrintLog ();

  std::string sourceDataRate;
  std::string filename;
  Ptr<QueueMonitor> m_queue_monitor;

  std::unordered_map<uint32_t, int> address_client_map;
  std::unordered_map<uint32_t, int> address_server_map;

protected:
  virtual void NotifyConstructionCompleted ();

private:
  Time m_TimeWindow = MilliSeconds (1000); 
  // key: <<client, server>, <nodeID, interfaceID>>
  std::map<std::pair<std::pair<int, int>, std::pair<uint32_t, uint32_t>>, Ptr<FlowLogTracker>> m_logs;

};


} // namespace ns3

#endif /* FLOW_LOG_H */
