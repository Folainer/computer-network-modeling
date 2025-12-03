#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <vector>
#include <iostream>
#include <map>
#include <cmath>
#include <queue>

#include "Packet.hpp"

#define INF LLONG_MAX

using namespace std;

extern int MTU;
extern int ROUTER_BUFFER_SIZE;
extern int header_size;
extern int TTL;

enum ChannelType {DUPLEX, HALF_DUPLEX};

// Forward declarations for types defined elsewhere
class Graph;
class PathAlgorithm;

struct Weight {
    double latency_ms;
    double bandwidth_mbps;

    Weight(double latency, double bandwidth);
    double calculate() const;
};

struct Edge {
    int to;
    int id;
    Weight weight;
    ChannelType type;
    double p_error;

    queue<Packet> buffer;
    

    Edge(int t, Weight w, ChannelType ct, double pe);

private:
    static int nextId;
};

struct Node {
    bool isSatellite;
    vector<long long> distTable;
    vector<int> parentTable;
    queue<Packet> buffer;

    Node();
    Node(bool isSatellite);
    void fillTable(Graph& g, int id);
    vector<int> findReservedPath(int source, int target, Graph& g);
    bool isCalculated() const;
};

class Graph {
public:
    int n;
    map<int, Node> nodes;
    vector<vector<Edge>> adj;
    vector<pair<int,int>> edgeEndpoints;

    Graph(int nonSatelliteNodeCount, int satelliteCount);
    void addNonDirectedEdge(int u, int v, Weight weight, ChannelType ct, double p_error);
    void output(ostream& stream);
};

class Random {
public:
    Random(int initValue);
    double getValue();

private:
    double _currentValue;
};

#endif