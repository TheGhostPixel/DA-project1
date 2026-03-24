//
// Created by ASUS on 3/22/2026.
//


#ifndef MAXFLOW_H
#define MAXFLOW_H

#include "Graph.h"

/// @brief BFS to find an augmenting path. Returns true if path found. O(V+E)
bool bfs(Graph<int>& g, int source, int sink);

/// @brief Runs Edmonds-Karp and returns the max flow value. O(V * E^2)
double edmondsKarp(Graph<int>& g, int source, int sink);

#endif //MAXFLOW_H
