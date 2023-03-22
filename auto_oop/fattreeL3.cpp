#include "util.h"
#include "fattree.h"
#include "fattreeL3.h"

using namespace std;

// This function generates a Fat Tree for a given number of pods (K)
void FattreeL3::create(int K, double linkBandwidth, double datarate_unit)
{
	// es:remove
	cout << linkBandwidth << endl;
    No_of_ports = K;

    total_no_of_core = (No_of_ports / 2) * (No_of_ports / 2);       // number of cores in the fattree
    total_no_of_aggr = No_of_ports * No_of_ports / 2;               // number of aggr switch in the fattree
    total_no_of_edge = No_of_ports * No_of_ports / 2;               // number of edge switch in the fattree
    total_no_of_host = No_of_ports * No_of_ports * No_of_ports / 4; // number of nodes in the fattree

    No_of_core_per_group = No_of_ports / 2;
    No_of_aggr_per_pod = No_of_ports / 2;
    No_of_edge_per_pod = No_of_ports / 2;
    No_of_host_per_pod = (No_of_ports / 2) * (No_of_ports / 2);

    No_aggr_per_core = No_of_ports;     // the number of aggr switch each core is connected to
    No_edge_per_aggr = No_of_ports / 2; // the number of edge switch each aggr is connected to
    No_host_per_edge = No_of_ports / 2; // the number of nodes each edge is connected to

    total_no_of_devices = total_no_of_core + total_no_of_aggr + total_no_of_edge + total_no_of_host;
    total_no_of_queues = (total_no_of_core + total_no_of_aggr + total_no_of_edge) * No_of_ports + total_no_of_host;

    No_of_core_groups = No_of_ports / 2;
    No_of_pods = No_of_ports;

    int global_deviceId = 0;
    int global_queueId = 0;
    // create and initialize core switches
    int local_deviceID = 0;
    for (int i = 0; i < No_of_core_groups; i++)
    {
        coreGroup.push_back(vector<int>());
        for (int j = 0; j < No_of_core_per_group; j++)
        {
            Device coreSwitch;
            coreSwitch.globalDeviceId = global_deviceId++;
            coreSwitch.deviceName = "core_" + std::to_string(local_deviceID++);
            coreSwitch.podId = i;
            coreSwitch.localDeviceId = j;
            coreSwitch.numQueues = No_of_ports;
            coreSwitch.type = Device::core;

            for (int k = 0; k < coreSwitch.numQueues; k++)
            {
                Queue queue;
				queue.globalDeviceId = coreSwitch.globalDeviceId;
                queue.globalQueueId = global_queueId++;
                queue.deviceName = coreSwitch.deviceName;
                queue.localQueueId = k;
                queue.podId = coreSwitch.podId;
                queue.localDeviceId = coreSwitch.localDeviceId;

				queue.linkBandwidth = linkBandwidth;
				queue.datarate_unit = datarate_unit;

                coreSwitch.vectorQueueId.push_back(queue.globalQueueId);
                globalQueues.push_back(queue);
            }

            globalDevices.push_back(coreSwitch);

            coreGroup.at(i).push_back(coreSwitch.globalDeviceId);
        }
    }

    // create and initialize aggr switches
    local_deviceID = 0;
    for (int i = 0; i < No_of_pods; i++)
    {
        aggrPod.push_back(vector<int>());
        for (int j = 0; j < No_of_aggr_per_pod; j++)
        {
            Device aggrSwitch;
            aggrSwitch.globalDeviceId = global_deviceId++;
            aggrSwitch.deviceName = "aggr_" + std::to_string(local_deviceID++);
            aggrSwitch.podId = i;
            aggrSwitch.localDeviceId = j;
            aggrSwitch.numQueues = No_of_ports;
            aggrSwitch.type = Device::aggr;

            for (int k = 0; k < aggrSwitch.numQueues; k++)
            {
                Queue queue;
                queue.globalDeviceId = aggrSwitch.globalDeviceId;
                queue.globalQueueId = global_queueId++;
                queue.deviceName = aggrSwitch.deviceName;
                queue.localQueueId = k;
                queue.podId = aggrSwitch.podId;
                queue.localDeviceId = aggrSwitch.localDeviceId;

				queue.linkBandwidth = linkBandwidth;
				queue.datarate_unit = datarate_unit;

                aggrSwitch.vectorQueueId.push_back(queue.globalQueueId);
                globalQueues.push_back(queue);
            }

            globalDevices.push_back(aggrSwitch);

            aggrPod.at(i).push_back(aggrSwitch.globalDeviceId);
        }
    }

    // create and initialize edge switches
    local_deviceID = 0;
    for (int i = 0; i < No_of_pods; i++)
    {
        edgePod.push_back(vector<int>());
        for (int j = 0; j < No_of_edge_per_pod; j++)
        {
            Device edgeSwitch;
            edgeSwitch.globalDeviceId = global_deviceId++;
            edgeSwitch.deviceName = "edge_" + std::to_string(local_deviceID++);
            edgeSwitch.podId = i;
            edgeSwitch.localDeviceId = j;
            edgeSwitch.numQueues = No_of_ports;
            edgeSwitch.type = Device::edge;

            for (int k = 0; k < edgeSwitch.numQueues; k++)
            {
                Queue queue;
                queue.globalDeviceId = edgeSwitch.globalDeviceId;
                queue.globalQueueId = global_queueId++;
                queue.deviceName = edgeSwitch.deviceName;
                queue.localQueueId = k;
                queue.podId = edgeSwitch.podId;
                queue.localDeviceId = edgeSwitch.localDeviceId;

				queue.linkBandwidth = linkBandwidth;
				queue.datarate_unit = datarate_unit;

                edgeSwitch.vectorQueueId.push_back(queue.globalQueueId);
                globalQueues.push_back(queue);
            }

            globalDevices.push_back(edgeSwitch);

            edgePod.at(i).push_back(edgeSwitch.globalDeviceId);
        }
    }

    // create and initialize hosts
    local_deviceID = 0;
    for (int i = 0; i < No_of_pods; i++)
    {
        hostPod.push_back(vector<int>());
        for (int j = 0; j < No_of_host_per_pod; j++)
        {
            Device host;
            host.globalDeviceId = global_deviceId++;
            host.deviceName = "host_" + std::to_string(local_deviceID++);
            host.podId = i;
            host.localDeviceId = j;
            host.numQueues = 1;
            host.type = Device::host;

            for (int k = 0; k < host.numQueues; k++)
            {
                Queue queue;
                queue.globalDeviceId = host.globalDeviceId;
                queue.globalQueueId = global_queueId++;
                queue.deviceName = host.deviceName;
                queue.localQueueId = k;
                queue.podId = host.podId;
                queue.localDeviceId = host.localDeviceId;

				queue.linkBandwidth = linkBandwidth;
				queue.datarate_unit = datarate_unit;

                host.vectorQueueId.push_back(queue.globalQueueId);
                globalQueues.push_back(queue);
            }

            globalDevices.push_back(host);

            hostPod.at(i).push_back(host.globalDeviceId);
        }
    }

#if DEBUG == 1
    cout << "total_no_of_devices: " << total_no_of_devices << ", global_deviceId: " << global_deviceId << ", globalDevices vecotr size: " << globalDevices.size() << endl;
#endif
}

