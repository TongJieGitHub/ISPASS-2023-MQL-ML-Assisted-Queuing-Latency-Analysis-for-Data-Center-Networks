#include "util.h"
#include "fattree.h"
#include "fattreeL2Custom.h"

using namespace std;

void FattreeL2custom::create(int K, int numNodes, double linkBandwidth, double datarate_unit)
{
	No_of_ports = K;                       // switchRadix
	total_no_of_host = numNodes;           // number of nodes in the fattree

	total_no_of_edge = total_no_of_host / ( No_of_ports / 2);  // number of edge switch in the fattree
	total_no_of_core = total_no_of_edge / 2;              // number of core switches in the fattree
	total_no_of_aggr = 0;                                 // no aggr switches in 2-tier

	No_host_per_edge = ceil(total_no_of_host/ total_no_of_edge);    // the number of nodes each edge is connected to

	if (No_of_ports > total_no_of_edge)
	{
		No_of_core_groups = 1;
	    No_of_core_per_group = total_no_of_core;               // The number of cores that connect to the same edges 
		No_of_edge_per_pod = 1;
		No_of_pods = total_no_of_edge;
		No_of_host_per_pod = No_host_per_edge;
		No_of_links_per_core_edge_pair = floor(No_of_ports/total_no_of_edge);
	}
	else
	{
		cout << "ERROR: 2-tier - not implemented when No_of_ports < total_no_of_edges \n";
	    exit(1);
	}

	// No_of_aggr_per_pod = No_of_ports / 2;
	// No_aggr_per_core = No_of_ports;     // the number of aggr switch each core is connected to
	// No_edge_per_aggr = No_of_ports / 2; the number of edge switch each aggr is connected to

	total_no_of_devices = total_no_of_core + total_no_of_edge + total_no_of_host;
	total_no_of_queues = (total_no_of_core + total_no_of_edge) * No_of_ports + total_no_of_host;


#if DEBUG == 1
	cout << endl;
	cout << "    total_no_of_host= " << total_no_of_host << endl;
	cout << "         No_of_ports= " << No_of_ports << endl;
	cout << "  No_of_host_per_pod= " << No_of_host_per_pod << endl;
    cout << endl;
	cout << "    total_no_of_edge= " << total_no_of_edge << endl;
	cout << "          No_of_pods= " << No_of_pods << endl;
	cout << "  No_of_edge_per_pod= " << No_of_edge_per_pod << endl;
	cout << "    No_host_per_edge= " << No_host_per_edge << endl;
    cout << endl;
	cout << "    total_no_of_aggr= " << total_no_of_aggr << endl;
    cout << endl;
	cout << "    total_no_of_core= " << total_no_of_core << endl;
	cout << "No_of_core_per_group= " <<    No_of_core_per_group << endl;
	cout << "   No_of_core_groups= " << No_of_core_groups << endl;
    cout << endl;
	cout << " total_no_of_devices= " << total_no_of_devices << endl;
	cout << "  total_no_of_queues= " << total_no_of_queues << endl;
	cout << endl;
#endif

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
#if DEBUG == 1 
			cout << "\tedgePod= " << i << " \tedgeSwitch.deviceName= " << edgeSwitch.deviceName << " \tlocalDeviceId= " << j << endl;
#endif 

        }
    }

#if DEBUG == 1 
	cout << "\t ====== " << endl;
#endif 

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


void FattreeL2custom::link()
{
    //  core to edge
    for (auto const &group : coreGroup)
    {
        for (auto const &deviceId : group)
        {
            auto &coreSwitch = globalDevices.at(deviceId);
			// cout << "\n\tdeviceID= " << deviceId << " \tcoreSwitch.deviceName= " << coreSwitch.deviceName << " \tNumLinksPer= " << No_of_links_per_core_edge_pair << endl;
            for (auto const &coreQueueId : coreSwitch.vectorQueueId)
            {
                auto &coreQueue = globalQueues.at(coreQueueId);
                coreQueue.nextdeviceId = globalDevices.at(edgePod.at(coreQueue.localQueueId / No_of_links_per_core_edge_pair).at(coreSwitch.podId)).globalDeviceId;
                coreQueue.nextDeviceName = globalDevices.at(edgePod.at(coreQueue.localQueueId / No_of_links_per_core_edge_pair).at(coreSwitch.podId)).deviceName;
                coreQueue.queueName = coreQueue.deviceName + "_queue_" + std::to_string(coreQueue.localQueueId) + "_down";
                coreQueue.type = Queue::core_down;

// #if DEBUG == 1 
// 				cout << "\tglobal.queueId= " << coreQueueId << "\tlocal.queueId= " << coreQueue.localQueueId << endl;
// 				cout << "\tcoreQueue.nextDeviceName= " << coreQueue.nextDeviceName << " \tcoreQueue.localQueueID= " << coreQueue.localQueueId << " \tcoreSwitch.podID= " << coreSwitch.podId << endl;
// 				cout << "\tcoreQueue.nextDeviceName= " << coreQueue.nextDeviceName << "\t podId= " << (coreQueue.localQueueId / No_of_links_per_core_edge_pair) << endl;
// #endif 
            }
        }
    }
	
    //  edge to core
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
                    queue.nextdeviceId = globalDevices.at(coreGroup.at(edgeSwitch.localDeviceId).at(queue.localQueueId/No_of_links_per_core_edge_pair)).globalDeviceId;
                    queue.nextDeviceName = globalDevices.at(coreGroup.at(edgeSwitch.localDeviceId).at(queue.localQueueId/No_of_links_per_core_edge_pair)).deviceName;
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


void FattreeL2custom::route()
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
						//                        else if (queue_client.podId == queue_server.podId)
						else
                        {
                            for (auto const &edgeUpQueueId : globalDevices.at(queue_client.nextdeviceId).vectorQueueId)
                            {
                                auto &edge_up_queue = globalQueues.at(edgeUpQueueId);
                                if (globalDevices.at(edge_up_queue.nextdeviceId).type == Device::core)
                                {
                                    for (auto const &coreDownQueueId : globalDevices.at(edge_up_queue.nextdeviceId).vectorQueueId)
                                    {
                                        auto &core_down_queue = globalQueues.at(coreDownQueueId);
                                        if (core_down_queue.nextdeviceId == queue_server.nextdeviceId)
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
													//current_flow_split.stages.globalQueueId_stage_4 = core_down_queue.globalQueueId;
													current_flow_split.stages.globalQueueId_stage_3 = core_down_queue.globalQueueId;
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
                        globalFlows.push_back(current_flow);
                    }
                }
            }
        }
    }
}


