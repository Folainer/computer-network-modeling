#include "Global.hpp"
#include "PathAlgorithms.hpp"
#include "Packet.hpp"

// Global variables
int MTU = 1500;
int ROUTER_BUFFER_SIZE = 10;
int header_size = 50;
int TTL = 200;

// Weight implementation
Weight::Weight(double latency, double bandwidth)
    : latency_ms(latency), bandwidth_mbps(bandwidth) {}

double Weight::calculate() const {
    if (bandwidth_mbps <= 0) return INF;
    return round(10.0 * latency_ms / bandwidth_mbps + 1.0);
}

// Edge implementation
int Edge::nextId = 0;

Edge::Edge(int to, const Weight& w, ChannelType ct, double pe)
    : to(to), id(nextId++), weight(w), type(ct), p_error(pe) {}

// Node implementation
Node::Node(bool isSatellite)
    : isSatellite(isSatellite) {}

void Node::fillTable(const Graph& g, int id) {
    vector<bool> disabled(g.edgeEndpoints.size(), false);
    auto [dist, parentEdge] = PathAlgorithm::dijkstra(g, id, disabled);
    distTable = move(dist);
    parentTable = move(parentEdge);
}

vector<int> Node::findReservedPath(int source, int target, const Graph& g) {
    vector<bool> disabled(g.edgeEndpoints.size(), false);

    // Calculate primary path if not already done
    if (!isCalculated()) {
        // Need to modify this node's tables, but we're in a const method context
        // We need to cast away constness here since we're modifying cache
        Node* mutableThis = const_cast<Node*>(this);
        mutableThis->fillTable(g, source);
    }

    // Get primary path and disable its edges
    // Reconstruct the path from source to target
    int cur = target;
    while (cur != source && parentTable[cur] != -1) {
        int eid = parentTable[cur];
        if (eid >= 0 && eid < (int)disabled.size()) {
            disabled[eid] = true;
        }
        
        if (eid >= 0 && eid < (int)g.edgeEndpoints.size()) {
            cur = g.edgeEndpoints[eid].first;
        } else {
            break;
        }
    }

    // Find backup path with primary path disabled
    auto [dist, parentEdge] = PathAlgorithm::dijkstra(g, source, disabled);

    if (dist.empty() || target >= (int)dist.size() || dist[target] == INF) {
        return {};
    }

    return PathAlgorithm::reconstruct_nodes_from_parentEdges(
        target, parentEdge, g.edgeEndpoints);
}

bool Node::isCalculated() const {
    return !distTable.empty();
}

// Graph implementation
Graph::Graph(int nonSatelliteNodeCount, int satelliteCount)
    : n(nonSatelliteNodeCount + satelliteCount), adj(n) {
    
    // Create satellite nodes (indices 0 to satelliteCount-1)
    for (int i = 0; i < satelliteCount; i++) {
        nodes.emplace(i, Node(true));
    }
    
    // Create non-satellite nodes (indices satelliteCount to n-1)
    for (int i = 0; i < nonSatelliteNodeCount; i++) {
        nodes.emplace(satelliteCount + i, Node(false));
    }
}

void Graph::addNonDirectedEdge(int u, int v, const Weight& weight, 
                                ChannelType ct, double p_error) {
    if (u < 0 || u >= n || v < 0 || v >= n) {
        cerr << "Error: Invalid node indices " << u << " -> " << v << endl;
        return;
    }

    // Add edge u -> v
    adj[u].emplace_back(v, weight, ct, p_error);
    edgeEndpoints.push_back({u, v});
    
    // Add edge v -> u (undirected)
    adj[v].emplace_back(u, weight, ct, p_error);
    edgeEndpoints.push_back({v, u});
}

void Graph::output(ostream& stream) const {
    for (size_t i = 0; i < adj.size(); i++) {
        vector<int> outputLine(adj.size(), 0);
        
        for (const auto& edge : adj[i]) {
            if (edge.to >= 0 && edge.to < (int)adj.size()) {
                outputLine[edge.to] = 1;
            }
        }

        for (int val : outputLine) {
            stream << val << ' ';
        }
        stream << '\n';
    }
}

Node* Graph::getNode(int id) {
    auto it = nodes.find(id);
    return (it != nodes.end()) ? &it->second : nullptr;
}

const Node* Graph::getNode(int id) const {
    auto it = nodes.find(id);
    return (it != nodes.end()) ? &it->second : nullptr;
}

Edge* Graph::findEdge(int from, int to) {
    if (from < 0 || from >= n) return nullptr;
    
    for (auto& edge : adj[from]) {
        if (edge.to == to) {
            return &edge;
        }
    }
    return nullptr;
}

const Edge* Graph::findEdge(int from, int to) const {
    if (from < 0 || from >= n) return nullptr;
    
    for (const auto& edge : adj[from]) {
        if (edge.to == to) {
            return &edge;
        }
    }
    return nullptr;
}

// Random implementation
Random::Random(double initValue) : _currentValue(initValue) {
    if (_currentValue < 0.0) _currentValue = 0.0;
    else if (_currentValue > 1.0) _currentValue = 1.0;
}

double Random::getValue() {
    if (_currentValue == 0.0) {
        _currentValue = 0.01;
    }

    // Logistic map: x_{n+1} = r * x_n * (1 - x_n), where r = 4
    _currentValue = _currentValue * (1.0 - _currentValue) * 4.0;

    if (_currentValue < 0.0) _currentValue = 0.0;
    else if (_currentValue > 1.0) _currentValue = 1.0;

    return _currentValue;
}