void FattreeL3::link()
{
    //  core to aggr
    for (auto const &group : coreGroup)
    {
        for (auto const &deviceId : group)
        {
            auto &coreSwitch = globalDevices.at(deviceId);
            for (auto const &queueId : coreSwitch.vectorQueueId)
            {
                auto &queue = globalQueues.at(queueId);
                queue.nextdeviceId = globalDevices.at(aggrPod.at(queue.localQueueId).at(coreSwitch.podId)).globalDeviceId;
                queue.nextDeviceName = globalDevices.at(aggrPod.at(queue.localQueueId).at(coreSwitch.podId)).deviceName;
                queue.queueName = queue.deviceName + "_queue_" + std::to_string(queue.localQueueId) + "_down";
                queue.type = Queue::core_down;
            }
        }
    }

    //  aggr to core
    for (auto const &pod : aggrPod)
    {
        for (auto const &deviceId : pod)
        {
            auto &aggrSwitch = globalDevices.at(deviceId);
            for (auto const &queueId : aggrSwitch.vectorQueueId)
            {
                auto &queue = globalQueues.at(queueId);
                if (queue.localQueueId < aggrSwitch.vectorQueueId.size() / 2)
                {
                    queue.nextdeviceId = globalDevices.at(coreGroup.at(aggrSwitch.localDeviceId).at(queue.localQueueId)).globalDeviceId;
                    queue.nextDeviceName = globalDevices.at(coreGroup.at(aggrSwitch.localDeviceId).at(queue.localQueueId)).deviceName;
                    queue.queueName = queue.deviceName + "_queue_" + std::to_string(queue.localQueueId) + "_up";
                    queue.type = Queue::aggr_up;
                }
            }
        }
    }

    //  aggr to edge
    for (auto const &pod : aggrPod)
    {
        for (auto const &deviceId : pod)
        {
            auto &aggrSwitch = globalDevices.at(deviceId);
            for (auto const &queueId : aggrSwitch.vectorQueueId)
            {
                auto &queue = globalQueues.at(queueId);
                if (queue.localQueueId >= aggrSwitch.vectorQueueId.size() / 2)
                {
                    queue.nextdeviceId = globalDevices.at(edgePod.at(aggrSwitch.podId).at(queue.localQueueId % (aggrSwitch.vectorQueueId.size() / 2))).globalDeviceId;
                    queue.nextDeviceName = globalDevices.at(edgePod.at(aggrSwitch.podId).at(queue.localQueueId % (aggrSwitch.vectorQueueId.size() / 2))).deviceName;
                    queue.queueName = queue.deviceName + "_queue_" + std::to_string(queue.localQueueId) + "_down";
                    queue.type = Queue::aggr_down;
                }
            }
        }
    }

    //  edge to aggr
    for (auto const &pod : edgePod)
    {
        for (auto const &deviceId : pod)
        {
            auto &edgeSwitch = globalDevices.at(deviceId);
            for (auto const &queueId : edgeSwitch.vectorQueueId)
            {
                auto &queue = globalQueues.at(queueId);
                if (queue.localQueueId < edgeSwitch.vectorQueueId.size() / 2)
                {
                    queue.nextdeviceId = globalDevices.at(aggrPod.at(edgeSwitch.podId).at(queue.localQueueId)).globalDeviceId;
                    queue.nextDeviceName = globalDevices.at(aggrPod.at(edgeSwitch.podId).at(queue.localQueueId)).deviceName;
                    queue.queueName = queue.deviceName + "_queue_" + std::to_string(queue.localQueueId) + "_up";
                    queue.type = Queue::edge_up;
                }
            }
        }
    }

    //  edge to host
    for (auto const &pod : edgePod)
    {
        for (auto const &deviceId : pod)
        {
            auto &edgeSwitch = globalDevices.at(deviceId);
            for (auto const &queueId : edgeSwitch.vectorQueueId)
            {
                auto &queue = globalQueues.at(queueId);
                if (queue.localQueueId >= edgeSwitch.vectorQueueId.size() / 2)
                {
                    queue.nextdeviceId = globalDevices.at(hostPod.at(edgeSwitch.podId).at(queue.localQueueId % (edgeSwitch.vectorQueueId.size() / 2) + edgeSwitch.localDeviceId * No_of_edge_per_pod)).globalDeviceId;
                    queue.nextDeviceName = globalDevices.at(hostPod.at(edgeSwitch.podId).at(queue.localQueueId % (edgeSwitch.vectorQueueId.size() / 2) + edgeSwitch.localDeviceId * No_of_edge_per_pod)).deviceName;
                    queue.queueName = queue.deviceName + "_queue_" + std::to_string(queue.localQueueId) + "_down";
                    queue.type = Queue::edge_down;
                }
            }
        }
    }

    //  host to edge
    for (auto const &pod : hostPod)
    {
        for (auto const &deviceId : pod)
        {
            auto &host = globalDevices.at(deviceId);
            for (auto const &queueId : host.vectorQueueId)
            {
                auto &queue = globalQueues.at(queueId);
                queue.nextdeviceId = globalDevices.at(edgePod.at(host.podId).at(host.localDeviceId / No_host_per_edge)).globalDeviceId;
                queue.nextDeviceName = globalDevices.at(edgePod.at(host.podId).at(host.localDeviceId / No_host_per_edge)).deviceName;
                queue.queueName = queue.deviceName + "_queue_" + std::to_string(queue.localQueueId) + "_up";
                queue.type = Queue::host_up;
            }
        }
    }
}

