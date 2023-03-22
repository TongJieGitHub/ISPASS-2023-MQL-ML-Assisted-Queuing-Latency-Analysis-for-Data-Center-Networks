#ifndef FATTREE_L2custom_H
#define FATTREE_L2custom_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <cmath>
#include "fattree.h"

using namespace std;

class FattreeL2custom : public Fattree
{
public:
	int No_of_links_per_core_edge_pair;

	void create(int K, int numNodes, double linkBandwidth, double datarate_unit);
    void link();
    void route();     // build all-to-all routing table - using ecmp

    // added for fattreeL2custom
	// void printFlowSplits

};

#endif //FATTREE_L2custom_H
