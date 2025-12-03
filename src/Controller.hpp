#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

#include "Global.hpp"

using namespace std;

class Controller {
    public:
    Controller(int nonSatelliteNodeCount, int satelliteCount);
    void run();

    vector<int> findPath(int from, int to) const;

    private:
    Graph _graph;

    void help() const;
    void initGraph();
    void displayGraphWeights() const;
    void displayFullGraphWeights() const;
    void displayNodeDistance(int id) const;
    void displayPath(int from, int to) const;
    void displayPathTable(int from) const;
    void displayBackDistance(int from, int to) const;
    void displayBackDistanceTable(int from) const;
};

#endif