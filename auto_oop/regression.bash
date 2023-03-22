#!/bin/bash

echo "==== comparing golden_routing_L3_k4.csv  wtih routing.csv ====="
sdiff -s golden_routing_L3_k4.csv routing.csv

echo " "
echo "==== comparing golden_routing_L3_k4.csv  wtih routing.csv ===="
sdiff -s golden_queue_L3_k4.csv queue.csv

echo " "
echo "done"
