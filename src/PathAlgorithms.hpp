#ifndef PATHALGORITHMS_HPP
#define PATHALGORITHMS_HPP

#include <vector>
#include <queue>
#include <algorithm>

#include "Global.hpp"

class Graph;

using namespace std;

class PathAlgorithm
{
    public:
    static pair<vector<long long>, vector<int>> dijkstra(const Graph& g, int s, const vector<bool> &disableEdge);
    static vector<int> reconstruct_nodes_from_parentEdges(int t, const vector<int> &parentEdge, const vector<pair<int, int>> &edgeEndpoints);
    static vector<int> reconstruct_edges_from_parentEdges(int target, const vector<int>& parentEdge, const vector<pair<int,int>>& edgeEndpoints);
};

#endif