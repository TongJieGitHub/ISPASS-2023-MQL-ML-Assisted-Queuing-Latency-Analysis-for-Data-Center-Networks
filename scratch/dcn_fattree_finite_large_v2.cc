#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <chrono>
#include <ctime>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <unordered_map>
#include <sys/stat.h>
#include <sys/types.h>

#include "ns3/flow-monitor-module.h"
#include "ns3/bridge-helper.h"
#include "ns3/bridge-net-device.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/csma-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traffic-control-module.h"
#include "ns3/nix-vector-helper.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE("Dcn-Star");

const string COMMUNICATION_PATTERN_ONETOONE = "oneToOne";
const string COMMUNICATION_PATTERN_ONETOMANY = "oneToMany";
const string COMMUNICATION_PATTERN_MANYTOONE = "manyToOne";
const string COMMUNICATION_PATTERN_CUSTOM = "custom";
// const string TRAFFIC_SOURCE_ONOFF = "onOff";
// const string TRAFFIC_SOURCE_EXPONENTIAL = "exponential";
const string TRAFFIC_SOURCE_GEN_EXPONENTIAL = "generalizedExponential";
const string INTERNET_PROTOCOL_UDP = "ns3::UdpSocketFactory";
const string INTERNET_PROTOCOL_TCP = "ns3::TcpSocketFactory";

// Function to create address string from numbers
char *toString(int a, int b, int c, int d)
{

	int first = a;
	int second = b;
	int third = c;
	int fourth = d;

	char *address = new char[30];
	char firstOctet[30], secondOctet[30], thirdOctet[30], fourthOctet[30];

	bzero(address, 30);

	snprintf(firstOctet, 10, "%d", first);
	strcat(firstOctet, ".");
	snprintf(secondOctet, 10, "%d", second);
	strcat(secondOctet, ".");
	snprintf(thirdOctet, 10, "%d", third);
	strcat(thirdOctet, ".");
	snprintf(fourthOctet, 10, "%d", fourth);

	strcat(thirdOctet, fourthOctet);
	strcat(secondOctet, thirdOctet);
	strcat(firstOctet, secondOctet);
	strcat(address, firstOctet);

	return address;
}

// Function to split a string by a delimiter
std::vector<std::string> split(std::string str, std::string sep)
{
	char *cstr = const_cast<char *>(str.c_str());
	char *current;
	std::vector<std::string> arr;
	current = strtok(cstr, sep.c_str());
	while (current != NULL)
	{
		arr.push_back(current);
		current = strtok(NULL, sep.c_str());
	}
	return arr;
}

