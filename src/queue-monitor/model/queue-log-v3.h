#ifndef QUEUE_LOG_V3_H
#define QUEUE_LOG_V3_H

#include <vector>
#include <map>
#include <unordered_map>

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/queue-probe.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class QueueLogV3 : public Object
{
public:

  // --- basic methods ---
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;
  QueueLogV3 ();
  virtual ~QueueLogV3();

  class QueueLogTracker: public Object {
  public:
      uint32_t nodeId;
      uint32_t interface;

      double CA_sqr = 0;
      double CD_sqr = 0;

      double meanDatarate_enqueue = 0;
      double lambda_enqueue = 0;
      double meanInterarrival_enqueue = 0;
      double meanPacketSize_enqueue = 0;
      double meanServiceTime_enqueue = 0;
      double mu = 0;
      double rho = 0;
      uint32_t txPackets_enqueue = 0;
      int64_t txBytes_enqueue = 0;

      double meanDatarate_dequeue = 0;
      double lambda_dequeue = 0;
      double meanInterarrival_dequeue = 0;
      double meanPacketSize_dequeue = 0;
      double meanServiceTime_dequeue = 0;
      uint32_t txPackets_dequeue = 0;
      int64_t txBytes_dequeue = 0;

      double meanDropPacketSize = 0;
      // double meanDatarateEff = 0;
      // double lambda_eff = 0;
      // double rho_eff = 0;
      uint32_t dropPackets = 0;
      int64_t dropBytes = 0;
      std::vector<int64_t> vectorPacketSizeDrop;


      std::vector<int64_t> vectorTime_enqueue;
      std::vector<int64_t> vectorPacketSizeTx_enqueue;
      std::vector<double> vectorInterarrival_enqueue;

      std::vector<int64_t> vectorTime_dequeue;
      std::vector<int64_t> vectorPacketSizeTx_dequeue;
      std::vector<double> vectorInterarrival_dequeue;

      int64_t startTime_sim = 0;
    //   int64_t startTime_ana = 0;

  };


  void CreateQueueLog(uint32_t nodeID, uint32_t interface);
  void UpdateQueueLogEnqueue(uint32_t nodeID, uint32_t interface, uint32_t packetSize);
  void UpdateQueueLogDequeue(uint32_t nodeID, uint32_t interface, uint32_t packetSize);
  void UpdateQueueDrop(uint32_t nodeID, uint32_t interface, uint32_t packetSize);

  void UpdateWindow ();
//   void DoAnalyticalModel ();
  void PrintLog ();

  void PeriodicUpdateWindow ();
  void PeriodicDoAnalyticalModel ();
  void PeriodicPrintLog ();

  std::string sourceDataRate;
  std::string filename;

  Time m_TimeWindow = MicroSeconds(10);
  // key: <nodeID, interfaceID>
  std::map<std::pair<uint32_t, uint32_t>, Ptr<QueueLogTracker>> m_queue_logs;
  
protected:
  virtual void NotifyConstructionCompleted ();


};


} // namespace ns3

#endif /* QUEUE_LOG_V3_H */