// TODO: EXPLAIN WHAT THIS FUNCTION DOES / WHEN IT IS CALLED
void FattreeL3::route()
{
    int count_flow = 0;
    int count_flow_split = 0;
    for (auto const &pod_client : hostPod)
    {
        for (auto const &deviceId_client : pod_client)
        {
            auto &host_client = globalDevices.at(deviceId_client);
            auto &queue_client = globalQueues.at(host_client.vectorQueueId.at(0));
            for (auto const &pod_server : hostPod)
            {
                for (auto const &deviceId_server : pod_server)
                {
                    auto &host_server = globalDevices.at(deviceId_server);
                    auto &queue_server = globalQueues.at(host_server.vectorQueueId.at(0));
                    if (queue_client.globalDeviceId != queue_server.globalDeviceId)
                    {
                        Flow current_flow;
                        int clientId = queue_client.podId * No_of_host_per_pod + queue_client.localDeviceId;
                        int serverId = queue_server.podId * No_of_host_per_pod + queue_server.localDeviceId;
                        current_flow.clientId = clientId;
                        current_flow.serverId = serverId;
                        current_flow.flowId = count_flow++;

                        // 2 hops
                        if (queue_client.nextdeviceId == queue_server.nextdeviceId)
                        {
                            for (auto const &edgeDownQueueId : globalDevices.at(queue_server.nextdeviceId).vectorQueueId)
                            {
                                auto &edge_down_queue = globalQueues.at(edgeDownQueueId);
                                if (edge_down_queue.nextdeviceId == host_server.globalDeviceId)
                                {
                                    FlowSplit current_flow_split;
                                    current_flow_split.clientId = clientId;
                                    current_flow_split.serverId = serverId;
                                    current_flow_split.flowId = current_flow.flowId;
                                    current_flow_split.flowSplitId = count_flow_split++;
                                    current_flow_split.stages.globalQueueId_stage_0 = queue_client.globalQueueId;
                                    current_flow_split.stages.globalQueueId_stage_5 = edge_down_queue.globalQueueId;
                                    current_flow.vectorFlowSplitId.push_back(current_flow_split.flowSplitId);
                                    globalFlowSplits.push_back(current_flow_split);
                                }
                            }
                        }
                        // 4 hops
                        else if (queue_client.podId == queue_server.podId)
                        {
                            for (auto const &edgeUpQueueId : globalDevices.at(queue_client.nextdeviceId).vectorQueueId)
                            {
                                auto &edge_up_queue = globalQueues.at(edgeUpQueueId);
                                if (globalDevices.at(edge_up_queue.nextdeviceId).type == Device::aggr)
                                {
                                    for (auto const &aggrDownQueueId : globalDevices.at(edge_up_queue.nextdeviceId).vectorQueueId)
                                    {
                                        auto &aggr_down_queue = globalQueues.at(aggrDownQueueId);
                                        if (aggr_down_queue.nextdeviceId == queue_server.nextdeviceId)
                                        {
                                            for (auto const &edgeDownQueueId : globalDevices.at(queue_server.nextdeviceId).vectorQueueId)
                                            {
                                                auto &edge_down_queue = globalQueues.at(edgeDownQueueId);
                                                if (edge_down_queue.nextdeviceId == host_server.globalDeviceId)
                                                {
                                                    FlowSplit current_flow_split;
                                                    current_flow_split.clientId = clientId;
                                                    current_flow_split.serverId = serverId;
                                                    current_flow_split.flowId = current_flow.flowId;
                                                    current_flow_split.flowSplitId = count_flow_split++;
                                                    current_flow_split.stages.globalQueueId_stage_0 = queue_client.globalQueueId;
                                                    current_flow_split.stages.globalQueueId_stage_1 = edge_up_queue.globalQueueId;
                                                    current_flow_split.stages.globalQueueId_stage_4 = aggr_down_queue.globalQueueId;
                                                    current_flow_split.stages.globalQueueId_stage_5 = edge_down_queue.globalQueueId;
                                                    current_flow.vectorFlowSplitId.push_back(current_flow_split.flowSplitId);
                                                    globalFlowSplits.push_back(current_flow_split);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        // 6 hops
                        else
                        {
                            for (auto const &edgeUpQueueId : globalDevices.at(queue_client.nextdeviceId).vectorQueueId)
                            {
                                auto &edge_up_queue = globalQueues.at(edgeUpQueueId);
                                if (globalDevices.at(edge_up_queue.nextdeviceId).type == Device::aggr)
                                {
                                    for (auto const &aggrUpQueueId : globalDevices.at(edge_up_queue.nextdeviceId).vectorQueueId)
                                    {
                                        auto &aggr_up_queue = globalQueues.at(aggrUpQueueId);
                                        if (globalDevices.at(aggr_up_queue.nextdeviceId).type == Device::core)
                                        {
                                            for (auto const &coreDownQueueId : globalDevices.at(aggr_up_queue.nextdeviceId).vectorQueueId)
                                            {
                                                auto &core_down_queue = globalQueues.at(coreDownQueueId);
                                                if (globalDevices.at(core_down_queue.nextdeviceId).podId == queue_server.podId)
                                                {
                                                    for (auto const &aggrDownQueueId : globalDevices.at(core_down_queue.nextdeviceId).vectorQueueId)
                                                    {
                                                        auto &aggr_down_queue = globalQueues.at(aggrDownQueueId);
                                                        if (aggr_down_queue.nextdeviceId == queue_server.nextdeviceId)
                                                        {
                                                            for (auto const &edgeDownQueueId : globalDevices.at(queue_server.nextdeviceId).vectorQueueId)
                                                            {
                                                                auto &edge_down_queue = globalQueues.at(edgeDownQueueId);
                                                                if (edge_down_queue.nextdeviceId == host_server.globalDeviceId)
                                                                {
                                                                    FlowSplit current_flow_split;
                                                                    current_flow_split.clientId = clientId;
                                                                    current_flow_split.serverId = serverId;
                                                                    current_flow_split.flowId = current_flow.flowId;
                                                                    current_flow_split.flowSplitId = count_flow_split++;
                                                                    current_flow_split.stages.globalQueueId_stage_0 = queue_client.globalQueueId;
                                                                    current_flow_split.stages.globalQueueId_stage_1 = edge_up_queue.globalQueueId;
                                                                    current_flow_split.stages.globalQueueId_stage_2 = aggr_up_queue.globalQueueId;
                                                                    current_flow_split.stages.globalQueueId_stage_3 = core_down_queue.globalQueueId;
                                                                    current_flow_split.stages.globalQueueId_stage_4 = aggr_down_queue.globalQueueId;
                                                                    current_flow_split.stages.globalQueueId_stage_5 = edge_down_queue.globalQueueId;
                                                                    current_flow.vectorFlowSplitId.push_back(current_flow_split.flowSplitId);
                                                                    globalFlowSplits.push_back(current_flow_split);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        globalFlows.push_back(current_flow);
                    }
                }
            }
        }
    }
}

