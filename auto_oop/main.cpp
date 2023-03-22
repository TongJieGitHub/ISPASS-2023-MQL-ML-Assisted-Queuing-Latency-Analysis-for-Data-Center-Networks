#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <cmath>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ratio>
#include "util.h"
#include "fattree.h"
#include "fattreeL3.h"
#include "fattreeL2Custom.h"
#include "cxxopts.hpp"

// TODO: DO WE HAVE A README? WE SHOULD HAVE ONE AND CITE IT HERE.
int main(int argc, char *argv[])
{

    // Declaring the command line arguments
    double simTime;
    double windowTime;
    double warmupTime;
    int numPods;
    int queueSize;
    std::string topologyParam;
	double linkBandwidthParam;
	double datarate_unit;
    std::string unitsDatarateParam;
    std::string tag;
	int switchRadix;   // L2custom
	int numNodes;      // L2custom

    // CommandLine cmd;
    // cmd.AddValue("simTime", "Total time of simulation (in milliseconds), default value of 10,000 milliseconds", simTime);
    // cmd.AddValue("windowTime", "Size of the window (in milliseconds), default value of 200 milliseonds", windowTime);
    // cmd.AddValue("numPods", "Number of pods in the DCN, default value of 4", numPods);
    // cmd.Parse(argc, argv);

    // Set parameters via command line arguments
    // Users will add more command line arguments as necessary
    cxxopts::Options options("analytical_model", "A program to run analytical model with flow parameters from simulation to estimate end-to-end latency");
    options.add_options()
    ("s,simTime", "Set the simulation length (in milliseconds)", cxxopts::value<double>()->default_value("10000"))
    ("w,windowTime", "Set the window duration (in milliseconds)", cxxopts::value<double>()->default_value("200"))
    ("m,warmupTime", "Set the warm up duration (in milliseconds)", cxxopts::value<double>()->default_value("1000"))
    ("q,queueSize", "Set the queue length", cxxopts::value<int>()->default_value("128"))
    ("n,numPods", "Set the number of pods", cxxopts::value<int>()->default_value("4"))
    ("k,switchRadix", "#porst of the switch, used by L2Custom", cxxopts::value<int>()->default_value("8"))
    ("v,numNodes", "#nodes aka #hosts or #serVers", cxxopts::value<int>()->default_value("16"))
	("y,topologyParam", "Specify the topolgY, L3 or L2Custom", cxxopts::value<std::string>()->default_value("L3"))
    ("l,linkBandwidthParam", "Set the link bandwidth with no units", cxxopts::value<double>()->default_value("100"))
    ("u,unitsDatarateParam", "Set units for linkBandwidth and datarates",cxxopts::value<std::string>()->default_value("Mbps"))
    ("t,tag", "Set the tag name to identify run/result directories", cxxopts::value<std::string>()->default_value(""))
    ("h,help","Print help")
    ;

    // Parse command line arguments
    auto result = options.parse(argc, argv);

    // Print out help message at command line if necessary
    if (result.count("help")) {
      printf("%s\n", options.help().c_str());
      return 0;
    }

    // Read command line arguments into variables
    simTime             = result["simTime"].as<double>();     // in milliseconds
    windowTime          = result["windowTime"].as<double>();  // in milliseconds
    warmupTime          = result["warmupTime"].as<double>();  // in milliseconds
    queueSize           = result["queueSize"].as<int>();
    numPods             = result["numPods"].as<int>();
    switchRadix         = result["switchRadix"].as<int>();
    numNodes            = result["numNodes"].as<int>();
    topologyParam       = result["topologyParam"].as<std::string>();
    linkBandwidthParam  = result["linkBandwidthParam"].as<double>();
	unitsDatarateParam  = result["unitsDatarateParam"].as<std::string>();
    tag                 = result["tag"].as<std::string>();


    enum Topology {fattreeL3, fattreeL2custom};
	enum Topology topology = fattreeL3;    // default to fattreeL3
    if (topologyParam.compare("L2custom") == 0)
		topology = fattreeL2custom;

    if (unitsDatarateParam.compare("Mbps") == 0) 
		datarate_unit = 1000000;    // Mega bits per second
	else if (unitsDatarateParam.compare("Gbps") == 0)
		datarate_unit = 1000000000;  // Giga bits per second
	else
		datarate_unit = 1000000;    // default Mega bits per second

	double linkBandwidth = linkBandwidthParam * datarate_unit;
	cout << datarate_unit << endl;
	cout << linkBandwidth << endl;
	
	// Timer related variables
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    std::chrono::duration<double, std::milli> duration_sec;
    start = std::chrono::high_resolution_clock::now();

    // ===============================================================
    // defaults, may be override in switch statement below - or from the trace
    int packet_size = 500;            // bytes
    int packet_header = 30;           // bytes
    int bits = 8;
    int min_packet_size = 500;
    int max_packet_size = 500;
    int delta = max_packet_size - (max_packet_size + min_packet_size) / 2;

    string dataratefile;
    vector<double> datarate_list;
    vector<double> tmp;
    string suffix;
    vector<int> time_list;

    double divideDataRate;                              // amount to divideDataRate by - often divide by total number of flows
	//    double maxDataRate = linkBandwidth / datarate_unit;
    bool use_mimic = true;

    // TODO: Why do we need to do this? We can directly move this for loop at the time of reading files
    // TODO: Unnecessary step
    for (int t = warmupTime + windowTime; t <= simTime; t += windowTime)
        time_list.push_back(t);
	dataratefile = "../runs/" + tag + "/outputs_sim/mimic_";
	//    dataratefile = "../runs/dcn_fattree_finite_large_l2custom_100M_v3_q128_w10000_tL2c_size16_1flow_disGE_packetFixed_dr5p0_s1/outputs_sim/mimic_";
    suffix = "mimic_1x1";

    // switch (select_ns3_results)
    // {
    // case 0:
    //     k = 4;
    //     for (double d = 10.0 / 4; d <= 100.0 / 4; d += 5.0 / 4)
    //         datarate_list.push_back(d);
    //     dataratefile = "D4x4.txt";
    //     suffix = "4x4";
    //     break;
    // case 1:
    //     k = 4;
    //     for (double d = 10.0 / 7; d <= 100.0 / 7; d += 5.0 / 7)
    //         datarate_list.push_back(d);
    //     dataratefile = "D7x1.txt";
    //     suffix = "7x1";
    //     break;
    // case 2:
    //     k = 4;
    //     for (double d = 10.0 / 7; d <= 100.0 / 7; d += 5.0 / 7)
    //         datarate_list.push_back(d);
    //     dataratefile = "D1x7.txt";
    //     suffix = "1x7";
    //     break;
    // case 3:
    //     k = 4;
    //     for (double d = 10.0 / 7; d <= 100.0 / 7; d += 5.0 / 7)
    //         datarate_list.push_back(d);
    //     dataratefile = "D8x8alltoall.txt";
    //     suffix = "8x8alltoall";
    //     break;
    // case 20:
    //     k = 8;
    //     for (double d = 10.0 / 64; d <= 100.0 / 64; d += 5.0 / 64)
    //         datarate_list.push_back(d);
    //     dataratefile = "D64x64.txt";
    //     suffix = "64x64";
    //     break;
    // case 21:
    //     k = 8;
    //     for (double d = 10.0 / 127; d <= 100.0 / 127; d += 5.0 / 127)
    //         datarate_list.push_back(d);
    //     dataratefile = "D127x1.txt";
    //     suffix = "127x1";
    //     break;
    // case 22:
    //     k = 8;
    //     for (double d = 10.0 / 127; d <= 110.0 / 127; d += 5.0 / 127)
    //         datarate_list.push_back(d);
    //     dataratefile = "D1x127.txt";
    //     suffix = "1x127";
    //     break;
    // case 23:
    //     k = 8;
    //     for (double d = 10.0 / 127; d <= 110.0 / 127; d += 5.0 / 127)
    //         datarate_list.push_back(d);
    //     dataratefile = "D128x128alltoall.txt";
    //     suffix = "128x128alltoall";
    //     break;
    // case 30:
    //     k = 16;
    //     for (double d = 10.0 / 512; d <= 100.0 / 512; d += 10.0 / 512)
    //         datarate_list.push_back(d);
    //     dataratefile = "D512x512.txt";
    //     suffix = "512x512";
    //     break;
    // case 31:
    //     k = 16;
    //     for (double d = 10.0 / 1023; d <= 100.0 / 1023; d += 10.0 / 1023)
    //         datarate_list.push_back(d);
    //     dataratefile = "D1023x1.txt";
    //     suffix = "1023x1";
    //     break;
    // case 32:
    //     k = 16;
    //     for (double d = 10.0 / 1023; d <= 100.0 / 1023; d += 10.0 / 1023)
    //         datarate_list.push_back(d);
    //     dataratefile = "D1x1023.txt";
    //     suffix = "1x1023";
    //     break;
    // case 33:
    //     k = 16;
    //     for (double d = 10.0 / 1023; d <= 100.0 / 1023; d += 10.0 / 1023)
    //         datarate_list.push_back(d);
    //     dataratefile = "D1024x1024alltoall.txt";
    //     suffix = "1024x1024alltoall";
    //     break;
    // case 516: // 16 nodes - 2-tier custom - 8x8 all2all
    //     k = 8;
    //     numNodes = 16; // num nodes is an input
    //     topology = 2;
    //     linkBandwidth = 200000000000; // 200 Gbps
    //     datarate_unit = 1000000000;   // Giga bits per second
    //     // maxDataRate = linkBandwidth/datarate_unit;
    //     maxDataRate = 5 * linkBandwidth / datarate_unit;
    //     divideDataRate = 7;
    //     //        for (double d = 7.0 / divideDataRate; d <= maxDataRate; d += (1*7.0) / divideDataRate)
    //     // for (double d = 10.0 / divideDataRate; d <= 200/divideDataRate; d += (1*10.0) / divideDataRate)
    //     //             datarate_list.push_back(d);
    //     //		datarate_list.assign(2, 1.86, 2*1.86);
    //     //		tmp[] = {1.86, 2*1.86, 2*1.86};
    //     datarate_list.push_back(1.86);
    //     datarate_list.push_back(2 * 1.86);
    //     datarate_list.push_back(3 * 1.86);
    //     dataratefile = "D8x8alltoall.txt";
    //     suffix = "l2custom8x8alltoall";
    //     packet_size = 32;   // bytes
    //     packet_header = 30; // bytes
    //     bits = 8;
    //     min_packet_size = 32;
    //     max_packet_size = 32;
    //     delta = max_packet_size - (max_packet_size + min_packet_size) / 2;
    //     break;
    // case 517: // 16 nodes - 2-tier custom - 8x8 all2all - 200Mbps
    //     k = 8;
    //     numNodes = 16; // num nodes is an input
    //     topology = 2;
    //     linkBandwidth = 200000000; // 200 Mbps
    //     datarate_unit = 1000000;   // Mbps bits per second
    //                                // maxDataRate = linkBandwidth/datarate_unit;
    //     //         for (double d = 7.0 / divideDataRate; d <= maxDataRate; d += (1*7.0) / divideDataRate)
    //     //		datarate_list = {1,2,3,4,5,6,7};
    //     for (double d = 1; d <= 13; d += 1)
    //         datarate_list.push_back(d);
    //     dataratefile = "D8x8alltoall.txt";
    //     suffix = "l2custom8x8alltoall";
    //     //       dataratefile = "D2x2alltoall.txt";
    //     //        suffix = "l2custom2x2alltoall";
    //     packet_size = 32;   // bytes
    //     packet_header = 30; // bytes
    //     bits = 8;
    //     min_packet_size = 32;
    //     max_packet_size = 32;
    //     delta = max_packet_size - (max_packet_size + min_packet_size) / 2;
    //     break;
    // case 520: // 16 nodes - 2-tier custom - 1x7
    //     k = 8;
    //     numNodes = 16; // num nodes is an input
    //     topology = 2;
    //     divideDataRate = 7;
    //     for (double d = 10.0 / divideDataRate; d <= maxDataRate; d += 10.0 / divideDataRate)
    //         datarate_list.push_back(d);
    //     // dataratefile = "D4x4alltoall.txt";
    //     dataratefile = "D1x7.txt";
    //     suffix = "l2custom1x7";
    //     break;
    // case 5128: // 128 nodes - 2-tier custom
    //     k = 64;
    //     numNodes = 128; // numNodes is anput to FattreeL2custom
    //     topology = 2;
    //     divideDataRate = numNodes - 1;
    //     for (double d = 128.0 / divideDataRate; d <= maxDataRate; d += (2 * 128.0) / divideDataRate)
    //         datarate_list.push_back(d);
    //     dataratefile = "D128x128alltoall.txt";
    //     suffix = "l2custom128x128alltoall";
    //     break;
    // case 51024: // 1024 nodes - start fattreeL2custom
    //     k = 64;
    //     numNodes = 1024; // numNodes is anput to FattreeL2custom
    //     topology = 2;
    //     divideDataRate = numNodes - 1;
    //     for (double d = divideDataRate / divideDataRate; d <= maxDataRate; d += (2 * 128.0) / divideDataRate)
    //         datarate_list.push_back(d);
    //     dataratefile = "D1024x1024alltoall.txt";
    //     suffix = "l2custom1024x1024alltoall";
    //     break;
    // case -1:
    //     k = 4;
    //     for (int t = 0; t <= 10500; t += 200)
    //         time_list.push_back(t);
    //     use_mimic = true;
    //     // dataratefile = "../statistics_mimic_nodes_16_load_07_rand_1/mimic_";
    //     // suffix = "mimic_nodes_16_load_07_rand_1_max_0999";
    //     dataratefile = "../statistics_mimic_1x1/mimic_";
    //     suffix = "mimic_1x1";
    //     break;
    // default:
    //     k = 4;
    //     for (double d = 10.0 / 4; d <= 100.0 / 4; d += 5.0 / 4)
    //         datarate_list.push_back(d);
    //     dataratefile = "D4x4.txt";
    //     suffix = "4x4";
    //     break;
    // }

    //: es:todo: create an enumberated type for topology
    if (topology == fattreeL3)
    {
        cout << "\n========  FatreeL3 =======\n";
        if (use_mimic)
        {
            FattreeL3 fattree;
            fattree.use_sim_host_latency = true;
            suffix = suffix + "_q" + to_string(queueSize);

            fattree.create(numPods, linkBandwidth, datarate_unit);
            fattree.link();
            fattree.route();

            fattree.mapping_globalFlows();
            fattree.mapping_globalFlowSplits();

            // fattree.print_header_mimic("latency_per_flow_" + suffix + "_inf.csv");
            // fattree.print_header_mimic("latency_per_flow_" + suffix + "_finR.csv");
            // fattree.print_header_mimic("latency_per_flow_" + suffix + "_finC.csv");
            fattree.print_header_all_mimic("../runs/" + tag + "/reports_ana/latency_per_flow.csv");
            fattree.print_queue_header("../runs/" + tag + "/reports_ana/latency_per_queue.csv");

            // TODO: Why do we need this? Check the timestamp pointers
            // TODO: What timing is this?
            end = std::chrono::high_resolution_clock::now();
            duration_sec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
            std::ofstream of_time;
            of_time.open("../runs/" + tag + "/reports_ana/timing.csv", std::ios::out | std::ios::app);
            of_time << suffix << "," << 0 << "," << duration_sec.count() << endl;
            of_time.close();

            for (auto &timestamp : time_list)
            {
                start = std::chrono::high_resolution_clock::now();
                // TODO: Change this to log based display statements
                cout << "[ INFO] Processing time stamp: " << timestamp << endl;

                fattree.reset_flow();
                fattree.clear_queue_vector();
                fattree.load_traffic_file_mimic(dataratefile + std::to_string(timestamp) + ".txt");
                // fattree.update_flow_datarate(datarate, 0.0);
                // fattree.calc_CS_sqr_all(linkBandwidth, packet_size, packet_header, bits, min_packet_size, max_packet_size);
                fattree.calc_lambda_CA_sqr_stage_0_mimic();
                fattree.calc_stage_0_mimic();
                fattree.calc_stage_1_mimic();
                fattree.calc_stage_2_mimic();
                fattree.calc_stage_3_mimic();
                fattree.calc_stage_4_mimic();
                fattree.calc_stage_5_mimic();

                fattree.regression_tree_model();

                fattree.calc_flowSplit_latency_v2();
                fattree.calc_flow_latency_v2();

                // fattree.print_latency_inf_mimic("latency_per_flow_" + suffix + "_inf.csv", timestamp);
                // fattree.print_latency_finR_mimic("latency_per_flow_" + suffix + "_finR.csv", timestamp);
                // fattree.print_latency_finC_mimic("latency_per_flow_" + suffix + "_finC.csv", timestamp);
                // fattree.print_latency_all_mimic("latency_per_flow_" + suffix + ".csv", timestamp);
                // fattree.print_queue_info("queue_" + suffix + ".csv", timestamp);
                fattree.print_latency_all_mimic("../runs/" + tag + "/reports_ana/latency_per_flow.csv", timestamp);
                fattree.print_queue_info("../runs/" + tag + "/reports_ana/latency_per_queue.csv", timestamp);

                end = std::chrono::high_resolution_clock::now();
                duration_sec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
                of_time.open("../runs/" + tag + "/reports_ana/timing.csv", std::ios::out | std::ios::app);
                of_time << suffix << "," << timestamp << "," << duration_sec.count() << endl;
                of_time.close();
            }
        }
        else
        {
            FattreeL3 fattree;
            suffix = suffix + "_pkt" + to_string(packet_size) + "_delta" + to_string(delta) + "_q" + to_string(queueSize);

            fattree.create(numPods, linkBandwidth, datarate_unit);
            fattree.link();
            fattree.route();
            // fattree.printRoutingTable("routing.csv");
            // fattree.printQueues("queue.csv");

            fattree.mapping_globalFlows();
            fattree.mapping_globalFlowSplits();

            fattree.load_traffic_file(dataratefile);
            fattree.print_header("latency_per_flow_" + suffix + "_inf.csv");
            fattree.print_header("latency_per_flow_" + suffix + "_finR.csv");
            fattree.print_header("latency_per_flow_" + suffix + "_finC.csv");

            end = std::chrono::high_resolution_clock::now();
            duration_sec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
            std::ofstream of_time;
            of_time.open("../runs/" + tag + "/reports_ana/timing.csv", std::ios::out | std::ios::app);
            of_time << suffix << "," << 0 << "," << duration_sec.count() << endl;
            of_time.close();

            for (auto &datarate : datarate_list)
            {
                start = std::chrono::high_resolution_clock::now();
                cout << "data rate = " << datarate << "\n";

                fattree.clear_queue_vector();
                fattree.update_flow_datarate(datarate, 0.0);
                fattree.calc_CS_sqr_all(linkBandwidth, packet_size, packet_header, bits, min_packet_size, max_packet_size);
                fattree.calc_lambda_CA_sqr_stage_0();
                fattree.calc_stage_0();
                fattree.calc_stage_1();
                fattree.calc_stage_2();
                fattree.calc_stage_3();
                fattree.calc_stage_4();
                fattree.calc_stage_5();

                fattree.calc_flowSplit_latency();
                fattree.calc_flow_latency();

                fattree.print_latency_inf("latency_per_flow_" + suffix + "_inf.csv");
                fattree.print_latency_finR("latency_per_flow_" + suffix + "_finR.csv");
                fattree.print_latency_finC("latency_per_flow_" + suffix + "_finC.csv");

                end = std::chrono::high_resolution_clock::now();
                duration_sec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
                of_time.open("../runs/" + tag + "/reports_ana/timing.csv", std::ios::out | std::ios::app);
                of_time << suffix << "," << datarate << "," << duration_sec.count() << endl;
                of_time.close();
            }
        }
    }
    else
    {
        cout << "\n========  FatreeL2custom =======\n";
        cout << "\t   numNodes= " << numNodes << "\n";
        cout << "\tswitchRadix= " << switchRadix << "\n";
        cout << "\t  use_mimic= " << use_mimic << "\n";
        cout << "\t        tag= " << tag << "\n\n";
		
        if (use_mimic) {
            FattreeL2custom fattree;
            fattree.use_sim_host_latency = true;
            suffix = suffix + "_q" + to_string(queueSize);

  			fattree.create(switchRadix, numNodes, linkBandwidth, datarate_unit);
			fattree.link();
			fattree.route();
			fattree.printRoutingTable("routing.csv");
			fattree.printQueues("queue.csv");

            fattree.mapping_globalFlows();
            fattree.mapping_globalFlowSplits();

            // fattree.print_header_mimic("latency_per_flow_" + suffix + "_inf.csv");
            // fattree.print_header_mimic("latency_per_flow_" + suffix + "_finR.csv");
            // fattree.print_header_mimic("latency_per_flow_" + suffix + "_finC.csv");
            fattree.print_header_all_mimic("../runs/" + tag + "/reports_ana/latency_per_flow.csv");
            fattree.print_queue_header("../runs/" + tag + "/reports_ana/latency_per_queue.csv");

            // TODO: Why do we need this? Check the timestamp pointers
            // TODO: What timing is this?
            end = std::chrono::high_resolution_clock::now();
            duration_sec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
            std::ofstream of_time;
            of_time.open("timing.csv", std::ios::out | std::ios::app);
            of_time << suffix << "," << 0 << "," << duration_sec.count() << endl;
            of_time.close();

            for (auto &timestamp : time_list)
            {
                start = std::chrono::high_resolution_clock::now();
                // TODO: Change this to log based display statements
                cout << "[ INFO] Processing time stamp: " << timestamp << endl;

                fattree.reset_flow();
                fattree.clear_queue_vector();
                fattree.load_traffic_file_mimic(dataratefile + std::to_string(timestamp) + ".txt");
                // fattree.update_flow_datarate(datarate, 0.0);
                // fattree.calc_CS_sqr_all(linkBandwidth, packet_size, packet_header, bits, min_packet_size, max_packet_size);
                fattree.calc_lambda_CA_sqr_stage_0_mimic();
                fattree.calc_stage_0_mimic();
                fattree.calc_stage_1_mimic();
                fattree.calc_stage_2_mimic();
                fattree.calc_stage_3_mimic();
                fattree.calc_stage_4_mimic();
                fattree.calc_stage_5_mimic();

				fattree.regression_tree_model();

                fattree.calc_flowSplit_latency_v2();
                fattree.calc_flow_latency_v2();

                // fattree.print_latency_inf_mimic("latency_per_flow_" + suffix + "_inf.csv", timestamp);
                // fattree.print_latency_finR_mimic("latency_per_flow_" + suffix + "_finR.csv", timestamp);
                // fattree.print_latency_finC_mimic("latency_per_flow_" + suffix + "_finC.csv", timestamp);
                // fattree.print_latency_all_mimic("latency_per_flow_" + suffix + ".csv", timestamp);
                // fattree.print_queue_info("queue_" + suffix + ".csv", timestamp);
                fattree.print_latency_all_mimic("../runs/" + tag + "/reports_ana/latency_per_flow.csv", timestamp);
                fattree.print_queue_info("../runs/" + tag + "/reports_ana/latency_per_queue.csv", timestamp);

                end = std::chrono::high_resolution_clock::now();
                duration_sec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
                of_time.open("timing.csv", std::ios::out | std::ios::app);
                of_time << suffix << "," << timestamp << "," << duration_sec.count() << endl;
                of_time.close();
            }
		}
		else {
			FattreeL2custom fattree;
			suffix = suffix + "_pkt" + to_string(packet_size) + "_delta" + to_string(delta) + "_q" + to_string(queueSize);
			
			fattree.create(switchRadix, numNodes, linkBandwidth, datarate_unit);
			fattree.link();
			fattree.route();
			// fattree.printRoutingTable("routing.csv");
			// fattree.printQueues("queue.csv");
			
			fattree.mapping_globalFlows();
			fattree.mapping_globalFlowSplits();
			
			fattree.load_traffic_file(dataratefile);
			fattree.print_header("latency_per_flow_" + suffix + "_inf.csv");
			fattree.print_header("latency_per_flow_" + suffix + "_finR.csv");
			fattree.print_header("latency_per_flow_" + suffix + "_finC.csv");
			
			end = std::chrono::high_resolution_clock::now();
			duration_sec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
			std::ofstream of_time;
			of_time.open("timing.csv", std::ios::out | std::ios::app);
			of_time << suffix << "," << 0 << "," << duration_sec.count() << endl;
			of_time.close();
			
			for (auto &datarate : datarate_list)
			{
				start = std::chrono::high_resolution_clock::now();
				cout << "data rate = " << datarate << "\n";
				
				fattree.clear_queue_vector();
				fattree.update_flow_datarate(datarate, 0.0);
				fattree.calc_CS_sqr_all(linkBandwidth, packet_size, packet_header, bits, min_packet_size, max_packet_size);
				fattree.calc_lambda_CA_sqr_stage_0();
				fattree.calc_stage_0();
				fattree.calc_stage_1();
				fattree.calc_stage_2();
				fattree.calc_stage_3();
				fattree.calc_stage_4();
				fattree.calc_stage_5();
				
				fattree.calc_flowSplit_latency();
				fattree.calc_flow_latency();
				
				fattree.print_latency_inf("latency_per_flow_" + suffix + "_inf.csv");
				fattree.print_latency_finR("latency_per_flow_" + suffix + "_finR.csv");
				fattree.print_latency_finC("latency_per_flow_" + suffix + "_finC.csv");
				
				end = std::chrono::high_resolution_clock::now();
				duration_sec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
				of_time.open("timing.csv", std::ios::out | std::ios::app);
				of_time << suffix << "," << datarate << "," << duration_sec.count() << endl;
				of_time.close();
			}
		}
	}
	
	return 0;
}
