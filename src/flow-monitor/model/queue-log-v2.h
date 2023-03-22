#ifndef QUEUE_LOG_V2_H
#define QUEUE_LOG_V2_H

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

class QueueLogV2 : public Object
{
public:

  // --- basic methods ---
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;
  QueueLogV2 ();
  virtual ~QueueLogV2();

  class QueueLogTracker: public Object {
  public:
      uint32_t nodeId;
      uint32_t interface;

      double CA_sqr = 1;
      double CS_sqr = 0;
      double meanDatarate = 0;
      double lambda = 0;
      double mu = 0;
      double meanLatencySim = 0;
      double meanInterarrival = 0;
      double meanPacketSize = 0;
      double meanServiceTime = 0;
      double rho = 0;

      uint32_t num_flows = 0;

      double ME_lat_ana_inf_GEG1 = 0;
      double ME_lat_ana_inf_GEG1_sys = 0;

      uint32_t txPackets = 0;
      uint32_t rxPackets = 0;
      int64_t txBytes = 0;
      int64_t rxBytes = 0;

      double meanDropPacketSize = 0;
      double meanDatarateEff = 0;
      double lambda_eff = 0;
      double rho_eff = 0;
      uint32_t dropPackets = 0;
      int64_t dropBytes = 0;
      std::vector<int64_t> vectorPacketSizeDrop;

      std::vector<std::pair<int64_t, double>> InjectionLog;
      std::vector<std::pair<int64_t, double>> CaLog;
      std::vector<int64_t> vectorTime;
      std::vector<int64_t> vectorPacketSizeTx;
      std::vector<int64_t> vectorPacketSizeRx;
      std::vector<int64_t> vectorLatency;
      std::vector<double> vectorInterarrival;
      int64_t startTime_sim = 0;
    //   int64_t startTime_ana = 0;

  };

  //void CreateFlowPerQueueLog (Ptr<FlowProbe> probe, uint32_t interface, Ptr<FlowProbe> probeFrom, uint32_t interfaceFrom);
  //void UpdateFlowPerQueueLog (Ptr<FlowProbe> probe, uint32_t interface, Ptr<FlowProbe> probeFrom, uint32_t interfaceFrom, uint32_t packetSize);
  //void UpdateFlowPerQueueLatency (Ptr<FlowProbe> probe, uint32_t interface, Ptr<FlowProbe> probFrom, uint32_t interfaceFrom, uint32_t packetSize, int64_t latency);
  void CreateQueueLog(Ptr<FlowProbe> probe, uint32_t interface);
  void UpdateQueueLog(Ptr<FlowProbe> probe, uint32_t interface, uint32_t packetSize, int64_t latency);
  void UpdateQueueLatency(Ptr<FlowProbe> probe, uint32_t interface, uint32_t packetSize, int64_t latency);
  void UpdateQueueDrop(Ptr<FlowProbe> probe, uint32_t interface, uint32_t packetSize);

  void UpdateWindow ();
//   void DoAnalyticalModel ();
  void PrintLog ();

  void PeriodicUpdateWindow ();
  void PeriodicDoAnalyticalModel ();
  void PeriodicPrintLog ();

  std::string sourceDataRate;
  std::string filename;
protected:
  virtual void NotifyConstructionCompleted ();

private:
  Time m_TimeWindow = MilliSeconds (1000); 
  // key: <nodeID, interfaceID>
  std::map<std::pair<uint32_t, uint32_t>, Ptr<QueueLogTracker>> m_queue_logs;

};


} // namespace ns3

#endif /* QUEUE_LOG_V2_H */