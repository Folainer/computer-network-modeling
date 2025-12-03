#include "Global.hpp"
#include "PathAlgorithms.hpp"
#include "Simulation.hpp"

int MTU = 1500;
int ROUTER_BUFFER_SIZE = 10; 
int header_size = 50;
int TTL = 20;

double Weight::calculate() const
{
    return round(10* latency_ms / bandwidth_mbps + 1);
}

int Edge::nextId = 0;

Edge::Edge(int t, Weight w, ChannelType ct, double pe) 
    : to(t), id(nextId++), weight(w), type(ct), p_error(pe) {}

Node::Node(bool isSatellite)
    : isSatellite(isSatellite) {}

Node::Node()
    : isSatellite(false) {}


void Node::fillTable(Graph& g, int id)
{
    auto [dist, parentEdge] = PathAlgorithm::dijkstra(g, id, vector<bool> (g.nodes.size()));
    distTable = move(dist);
    parentTable = move(parentEdge);
}

vector<int> Node::findReservedPath(int source, int target, Graph& g)
{
    vector<bool> disabled(g.edgeEndpoints.size(), false);

    if (!isCalculated())
    {
        fillTable(g, target);
    }

    auto path = PathAlgorithm::reconstruct_nodes_from_parentEdges(target, parentTable, g.edgeEndpoints);

    for (int eid : path)
    {
        disabled[eid] = true;
    }

    auto [dist, parentEdge] = PathAlgorithm::dijkstra(g, source, disabled);

    if (dist[target] == INF)
    {
        return {};
    }

    return PathAlgorithm::reconstruct_nodes_from_parentEdges(target, parentEdge, g.edgeEndpoints);
}

bool Node::isCalculated() const
{
    return distTable.size() > 0;
}


Graph::Graph(int nonSatelliteNodeCount, int satelliteCount)
    : n(nonSatelliteNodeCount + satelliteCount), adj(n)
{
    int index = 0;
    for (int i = 0; i < satelliteCount; i++)
    {
        nodes.emplace(i, Node(true));
        index++;
    }
    for (int i = 0; i < nonSatelliteNodeCount; i++)
    {
        nodes.emplace(satelliteCount + i, Node(false));
        index++;
    }
}

void Graph::addNonDirectedEdge(int u, int v, Weight weight, ChannelType ct, double p_error)
{
    adj[u].emplace_back(v, weight, ct, p_error);
    edgeEndpoints.push_back({u, v});
    adj[v].emplace_back(u, weight, ct, p_error);
    edgeEndpoints.push_back({v, u});
}

void Graph::output(ostream& stream)
{
    for (auto& vec : adj)
    {
        vector<int> outputLine(adj.size(), 0);
        
        for (auto& item : vec)
        {
            outputLine[item.to] = 1;
        }

        for (size_t i = 0; i < outputLine.size(); i++)
        {
            stream << outputLine[i] << ' ';
        }

        stream << '\n';
    }
}

Random::Random(int initValue) : _currentValue(initValue)
{
    if (_currentValue < 0) _currentValue = 0;
    else if (_currentValue > 1) _currentValue = 1;
}

double Random::getValue()
{
    if (_currentValue == 0) {
        _currentValue = 0.01;
    }

    _currentValue = _currentValue * (1 - _currentValue) * 4;

    if (_currentValue < 0) _currentValue = 0;
    else if (_currentValue > 1) _currentValue = 1;

    return _currentValue;
}

