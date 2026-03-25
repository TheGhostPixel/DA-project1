/**
 * @file MaxFlow.cpp
 * @brief Edmonds-Karp max-flow algorithm implementation.
 *
 * Uses BFS to find augmenting paths (Edmonds-Karp variant of Ford-Fulkerson).
 * The algorithm repeatedly finds shortest augmenting paths via BFS and pushes
 * flow along them until no more augmenting paths exist.
 *
 * @note Time complexity: O(V × E²)
 */

#include "MaxFlow.h"
#include <queue>
#include <algorithm>

/**
 * @brief Helper: test if vertex w can be visited along edge e.
 *
 * If w hasn't been visited yet AND there is residual capacity > 0,
 * mark it as visited, record the edge as its "path" edge, and enqueue it.
 *
 * @param q        The BFS queue.
 * @param e        The edge being tested.
 * @param w        The target vertex.
 * @param residual The remaining capacity on this edge (cap - flow, or flow for reverse).
 */
template <class T>
void testAndVisit(std::queue<Vertex<T>*>& q, Edge<T>* e, Vertex<T>* w, double residual) {
    if (!w->isVisited() && residual > 0) {
        w->setVisited(true);
        w->setPath(e);
        q.push(w);
    }
}

/**
 * @brief BFS to find an augmenting path from source to sink.
 *
 * Checks outgoing edges (forward: residual = capacity - flow) and
 * incoming edges (backward: residual = flow). Records the path
 * via each vertex's `path` pointer so we can reconstruct it later.
 *
 * @param g      The flow graph.
 * @param source Node ID of the source.
 * @param sink   Node ID of the sink.
 * @return true if an augmenting path was found, false otherwise.
 */
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

/**
 * @brief Walk from sink to source along the BFS path and find the bottleneck.
 *
 * The bottleneck is the smallest residual capacity along the path.
 *
 * @param s Pointer to the source vertex.
 * @param t Pointer to the sink vertex.
 * @return The minimum residual capacity on the path.
 */
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

/**
 * @brief Push flow `f` along the augmenting path from sink back to source.
 *
 * For forward edges, adds `f` to the flow. For backward edges, subtracts `f`.
 *
 * @param s Pointer to the source vertex.
 * @param t Pointer to the sink vertex.
 * @param f The amount of flow to push.
 */
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

/**
 * @brief Edmonds-Karp algorithm: finds maximum flow from source to sink.
 *
 * Repeatedly runs BFS to find augmenting paths (shortest in terms of edges),
 * then pushes the bottleneck flow along each path. Stops when no more
 * augmenting paths exist.
 *
 * @param g      The flow graph (edges must have been added with addFlowEdge).
 * @param source Node ID of the source.
 * @param sink   Node ID of the sink.
 * @return The total maximum flow value.
 *
 * @note Time complexity: O(V × E²)
 */
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
