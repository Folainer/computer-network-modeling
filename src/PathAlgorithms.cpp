#include "PathAlgorithms.hpp"

pair<vector<long long>, vector<int>> PathAlgorithm::dijkstra(const Graph& g, int s, const vector<bool> &disableEdge)
{
    int n = g.n;

    vector<long long> dist(n, INF);
    vector<int> parentEdge(n, -1);
    dist[s] = 0;

    typedef pair<long long, int> CostIDPair;
    priority_queue<CostIDPair, vector<CostIDPair>, greater<>> pq;
    pq.push({0, s});

    while (!pq.empty())
    {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue;

        for (auto &e : g.adj[u])
        {
            if (e.id >= 0 && e.id < (int)disableEdge.size() && disableEdge[e.id]) continue;

            int v = e.to;
            long long nd = d + e.weight.calculate();

            if (nd < dist[v])
            {
                dist[v] = nd;
                parentEdge[v] = e.id;
                pq.push({nd, v});
            }
        }
    }

    return {dist, parentEdge}; 
}

vector<int> PathAlgorithm::reconstruct_nodes_from_parentEdges(int t, const vector<int> &parentEdge, const vector<pair<int, int>> &edgeEndpoints)
{
    vector<int> nodes;
    int cur = t;
    if (parentEdge[cur] == -1)
    {
        nodes.push_back(cur);
        return nodes;
    }

    while (true)
    {
        nodes.push_back(cur);
        int eid = parentEdge[cur];
        if (eid == -1) break;
        int u = edgeEndpoints[eid].first;

        if (u == cur)
        {
            break;
        }

        cur = u;
        if (cur == -1) break;
    }
    reverse(nodes.begin(), nodes.end());
    return nodes;
}