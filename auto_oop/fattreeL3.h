#ifndef FATTREE_L3_H
#define FATTREE_L3_H


#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include "fattree.h"

using namespace std;

class FattreeL3 : public Fattree
{
public:

	void create(int K, double linkBandwidth, double datarate_unit);   // moved to derived class as the arugments passed are different between L3 and L2custom
  void link();    // different implementation between L3 and L2custom
  // build all-to-all routing table
  void route();   // different code between L3 and L2custom
};

#endif //FATTREE_L3_H
