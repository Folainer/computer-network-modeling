#include "PathAlgorithms.hpp"
#include "Global.hpp"
#include <limits>

pair<vector<long long>, vector<int>> PathAlgorithm::dijkstra(
    const Graph& g, int source, const vector<bool>& disableEdge) {
    
    int n = g.n;
    vector<long long> dist(n, INF);
    vector<int> parentEdge(n, -1);
    
    if (source < 0 || source >= n) {
        return {dist, parentEdge};
    }
    
    dist[source] = 0;

    typedef pair<long long, int> CostNodePair;
    priority_queue<CostNodePair, vector<CostNodePair>, greater<CostNodePair>> pq;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        
        if (d > dist[u]) continue;

        for (const auto& e : g.adj[u]) {
            // Check if edge is disabled
            if (e.id >= 0 && e.id < (int)disableEdge.size() && disableEdge[e.id]) {
                continue;
            }

            int v = e.to;
            long long weight = e.weight.calculate();
            
            // Handle INF weight
            if (weight >= INF || d >= INF) continue;
            
            long long nd = d + weight;
            
            // Check for overflow
            if (nd < d) continue;  // Overflow occurred

            if (nd < dist[v]) {
                dist[v] = nd;
                parentEdge[v] = e.id;
                pq.push({nd, v});
            }
        }
    }

    return {dist, parentEdge};
}

vector<int> PathAlgorithm::reconstruct_nodes_from_parentEdges(
    int target, const vector<int>& parentEdge, 
    const vector<pair<int, int>>& edgeEndpoints) {
    
    vector<int> nodes;
    
    if (target < 0 || target >= (int)parentEdge.size()) {
        return nodes;
    }
    
    int cur = target;
    nodes.push_back(cur);
    
    // If no parent, this is the source itself
    if (parentEdge[cur] == -1) {
        return nodes;
    }

    // Trace back through parent edges
    int maxSteps = parentEdge.size();  // Prevent infinite loops
    int steps = 0;
    
    while (parentEdge[cur] != -1 && steps < maxSteps) {
        int eid = parentEdge[cur];
        
        if (eid < 0 || eid >= (int)edgeEndpoints.size()) {
            break;
        }
        
        int parent = edgeEndpoints[eid].first;
        
        if (parent == cur) {
            break; // Avoid infinite loop
        }
        
        nodes.push_back(parent);
        cur = parent;
        steps++;
    }
    
    reverse(nodes.begin(), nodes.end());
    return nodes;
}

vector<int> PathAlgorithm::reconstruct_edges_from_parentEdges(
    int target, const vector<int>& parentEdge,
    const vector<pair<int, int>>& edgeEndpoints) {
    
    vector<int> edges;
    
    if (target < 0 || target >= (int)parentEdge.size()) {
        return edges;
    }
    
    int cur = target;
    
    // If no parent, this is the source itself
    if (parentEdge[cur] == -1) {
        return edges;
    }

    // Collect edge IDs
    int maxSteps = parentEdge.size();  // Prevent infinite loops
    int steps = 0;
    
    while (parentEdge[cur] != -1 && steps < maxSteps) {
        int eid = parentEdge[cur];
        
        if (eid < 0 || eid >= (int)edgeEndpoints.size()) {
            break;
        }
        
        edges.push_back(eid);
        
        int parent = edgeEndpoints[eid].first;
        
        if (parent == cur) {
            break; // Avoid infinite loop
        }
        
        cur = parent;
        steps++;
    }
    
    reverse(edges.begin(), edges.end());
    return edges;
}