// Main function
int main(int argc, char *argv[])
{

	// 01. Parameter and Variable Declaration:

	// Simulation Parameters
	int randSeed = 1;
	double simTime = 11;
	// int numServer = 4;

	// char dirname[100];
	char fname[1000];

	int numPods = 4;			 // Number of Pods = number of ports per switch
	int numAggSwInPod;			 // Number of aggr switches in each pod
	int numEdgeSwInPod;			 // Number of edge switches in each pod
	int numNodesInEdge = 0;		 // Number of nodes in each edge switch
	int numNodesInPod;			 // Number of nodes in each pod
	int numCoreSw;				 // Total number of core switches
	int numAggSw;				 // Total number of aggr switches
	int numEdgeSw;				 // Total number of edge switches
	int numNodes;				 // Total number of hosts
	int numApps;				 // Total number of hosts
	bool showIpAddresses = true; // Switch for printing device ip addresses

	// Link parameters
	string linkDataRate = "100";
	uint64_t linkDelay = 0;
	int queueSize = 100; // default: 100 packets

	// Application parameters
	string netProtocol = INTERNET_PROTOCOL_TCP;					// UDP, TCP
	string communicationPattern = COMMUNICATION_PATTERN_CUSTOM; // OneToOne, OneToMany, ManyToOne
	string trafficSource = TRAFFIC_SOURCE_GEN_EXPONENTIAL;		// GeneralizedExponential
	string sourceDataRate = "0.1";
	string sourceOnTime = "[Mean=1]";
	string sourceOffTime = "[Mean=0]";
	float p_burst = 0.0;
	bool ecmp = false;
	string trafficFile = "4x4";
	bool enableLog = false;

	// const string STATISTICS_FILE = "statistics/ft_4x4_GEG1_pb0_cs/dcn-star";

	char sourceMaxBytes[] = "0"; // unlimited
	double appTime = 10;
	int port = 50001;
	int randomNode = -1;
	Ptr<Ipv4> ipv4;
	Ipv4Address serverIpAddress;
	Ipv4Address clientIpAddress;
	unordered_map<uint32_t, int> address_client_map;
	unordered_map<uint32_t, int> address_server_map;

	// Packet/Trasnmission parameters
	string packetSizeDistrubution = "uniform"; // deterministic, uniform, exponential
	int packetSize = 1024;
	int minPacketSize = 512;
	int maxPacketSize = 1536;
	double burstProbability = 0.0;

	// Set parameters via command line arguments
	CommandLine cmd;
	cmd.AddValue("randSeed", "Global random seed (Default 1):", randSeed);
	cmd.AddValue("numPods", "Number of the array 'k' (Default 4):", numPods);
	cmd.AddValue("numNodsInEdge", "Number of Nodes in Edge:", numNodesInEdge);
	cmd.AddValue("appTime", "Application duration (Default 10):", appTime);
	cmd.AddValue("simTime", "Simulation duration (Default 11):", simTime);
	cmd.AddValue("delay", "Link delay (Default 0):", linkDelay);
	cmd.AddValue("dataRate", "Link data rate (Default 100 Mbps):", linkDataRate);
	cmd.AddValue("communicationPattern", "Communication pattern:", communicationPattern);
	cmd.AddValue("trafficSource", "Traffic source:", trafficSource);
	cmd.AddValue("dataRate_OnOff", "Source data rate (Default 0.20 Mbps):", sourceDataRate);
	cmd.AddValue("onTime", "Source app on state time (Default [Mean=1]):", sourceOnTime);
	cmd.AddValue("offTime", "Source app off state time (Default [Mean=0]):", sourceOffTime);
	cmd.AddValue("packetSizeDistrubution", "Source packet size distrubution (Default deterministic):", packetSizeDistrubution);
	cmd.AddValue("packetSize", "packetSize (Default 1024):", packetSize);
	cmd.AddValue("minPacketSize", "Min PacketSize (Default 512):", minPacketSize);
	cmd.AddValue("maxPacketSize", "Max PacketSize (Default 1536):", maxPacketSize);
	cmd.AddValue("p_burst", "Burst probability (Default 0):", p_burst);
	cmd.AddValue("queueSize", "Queue size as packets (Default 100p):", queueSize);
	cmd.AddValue("randomNode", "Set distinct selected client/server node (Default -1):", randomNode);
	cmd.AddValue("netProtocol", "ns3::UdpSocketFactory / ns3::TcpSocketFactory", netProtocol);
	cmd.AddValue("ecmp", "use ecmp routing(Default false):", ecmp);
	cmd.AddValue("trafficFile", "traffic file(Default 4x4):", trafficFile);
	cmd.AddValue("enableLog", "enable flow log(Default false):", enableLog);

	cmd.Parse(argc, argv);

	char arr[100];
	// sprintf(dirname, "statistics/ft_%s_GEG1_pb%.1f", trafficFile, p_burst);
	sprintf(arr, "statistics/ft_%s_GEG1_pb%.1f/dcn-star", trafficFile.c_str(), p_burst);
	std::string STATISTICS_FILE(arr);
	// mkdir(dirname, 0777);

	numAggSwInPod = numPods / 2;  // Number of aggr switches in each pod
	numEdgeSwInPod = numPods / 2; // Number of edge switches in each pod
	if (numNodesInEdge == 0)
		numNodesInEdge = numPods / 2;				 // Number of nodes in each edge switch
	numNodesInPod = numNodesInEdge * numEdgeSwInPod; // Number of nodes in each pod
	numCoreSw = ((numPods / 2) * (numPods / 2));	 // Total number of core switches
	numAggSw = (numAggSwInPod * numPods);			 // Total number of aggr switches
	numEdgeSw = (numEdgeSwInPod * numPods);			 // Total number of edge switches
	numNodes = (numNodesInPod * numPods);			 // Total number of hosts
	numApps = 2000 * numNodes;						 // Total number of hosts

	// Parameter adjusment
	linkDataRate = linkDataRate + "Mbps";
	string sourceDataRateMbps = sourceDataRate + "Mbps";
	string queueSizePkt;
	queueSizePkt = std::to_string(queueSize / 2) + "p";

	// Program variables
	ns3::RngSeedManager::SetSeed(randSeed);
	auto start = std::chrono::system_clock::now();

	string filename = STATISTICS_FILE + ".xml"; // filename for Flow Monitor xml output file
	string routing;
	if (ecmp)
	{
		routing = "ecmp";
	}
	else
	{
		routing = "bottleneck";
	}

	// Display all parameters
	cout << "==================================================================="
		 << "\n";
	cout << "Global random seed (Default 0): " << randSeed << "\n";
	cout << "Number of nodes (Default 8): " << numNodes << "\n";
	cout << "Application duration (Default 10): " << appTime << "\n";
	cout << "Simulation duration (Default 11): " << simTime << "\n";
	cout << "Link delay (Default 0): " << linkDelay << "\n";
	cout << "Link data rate (Default 100 Mbps): " << linkDataRate << "\n";
	cout << "Communication Pattern: " << communicationPattern << "\n";
	cout << "Traffic source: " << trafficSource << "\n";
	cout << "Source data rate (Default 0.20 Mbps): " << sourceDataRateMbps << "\n";
	cout << "Source app on state time (Default [Mean=1]): " << sourceOnTime << "\n";
	cout << "Source app off state time (Default [Mean=0]): " << sourceOffTime << "\n";
	cout << "Source packet size distrubution (Default deterministic): " << packetSizeDistrubution << "\n";
	cout << "packetSize (Default 1024): " << packetSize << "\n";
	cout << "Min PacketSize (Default 512): " << minPacketSize << "\n";
	cout << "Max PacketSize (Default 1536): " << maxPacketSize << "\n";
	cout << "Burst probability (Default 0): " << burstProbability << "\n";
	cout << "Queue size (Default 100 packets): " << queueSize << "\n";
	cout << "Seed =  " << ns3::RngSeedManager::GetSeed() << "\n";
	cout << "netProtocol: " << netProtocol << "\n";
	cout << "ecmp: " << ecmp << "\n";
	cout << "==================================================================="
		 << "\n";

	// LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
	// LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
	// LogComponentEnable ("Ipv4FlowProbe", LOG_LEVEL_INFO);
	// LogComponentEnable ("ApplicationPacketProbe", LOG_LEVEL_INFO);
	// LogComponentEnable ("PointToPointNetDevice", LOG_LEVEL_INFO);
	// LogComponentEnable ("PfifoFastQueueDisc", LOG_LEVEL_LOGIC);

	// 02. Creating Network Elements:

	// Initialize Internet Stack and Routing Protocols
	InternetStackHelper internet;
	Ipv4NixVectorHelper nixRouting;
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(ecmp));
	if (!ecmp)
	{
		list.Add(staticRouting, 0);
		list.Add(nixRouting, 10);
		internet.SetRoutingHelper(list);
	}

	// Create core switches
	NodeContainer coreSw;
	coreSw.Create(numCoreSw);
	internet.Install(coreSw);

	// Create Aggregation switches
	NodeContainer aggSw;
	aggSw.Create(numAggSw);
	internet.Install(aggSw);

	// Create Edge switches
	NodeContainer edgeSw;
	edgeSw.Create(numEdgeSw);
	internet.Install(edgeSw);

	// Create Nodes
	NodeContainer nodes; // Nodes
	nodes.Create(numNodes);
	internet.Install(nodes);

	// 03. Creating the Links and Connecting Nodes to the Star Switch

	// traffic control
	TrafficControlHelper tchPfifo;
	// Jie
	tchPfifo.SetRootQueueDisc("ns3::PfifoFastQueueDisc", "MaxSize", StringValue(queueSizePkt));

	// Inintialize the Helpers
	Ipv4AddressHelper address;
	PointToPointHelper p2p;

	// Set link attributes
	p2p.SetDeviceAttribute("DataRate", StringValue(linkDataRate));
	// p2p.SetDeviceAttribute ("SourceDataRate", StringValue (sourceDataRate));
	p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds(linkDelay)));
	// 2p.SetChannelAttribute ("StarSwitchId", UintegerValue(starSwitch.Get(0)->GetId()));
	//  p2p.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (queueSize))); // p in 100p stands for packets
	p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(queueSizePkt));

	int x = 1;										 // Print counter
	Ipv4AddressHelper addressCA;					 // Ipv4 helper - for only NS3 stack
	Ipv4InterfaceContainer icCA[numPods][numCoreSw]; // Interface Container to print assigned IP addresses - for only NS3 stack

	// Set up links between core and aggregation switches
	for (int pod = 0; pod < numPods; pod++)
	{
		for (int core = 0; core < numCoreSw; core++)
		{

			// Get agg sw number to connect with current core sw
			int aggr = (int)(core / (numPods / 2));
			int aggrn = numAggSwInPod * pod + aggr;

			// install link
			NodeContainer ncCA;
			NetDeviceContainer ndcCA;

			// Connect devices
			ncCA = NodeContainer(coreSw.Get(core), aggSw.Get(aggrn));
			ndcCA = p2p.Install(ncCA);
			tchPfifo.Install(ndcCA);

			// Assign IP addresses to two ends of the each link
			addressCA.SetBase(toString(core + 1, pod + 1, aggr + 1, 0), "255.255.255.0", "0.0.0.1");
			icCA[pod][core] = addressCA.Assign(ndcCA);
			if (showIpAddresses)
			{
				std::cout << "**********************************************" << std::endl;
				std::cout << "NS3 - Count: " << x << std::endl;
				std::cout << "CORE - AGGR" << std::endl;
				std::cout << "CORE IP: ";
				icCA[pod][core].GetAddress(0).Print(std::cout);
				std::cout << std::endl;
				std::cout << "AGG IP: ";
				icCA[pod][core].GetAddress(1).Print(std::cout);
				std::cout << std::endl;
				std::cout << "**********************************************" << std::endl;
			}
			x++;
		}
	}

	std::cout << "==============================================" << std::endl;
	std::cout << "Core-Agg Links created." << std::endl;
	std::cout << "==============================================" << std::endl;

	x = 1; // print counter reset

	Ipv4AddressHelper addressAE;
	Ipv4InterfaceContainer icAE[numPods][numAggSwInPod][numEdgeSwInPod];

	// set up links between aggregation and edge switches
	for (int pod = 0; pod < numPods; pod++)
	{
		for (int aggr = 0; aggr < numAggSwInPod; aggr++)
		{
			for (int edge = 0; edge < numEdgeSwInPod; edge++)
			{
				int aggrn = numAggSwInPod * pod + aggr;
				int edgen = numEdgeSwInPod * pod + edge;

				// install link
				NodeContainer ncAE;
				NetDeviceContainer ndcAE;

				// Connect devices
				ncAE = NodeContainer(aggSw.Get(aggrn), edgeSw.Get(edgen));
				ndcAE = p2p.Install(ncAE);
				tchPfifo.Install(ndcAE);

				// Assign IP addresses to two ends of the each link
				addressAE.SetBase(toString(pod + 101, aggr + 1, edge + 1, 0), "255.255.255.0", "0.0.0.1");
				icAE[pod][aggr][edge] = addressAE.Assign(ndcAE);
				if (showIpAddresses)
				{
					std::cout << "**********************************************" << std::endl;
					std::cout << "NS3 - Count: " << x << std::endl;
					std::cout << "AGGR - EDGE" << std::endl;
					std::cout << "AGGR IP: ";
					icAE[pod][aggr][edge].GetAddress(0).Print(std::cout);
					std::cout << std::endl;
					std::cout << "EDGE IP: ";
					icAE[pod][aggr][edge].GetAddress(1).Print(std::cout);
					std::cout << std::endl;
					std::cout << "**********************************************" << std::endl;
				}
				x++;
			}
		}
	}

	std::cout << "==============================================" << std::endl;
	std::cout << "Agg-Edge Links created." << std::endl;
	std::cout << "==============================================" << std::endl;

	x = 1; // print counter reset

	Ipv4AddressHelper addressEN;
	Ipv4InterfaceContainer icEN[numPods][numEdgeSwInPod][numNodesInEdge];

	// set up links between edge switches and end nodes
	for (int pod = 0; pod < numPods; pod++)
	{
		for (int edge = 0; edge < numEdgeSwInPod; edge++)
		{
			for (int node = 0; node < numNodesInEdge; node++)
			{

				int edgen = numEdgeSwInPod * pod + edge;
				int noden = numNodesInPod * pod + numNodesInEdge * edge + node;

				/* install link */
				NodeContainer ncEN;
				NetDeviceContainer ndcEN;

				// Connect devices
				ncEN = NodeContainer(edgeSw.Get(edgen), nodes.Get(noden));
				ndcEN = p2p.Install(ncEN);
				tchPfifo.Install(ndcEN);

				// Assign IP addresses to two ends of the each link
				addressEN.SetBase(toString(pod + 201, edge + 1, node + 1, 0), "255.255.255.0", "0.0.0.1");
				icEN[pod][edge][node] = addressEN.Assign(ndcEN);
				if (showIpAddresses)
				{
					std::cout << "**********************************************" << std::endl;
					std::cout << "NS3 - Count: " << x << std::endl;
					std::cout << "EDGE - HOST" << std::endl;
					std::cout << "EDGE IP: ";
					icEN[pod][edge][node].GetAddress(0).Print(std::cout);
					std::cout << std::endl;
					std::cout << "HOST IP: ";
					icEN[pod][edge][node].GetAddress(1).Print(std::cout);
					std::cout << std::endl;
					std::cout << "**********************************************" << std::endl;
				}
				x++;
			}
		}
	}

	std::cout << "==============================================" << std::endl;
	std::cout << "Edge-Node Links created." << std::endl;
	std::cout << "==============================================" << std::endl;

	// 03. Creating and Configuring the Source and the Target Applications

	ApplicationContainer *app = new ApplicationContainer[numApps];
	int appCount = 0;

	// The 1st element specifies the Node's status: 1=Client, 2=Server, 0=NotSet
	// The 2nd element specifies the Node's source or target Node number.
	//      if the Node is client then the 2nd element is its Server Node number,
	//      if the Node is server then the 2nd element is its Client Node number.
	unsigned int clientServer[numNodes][2]; // clientServer[NodeStatus][Client/Server Node]

	// Reset all Nodes' status
	for (int node = 0; node < numNodes; node++)
	{
		clientServer[node][0] = 0;
		clientServer[node][1] = 0;
	}

	// If communication pattern is custom then read Client-Server couples from the file
	if (communicationPattern == COMMUNICATION_PATTERN_CUSTOM)
	{
		// Get client server couples from file
		string line;
		int clientNode;
		int serverNode;
		std::vector<std::string> arr;
		stringstream converter;
		// int appCount = 0;
		ifstream myfile("scratch/dcn-traffic_" + trafficFile + ".txt");
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				if (!line.empty() && line.find(":") != std::string::npos)
				{
					arr = split(line, ":");

					if ((arr[0][0] == 'C' || arr[0][0] == 'c') && (arr[1][0] == 'S' || arr[1][0] == 's'))
					{

						appCount++;
						stringstream converter1(arr[0].erase(0, 1));
						converter1 >> clientNode;

						stringstream converter2(arr[1].erase(0, 1));
						converter2 >> serverNode;

						if (clientNode >= numNodes || serverNode >= numNodes)
						{
							cout << "ERROR! Server or Client node number cannot be greater than the total number of nodes.\n";
							return 0;
						}

						// Get server and client IP addresses
						ipv4 = nodes.Get(serverNode)->GetObject<Ipv4>();
						serverIpAddress = ipv4->GetAddress(1, 0).GetLocal();

						ipv4 = nodes.Get(clientNode)->GetObject<Ipv4>();
						clientIpAddress = ipv4->GetAddress(1, 0).GetLocal();

						// Install server application if not already installed
						if (clientServer[serverNode][0] != 2)
						{
							uint16_t portSink = 50001;
							Address hubLocalAddress(InetSocketAddress(serverIpAddress, portSink));
							PacketSinkHelper packetSinkHelper(netProtocol, hubLocalAddress);
							ApplicationContainer serverApp = packetSinkHelper.Install(nodes.Get(serverNode));
							serverApp.Start(Seconds(0.0));
							serverApp.Stop(Seconds(simTime));
						}

						// Initialize the source application
						GeneralizedExponentialHelper sourceApp = GeneralizedExponentialHelper(netProtocol, Address(InetSocketAddress(Ipv4Address(serverIpAddress), port)));

						// Set source app attributes
						sourceApp.SetAttribute("OnTime", StringValue("ns3::ExponentialRandomVariable" + sourceOnTime));
						sourceApp.SetAttribute("OffTime", StringValue("ns3::ExponentialRandomVariable" + sourceOffTime));
						sourceApp.SetAttribute("PacketSize", UintegerValue(packetSize));
						sourceApp.SetAttribute("DataRate", StringValue(sourceDataRateMbps));
						sourceApp.SetAttribute("MaxBytes", StringValue(sourceMaxBytes));
						sourceApp.SetAttribute("packetSizeDistrubution", StringValue(packetSizeDistrubution));
						sourceApp.SetAttribute("MinPacketSize", UintegerValue(minPacketSize));
						sourceApp.SetAttribute("MaxPacketSize", UintegerValue(maxPacketSize));
						sourceApp.SetAttribute("p_burst", DoubleValue(p_burst));

						// Install source app to the client
						app[appCount] = sourceApp.Install(nodes.Get(clientNode));

						cout << clientNode << ".Node (Client): " << clientIpAddress << " --> " << serverNode << ".Node (Server):" << serverIpAddress << "\n";
						cout << "Client Node ID: " << nodes.Get(clientNode)->GetId() << "\n";
						cout << "Server Node ID: " << nodes.Get(serverNode)->GetId() << "\n";

						clientServer[clientNode][0] = 1;		  // mark as client
						clientServer[clientNode][1] = serverNode; // set server node
						clientServer[serverNode][0] = 2;		  // mark as server
						clientServer[serverNode][1] = clientNode; // set client node
						communicationPattern = COMMUNICATION_PATTERN_CUSTOM;

						address_client_map[clientIpAddress.Get()] = clientNode;
						address_server_map[serverIpAddress.Get()] = serverNode;
					}
				}
			}
			myfile.close();
		}
		else
			cout << "Unable to open file";
	}
	else
	{
		// Set client and server nodes depending on communication pattern
		for (int i = 0; i < numNodes; i++)
		{
			// If status already set then pass
			if (clientServer[i][0] > 0)
				continue;

			if (communicationPattern == COMMUNICATION_PATTERN_ONETOONE)
			{
				randomNode = rand() % numNodes + 0;
				while ((clientServer[randomNode][0] != 0) || (randomNode == i))
				{
					randomNode = rand() % numNodes;
				}

				clientServer[i][0] = 1;			 // mark as client
				clientServer[i][1] = randomNode; // set server node
				clientServer[randomNode][0] = 2; // mark as server
				clientServer[randomNode][1] = i; // set client node
			}
			else if (communicationPattern == COMMUNICATION_PATTERN_MANYTOONE)
			{
				if (randomNode == -1)
					randomNode = rand() % numNodes + 0;

				clientServer[i][0] = 1;			  // mark as client
				clientServer[i][1] = randomNode;  // set server node
				clientServer[randomNode][0] = 2;  // mark as server
				clientServer[randomNode][1] = -1; // set client node
			}
			else if (communicationPattern == COMMUNICATION_PATTERN_ONETOMANY)
			{
				if (randomNode == -1)
					randomNode = rand() % numNodes + 0;

				clientServer[i][0] = 2;			 // mark as server
				clientServer[i][1] = randomNode; // set client node
				clientServer[randomNode][0] = 1; // mark as client
				clientServer[randomNode][1] = i; // set server node
			}
		}

		// Create applications and install to the nodes
		for (int i = 0; i < numNodes; i++)
		{
			if (clientServer[i][0] == 2) // If the node is a server
			{
				uint16_t portSink = 50001;
				Address hubLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), portSink));
				PacketSinkHelper packetSinkHelper(netProtocol, hubLocalAddress);
				ApplicationContainer serverApp = packetSinkHelper.Install(nodes.Get(i));
				serverApp.Start(Seconds(0.0));
				serverApp.Stop(Seconds(simTime));

				// If One node sends data to all other nodes (OneToMany);
				// Install the same number of client apps as the number of servers
				if (communicationPattern == COMMUNICATION_PATTERN_ONETOMANY)
				{
					// Get server and client IP addresses
					ipv4 = nodes.Get(i)->GetObject<Ipv4>();
					serverIpAddress = ipv4->GetAddress(1, 0).GetLocal();

					ipv4 = nodes.Get(clientServer[i][1])->GetObject<Ipv4>();
					clientIpAddress = ipv4->GetAddress(1, 0).GetLocal();

					// Initialize the source application
					GeneralizedExponentialHelper sourceApp = GeneralizedExponentialHelper(netProtocol, Address(InetSocketAddress(Ipv4Address(serverIpAddress), port)));

					// Set source app attributes
					sourceApp.SetAttribute("OnTime", StringValue("ns3::ExponentialRandomVariable" + sourceOnTime));
					sourceApp.SetAttribute("OffTime", StringValue("ns3::ExponentialRandomVariable" + sourceOffTime));
					sourceApp.SetAttribute("PacketSize", UintegerValue(packetSize));
					sourceApp.SetAttribute("DataRate", StringValue(sourceDataRateMbps));
					sourceApp.SetAttribute("MaxBytes", StringValue(sourceMaxBytes));
					sourceApp.SetAttribute("packetSizeDistrubution", StringValue(packetSizeDistrubution));
					sourceApp.SetAttribute("MinPacketSize", UintegerValue(minPacketSize));
					sourceApp.SetAttribute("MaxPacketSize", UintegerValue(maxPacketSize));
					sourceApp.SetAttribute("p_burst", DoubleValue(p_burst));

					// Install source app to the client
					app[i] = sourceApp.Install(nodes.Get(clientServer[i][1]));

					cout << clientServer[i][1] << ".Node (Client): " << clientIpAddress << " --> " << i << ".Node (Server):" << serverIpAddress << "\n";
					cout << "Client Node ID: " << nodes.Get(clientServer[i][1])->GetId() << "\n";
					cout << "Server Node ID: " << nodes.Get(i)->GetId() << "\n";
				}
			}
			else if (clientServer[i][0] == 1 && communicationPattern != COMMUNICATION_PATTERN_ONETOMANY)
			{
				// Get server and client IP addresses
				ipv4 = nodes.Get(clientServer[i][1])->GetObject<Ipv4>();
				serverIpAddress = ipv4->GetAddress(1, 0).GetLocal();

				ipv4 = nodes.Get(i)->GetObject<Ipv4>();
				clientIpAddress = ipv4->GetAddress(1, 0).GetLocal();

				// Initialize the source application
				GeneralizedExponentialHelper sourceApp = GeneralizedExponentialHelper(netProtocol, Address(InetSocketAddress(Ipv4Address(serverIpAddress), port)));

				// Set source app attributes
				sourceApp.SetAttribute("OnTime", StringValue("ns3::ExponentialRandomVariable" + sourceOnTime));
				sourceApp.SetAttribute("OffTime", StringValue("ns3::ExponentialRandomVariable" + sourceOffTime));
				sourceApp.SetAttribute("PacketSize", UintegerValue(packetSize));
				sourceApp.SetAttribute("DataRate", StringValue(sourceDataRateMbps));
				sourceApp.SetAttribute("MaxBytes", StringValue(sourceMaxBytes));
				sourceApp.SetAttribute("packetSizeDistrubution", StringValue(packetSizeDistrubution));
				sourceApp.SetAttribute("MinPacketSize", UintegerValue(minPacketSize));
				sourceApp.SetAttribute("MaxPacketSize", UintegerValue(maxPacketSize));
				sourceApp.SetAttribute("p_burst", DoubleValue(p_burst));

				// Install source Application to the client
				app[i] = sourceApp.Install(nodes.Get(i));

				cout << i << ".Node: " << clientIpAddress << " --> " << clientServer[i][1] << ".Server Node:" << serverIpAddress << "\n";
				cout << "Client Node ID: " << nodes.Get(i)->GetId() << "\n";
				cout << "Server Node ID: " << nodes.Get(clientServer[i][1])->GetId() << "\n";
			}
		}

		cout << "==================================================================="
			 << "\n";
	}
	// 05. Starting the Simulation and Configuring the Outputs

	cout << "Start Simulation.. "
		 << "\n";
	for (int i = 0; i <= appCount; i++)
	{
		app[i].Start(Seconds(0.0));
		app[i].Stop(Seconds(appTime));
	}
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	// Enable flowmonitor
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();
	monitor->enable_log = enableLog;
	monitor->sourceDataRate = sourceDataRate;
	sprintf(fname, "%s_pkt%d_delta%d_queue%d_%s_%s_time%.0f.csv", trafficFile.c_str(), (maxPacketSize + minPacketSize) / 2, (maxPacketSize - maxPacketSize) / 2, queueSize, netProtocol.substr(5, 3).c_str(), routing.c_str(), appTime);
	monitor->filename = fname;
	// // Enable pcap
	// p2p.EnablePcapAll (STATISTICS_FILE);

	// // Enable ascii
	// AsciiTraceHelper ascii;
	// p2p.EnableAsciiAll (ascii.CreateFileStream (STATISTICS_FILE + ".tr"));
	auto end_setup = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_setup_seconds = end_setup - start;

	// Run simulation.
	NS_LOG_INFO("Run Simulation.");
	Simulator::Stop(Seconds(simTime));
	Simulator::Run();

	// Configure Flowmonitor outputs
	monitor->CheckForLostPackets();

	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

	int txPackets = 0, rxPackets = 0, lostPackets = 0, txBytes = 0, rxBytes = 0;
	ns3::Time delaySum = NanoSeconds(0.0);
	ns3::Time jitterSum = NanoSeconds(0.0);
	ns3::Time lastDelay = NanoSeconds(0.0);
	int timesForwarded = 0;
	// double averageDelay=0;
	double throughput = 0;
	int nFlows = 0;

	vector<uint32_t> flow_id;
	vector<uint32_t> flow_rxPackets;
	vector<uint32_t> flow_rxBytes;
	vector<uint32_t> flow_lostPackets;
	vector<int64_t> flow_delaySum;
	vector<double> flow_avgdelay;
	vector<double> flow_goodput;
	vector<uint32_t> flow_txPackets;
	vector<uint32_t> flow_txBytes;
	vector<double> flow_injection;

	// int appNum;
	for (int i = 0; i < 2 * appCount; i++)
	{
		flow_id.push_back(0);
		flow_rxPackets.push_back(0);
		flow_rxBytes.push_back(0);
		flow_lostPackets.push_back(0);
		flow_delaySum.push_back(0);
		flow_avgdelay.push_back(0);
		flow_goodput.push_back(0);
		flow_txPackets.push_back(0);
		flow_txBytes.push_back(0);
		flow_injection.push_back(0);
	}

	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin(); iter != stats.end(); ++iter)
	{
		txPackets += iter->second.txPackets;
		txBytes += iter->second.txBytes;
		rxPackets += iter->second.rxPackets;
		rxBytes += iter->second.rxBytes;
		lostPackets += iter->second.lostPackets;
		delaySum += iter->second.delaySum;
		jitterSum += iter->second.jitterSum;
		lastDelay += iter->second.lastDelay;
		timesForwarded += iter->second.timesForwarded;
		// averageDelay+=iter->second.delaySum.GetNanoSeconds()/iter->second.rxPackets;
		throughput += (((iter->second.txBytes * 8.0) / (iter->second.timeLastTxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds())) / 1024) / 1024;

		flow_id.at(nFlows) = iter->first;
		flow_rxPackets.at(nFlows) = iter->second.rxPackets;
		flow_rxBytes.at(nFlows) = iter->second.rxBytes;
		flow_lostPackets.at(nFlows) = iter->second.lostPackets;
		flow_delaySum.at(nFlows) = iter->second.delaySum.GetNanoSeconds();
		flow_goodput.at(nFlows) = iter->second.rxBytes * 8.0 * 1000 / (iter->second.timeLastRxPacket.GetNanoSeconds() - iter->second.timeFirstRxPacket.GetNanoSeconds() + 1);
		// flow_avgdelay[nFlows] = flow_delaySum[nFlows]/flow_rxPackets[nFlows];
		if (flow_rxPackets[nFlows] != 0)
			flow_avgdelay[nFlows] = static_cast<double>(flow_delaySum[nFlows]) / flow_rxPackets[nFlows];
		else
			flow_avgdelay[nFlows] = 0;
		flow_txPackets.at(nFlows) = iter->second.txPackets;
		flow_txBytes.at(nFlows) = iter->second.txBytes;
		flow_injection.at(nFlows) = iter->second.txBytes * 8.0 * 1000 / (iter->second.timeLastTxPacket.GetNanoSeconds() - iter->second.timeFirstTxPacket.GetNanoSeconds() + 1);

		nFlows++;

		// std::cout << "**********************************************" << "\n";
		// std::cout << "nFlows: " << nFlows << "\n";
		// std::cout << "rxPackets: " << iter->second.rxPackets << "\n";
		// std::cout << "rxBytes: " << iter->second.rxBytes << "\n";
		// std::cout << "delaySum: " << iter->second.delaySum.GetNanoSeconds() << "\n";

		// std::cout << "**********************************************" << "\n";
	}

	ofstream of;

	// build inverse_map
	std::unordered_map<FlowId, Ipv4FlowClassifier::FiveTuple> inverted_map;
	for (std::map<Ipv4FlowClassifier::FiveTuple, FlowId>::const_iterator
			 iter = classifier->m_flowMap.begin();
		 iter != classifier->m_flowMap.end(); iter++)
	{
		inverted_map[iter->second] = iter->first;
	}

	for (int i = 0; i < nFlows; i++)
	{
		// Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow_id[i]);
		Ipv4FlowClassifier::FiveTuple t = inverted_map[flow_id[i]];
		Ipv4Address sourceAddress = t.sourceAddress;
		Ipv4Address destinationAddress = t.destinationAddress;
		char s[100];
		sprintf(s, "%d.%d.%d.%d", ((sourceAddress.Get() >> 24) & 0xff), ((sourceAddress.Get() >> 16) & 0xff), ((sourceAddress.Get() >> 8) & 0xff), ((sourceAddress.Get() >> 0) & 0xff));
		int client = address_client_map[sourceAddress.Get()];
		char d[100];
		sprintf(d, "%d.%d.%d.%d", ((destinationAddress.Get() >> 24) & 0xff), ((destinationAddress.Get() >> 16) & 0xff), ((destinationAddress.Get() >> 8) & 0xff), ((destinationAddress.Get() >> 0) & 0xff));
		int server = address_server_map[destinationAddress.Get()];

		if (nFlows <= 16384 || client == 0 || client == 1 || server == 0 || server == 1)
		{
			// sprintf(fname, "statistics/ft_%dx%d_GEG1_pb%.1f_cscs/flow_%s_%s.csv", numServer, numServer, p_burst, s, d);
			sprintf(fname, "statistics/statistics_%s_pkt%d_delta%d_queue%d_%s_%s_time%.0f.csv", trafficFile.c_str(), (maxPacketSize + minPacketSize) / 2, (maxPacketSize - minPacketSize) / 2, queueSize, netProtocol.substr(5, 3).c_str(), routing.c_str(), appTime);

			// cout << fname << endl;

			of.open(fname, ios::out | ios::app);
			of << sourceDataRate << "," << client << "," << server << "," << s << "," << d << "," << flow_injection[i] << "," << flow_txPackets[i] << "," << flow_txBytes[i] << "," << flow_rxPackets[i] << "," << flow_rxBytes[i] << "," << flow_delaySum[i] << "," << flow_avgdelay[i] << "," << flow_goodput[i] << "," << flow_lostPackets[i] << "," << numNodes << "," << randSeed << endl;

			of.close();
		}
	}

	auto end_sim = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_sim_seconds = end_sim - end_setup;

	cout << "==================================================================="
		 << "\n";
	cout << "Flowmon Stats "
		 << "\n";
	cout << "--------------"
		 << "\n";
	cout << "communicationPattern: " << communicationPattern << "\n";
	cout << "TrafficSource: " << trafficSource << "\n";
	cout << "PacketSizeDistrubution: " << packetSizeDistrubution << "\n";
	cout << "NumberOfNodes: " << numNodes << "\n";
	cout << "txPackets: " << txPackets << "\n";
	cout << "rxPackets: " << rxPackets << "\n";
	cout << "DelaySum: " << delaySum << "\n";
	cout << "JitterSum: " << jitterSum << "\n";
	cout << "LastDelay: " << lastDelay << "\n";
	cout << "LostPackets: " << lostPackets << "\n";
	cout << "TimesForwarded: " << timesForwarded << "\n";
	cout << "AverageDelay: " << (delaySum / rxPackets) << "\n";
	cout << "LinkBandwidth: " << linkDataRate << "\n";
	cout << "nFlows: " << nFlows << "\n";
	cout << "==================================================================="
		 << "\n";

	monitor->SerializeToXmlFile(filename, true, true);
	cout << "Simulation finished "
		 << "\n";

	ofstream of_time;
	sprintf(fname, "statistics/timing_%s_pkt%d_delta%d_queue%d_%s_%s_time%.0f.csv", trafficFile.c_str(), (maxPacketSize + minPacketSize) / 2, (maxPacketSize - minPacketSize) / 2, queueSize, netProtocol.substr(5, 3).c_str(), routing.c_str(), appTime);
	of_time.open(fname, ios::out | ios::app);
	of_time << sourceDataRate << "," << elapsed_setup_seconds.count() << "," << elapsed_sim_seconds.count() << endl;
	of_time.close();

	Simulator::Destroy();
	NS_LOG_INFO("Done.");

	return 0;
}
