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
            // Перевірити якщо ребро вимкнуте
            if (e.id >= 0 && e.id < (int)disableEdge.size() && disableEdge[e.id]) {
                continue;
            }

            int v = e.to;
            long long weight = e.weight.calculate();
            
            // Опрацювати безкінечну вагу
            if (weight >= INF || d >= INF) continue;
            
            long long nd = d + weight;
            
            // Перевірити на переповнення
            if (nd < d) continue;  

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
    
    // Якщо не має предка, це є джерелом
    if (parentEdge[cur] == -1) {
        return nodes;
    }

    // Повернитися назад через батківські ребра
    int maxSteps = parentEdge.size(); 
    int steps = 0;
    
    while (parentEdge[cur] != -1 && steps < maxSteps) {
        int eid = parentEdge[cur];
        
        if (eid < 0 || eid >= (int)edgeEndpoints.size()) {
            break;
        }
        
        int parent = edgeEndpoints[eid].first;
        
        if (parent == cur) {
            break; // Уникнути безкінечного циклу
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
    
    // Якщо не має предка, то це є саме джерелом
    if (parentEdge[cur] == -1) {
        return edges;
    }

    // Зібрати ідентифікатори ребер
    int maxSteps = parentEdge.size();  // Уникнути безкінечного циклу
    int steps = 0;
    
    while (parentEdge[cur] != -1 && steps < maxSteps) {
        int eid = parentEdge[cur];
        
        if (eid < 0 || eid >= (int)edgeEndpoints.size()) {
            break;
        }
        
        edges.push_back(eid);
        
        int parent = edgeEndpoints[eid].first;
        
        if (parent == cur) {
            break; // Уникнути безкінечного циклу
        }
        
        cur = parent;
        steps++;
    }
    
    reverse(edges.begin(), edges.end());
    return edges;
}