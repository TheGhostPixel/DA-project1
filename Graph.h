//
// Created by ASUS on 3/20/2026.
//
//  Graph structure for the project


#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

template <class T> class Edge;
template <class T> class Graph;

/// @brief A vertex (node) in the graph. Same as TP, we just added path for BFS.
template <class T>
class Vertex {
public:
    explicit Vertex(T in) : info(in) {}

    /// @brief Get the value stored in this vertex.
    T getInfo() const { return info; }

    /// @brief Get outgoing edges.
    std::vector<Edge<T>*> getAdj() const { return adj; }

    /// @brief Get incoming edges (we need this for the residual graph in Edmonds-Karp).
    std::vector<Edge<T>*> getIncoming() const { return incoming; }

    /// @brief Was this vertex already visited in BFS?
    bool isVisited() const { return visited; }

    /// @brief Mark vertex as visited or not.
    void setVisited(bool v) { visited = v; }

    int getIndegree() const { return indegree; }
    void setIndegree(int i) { indegree = i; }

    /// @brief Which edge was used to reach this vertex in BFS. We need this to reconstruct the augmenting path.
    Edge<T>* getPath() const { return path; }
    void setPath(Edge<T>* p) { path = p; }

    friend class Graph<T>;

private:
    T info;
    std::vector<Edge<T>*> adj;       // outgoing edges
    std::vector<Edge<T>*> incoming;  // incoming edges
    bool visited = false;
    bool processing = false;
    int indegree = 0;
    int low = 0, num = 0;
    Edge<T>* path = nullptr;  // edge used to get here in BFS
};

/// @brief An edge in the graph. Same as TP but we added capacity and flow for Max-Flow.
template <class T>
class Edge {
public:
    Edge(Vertex<T>* orig, Vertex<T>* dest, double w)
        : orig(orig), dest(dest), weight(w), capacity(w), flow(0.0) {}

    /// @brief Where does this edge go?
    Vertex<T>* getDest() const { return dest; }

    /// @brief Where does this edge come from?
    Vertex<T>* getOrig() const { return orig; }

    /// @brief The capacity of this edge (same as weight at the start).
    double getWeight() const { return weight; }

    /// @brief How much flow is going through this edge right now.
    double getFlow() const { return flow; }

    /// @brief Set the flow on this edge.
    void setFlow(double f) { flow = f; }

    /// @brief Get the reverse edge. We need it to update the residual graph.
    Edge<T>* getReverse() const { return reverse; }
    void setReverse(Edge<T>* r) { reverse = r; }

    friend class Graph<T>;
    friend class Vertex<T>;

private:
    Vertex<T>* orig;
    Vertex<T>* dest;
    double weight;
    double capacity;
    double flow;
    Edge<T>* reverse = nullptr;
};

/// @brief The graph itself. Same as TP, we added addFlowEdge() and resetFlow().
template <class T>
class Graph {
public:
    /// @brief Get all vertices.
    std::vector<Vertex<T>*> getVertexSet() const { return vertexSet; }

    /// @brief Find a vertex by its value. Returns nullptr if not found. O(V)
    Vertex<T>* findVertex(const T& in) const {
        for (auto v : vertexSet)
            if (v->info == in) return v;
        return nullptr;
    }

    int getNumVertex() const { return (int)vertexSet.size(); }

    /// @brief Add a vertex. Does nothing if it already exists. O(V)
    bool addVertex(const T& in) {
        if (findVertex(in) != nullptr) return false;
        vertexSet.push_back(new Vertex<T>(in));
        return true;
    }

    /// @brief Add a normal edge (like in TP). O(V)
    bool addEdge(const T& src, const T& dest, double weight) {
        auto v1 = findVertex(src);
        auto v2 = findVertex(dest);
        if (!v1 || !v2) return false;
        auto e = new Edge<T>(v1, v2, weight);
        v1->adj.push_back(e);
        v2->incoming.push_back(e);
        return true;
    }

    /// @brief Add a flow edge + its reverse. The reverse starts with capacity 0. O(V)
    bool addFlowEdge(const T& src, const T& dest, double weight) {
        auto v1 = findVertex(src);
        auto v2 = findVertex(dest);
        if (!v1 || !v2) return false;

        auto fwd = new Edge<T>(v1, v2, weight);  // real edge
        auto rev = new Edge<T>(v2, v1, 0.0);     // reverse (capacity 0)
        fwd->setReverse(rev);
        rev->setReverse(fwd);

        v1->adj.push_back(fwd);
        v2->adj.push_back(rev);
        v2->incoming.push_back(fwd);
        v1->incoming.push_back(rev);
        return true;
    }

    /// @brief Reset visited flags for a new BFS. O(V)
    void resetVisited() {
        for (auto v : vertexSet) {
            v->visited = false;
            v->processing = false;
            v->path = nullptr;
        }
    }

    /// @brief Set all flows back to 0 (keeps capacities). O(V+E)
    void resetFlow() {
        for (auto v : vertexSet)
            for (auto e : v->adj)
                e->flow = 0.0;
    }

    ~Graph() {
        for (auto v : vertexSet) {
            for (auto e : v->adj) delete e;
            delete v;
        }
    }

private:
    std::vector<Vertex<T>*> vertexSet;
};

#endif // GRAPH_H

