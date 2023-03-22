#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <map>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <numeric>
#include "util.h"

#define DEBUG 0

using namespace std;

void Queue::clear()
{
    v_flowId.clear();
    v_flowSplitId.clear();

    v_flowIdV2.clear();
    v_flowSplitIdV2.clear();

    v_datarate.clear();
    v_lambda.clear();
    v_CA_sqr.clear();
    v_rho.clear();
    v_CS_sqr.clear();
    v_packet_size.clear();

    v_queue_latency_inf.clear();
    v_queue_latency_finR.clear();
    v_queue_latency_finC.clear();
    v_queue_occupancy_inf.clear();
    v_queue_occupancy_finR.clear();
    v_queue_occupancy_finC.clear();

    v_link_latency.clear();

    v_queue_correction_inf.clear();
}

// This function computes the square of service time coeffient of variation
// for a given link bandwidth and packet.
void Queue::calc_CS_sqr(double link_bandwidth, double packet_size, int packet_header, int bits, double min_packet_size, double max_packet_size)
{
    //es::toremove
	std::cout << link_bandwidth << std::endl;
    this->linkBandwidth = link_bandwidth;
    this->packet_size = packet_size;
    this->packet_header = packet_header;
    this->bits = bits;
    this->min_packet_size = min_packet_size;
    this->max_packet_size = max_packet_size;

    // Compute the channel latency in TODO: PLEASE PUT THE UNIT 
    // TODO: WHY MULTIPLY WITH 1000?
    this->channel_latency = ((packet_size + packet_header) * bits / link_bandwidth) * 1000;

    this->mu = (link_bandwidth) / ((packet_size + packet_header) * bits);
    this->t_serv = 1 / mu;

    //es::toremove
	std::cout << "link_bandwidth= " << link_bandwidth << std::endl;
	std::cout << "channel_latency= " << channel_latency << std::endl;


    double service_time = 1 / mu;
    double sumSecondMoment = 0.0;
    double sumMean = 0.0;

    for (double pktsize = min_packet_size; pktsize <= max_packet_size; pktsize++)
    {
        service_time = ((pktsize + packet_header) * bits / link_bandwidth);
        sumMean = sumMean + service_time;
        sumSecondMoment = sumSecondMoment + pow(service_time, 2);
    }

    double secondMoment = sumSecondMoment / (max_packet_size - min_packet_size + 1);
    double meanS = sumMean / (max_packet_size - min_packet_size + 1);
    this->CS_sqr = (secondMoment - pow(meanS, 2)) / pow(meanS, 2);
}

