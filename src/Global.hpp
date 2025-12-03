#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <vector>
#include <iostream>
#include <map>
#include <cmath>
#include <queue>
#include <memory>
#include <limits>

#define INF std::numeric_limits<long long>::max()

using namespace std;

// Global configuration
extern int MTU;
extern int ROUTER_BUFFER_SIZE;
extern int header_size;
extern int TTL;

enum ChannelType { DUPLEX, HALF_DUPLEX };
enum TransactionType { DATAGRAM, VIRTUAL_CHANNEL };

// Forward declarations
class Graph;
class Node;
class Edge;
class Packet;

// Weight structure for edges
struct Weight {
    double latency_ms;
    double bandwidth_mbps;

    Weight(double latency = 0.0, double bandwidth = 0.0);
    double calculate() const;
};

// Edge in the graph
class Edge {
public:
    int to;
    int id;
    Weight weight;
    ChannelType type;
    double p_error;
    queue<shared_ptr<Packet>> buffer;

    Edge(int to, const Weight& w, ChannelType ct, double pe);

private:
    static int nextId;
};

// Node in the graph
class Node {
public:
    bool isSatellite;
    vector<long long> distTable;
    vector<int> parentTable;
    queue<shared_ptr<Packet>> buffer;

    Node(bool isSatellite = false);
    
    void fillTable(const Graph& g, int id);
    vector<int> findReservedPath(int source, int target, const Graph& g);
    bool isCalculated() const;
};

// Graph structure
class Graph {
public:
    int n;
    map<int, Node> nodes;
    vector<vector<Edge>> adj;
    vector<pair<int, int>> edgeEndpoints;

    Graph(int nonSatelliteNodeCount, int satelliteCount);
    void addNonDirectedEdge(int u, int v, const Weight& weight, ChannelType ct, double p_error);
    void output(ostream& stream) const;
    
    Node* getNode(int id);
    const Node* getNode(int id) const;
    Edge* findEdge(int from, int to);
    const Edge* findEdge(int from, int to) const;
};

// Random number generator
class Random {
public:
    Random(double initValue);
    double getValue();

private:
    double _currentValue;
};

#endif // GLOBAL_HPP