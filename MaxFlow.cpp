//
// Created by ASUS on 3/22/2026.
//


#include "MaxFlow.h"
#include <queue>
#include <algorithm>

// Same as TP3: if the vertex w is not visited and there is residual capacity, visit it
template <class T>
void testAndVisit(std::queue<Vertex<T>*>& q, Edge<T>* e, Vertex<T>* w, double residual) {
    if (!w->isVisited() && residual > 0) {
        w->setVisited(true);
        w->setPath(e);
        q.push(w);
    }
}

// BFS to find an augmenting path from source to sink
// We check outgoing edges (forward) and incoming edges (backward)
// Same idea as TP3 findAugmentingPath
bool bfs(Graph<int>& g, int source, int sink) {
    for (auto v : g.getVertexSet())
        v->setVisited(false);

    auto s = g.findVertex(source);
    auto t = g.findVertex(sink);
    s->setVisited(true);

    std::queue<Vertex<int>*> q;
    q.push(s);

    while (!q.empty() && !t->isVisited()) {
        auto v = q.front();
        q.pop();

        // forward edges: residual = weight - flow
        for (auto e : v->getAdj())
            testAndVisit(q, e, e->getDest(), e->getWeight() - e->getFlow());

        // backward edges: residual = flow
        for (auto e : v->getIncoming())
            testAndVisit(q, e, e->getOrig(), e->getFlow());
    }

    return t->isVisited();
}

// Walk from sink to source along the path, find the smallest residual
// Same as TP3 findMinResidualAlongPath
static double findMinResidualAlongPath(Vertex<int>* s, Vertex<int>* t) {
    double f = 1e18;
    for (auto v = t; v != s; ) {
        auto e = v->getPath();
        if (e->getDest() == v) {
            f = std::min(f, e->getWeight() - e->getFlow());
            v = e->getOrig();
        } else {
            f = std::min(f, e->getFlow());
            v = e->getDest();
        }
    }
    return f;
}

// Push flow along the path
// Same as TP3 augmentFlowAlongPath
static void augmentFlowAlongPath(Vertex<int>* s, Vertex<int>* t, double f) {
    for (auto v = t; v != s; ) {
        auto e = v->getPath();
        double flow = e->getFlow();
        if (e->getDest() == v) {
            e->setFlow(flow + f);
            v = e->getOrig();
        } else {
            e->setFlow(flow - f);
            v = e->getDest();
        }
    }
}

// Edmonds-Karp: keep finding augmenting paths with BFS and pushing flow
// Same structure as TP3 edmondsKarp
// O(V * E^2)
double edmondsKarp(Graph<int>& g, int source, int sink) {
    // set all flows to 0
    for (auto v : g.getVertexSet())
        for (auto e : v->getAdj())
            e->setFlow(0);

    auto s = g.findVertex(source);
    auto t = g.findVertex(sink);
    double maxFlow = 0;

    while (bfs(g, source, sink)) {
        double f = findMinResidualAlongPath(s, t);
        augmentFlowAlongPath(s, t, f);
        maxFlow += f;
    }

    return maxFlow;
}
