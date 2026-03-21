//
// Created by ASUS on 3/20/2026.
//

/**
 * @file Graph.h
 * @brief Graph data structure based on the TP1 lectures template.
 *
 * We use the same Graph<T> structure shown in TP1 classes.
 * The only additions we made are for the Max-Flow algorithm:
 *   - Edge now has capacity, flow and reverseIndex fields
 *   - Graph has addFlowEdge() to add forward + reverse edges together0

 *   - Graph has resetFlow() to clear flow between runs
 *
 * Everything else (Vertex fields, addVertex, findVertex, etc.)
 * is exactly the same as in TP1.
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

// We need these forward declarations because Vertex uses Edge and vice versa
template <class T> class Edge;
template <class T> class Graph;


// ============================================================
// VERTEX
// ============================================================

/**
 * @brief A node in the graph.
 *
 * Same as TP1 - stores the node value and a list of edges going out.
 * The visited/processing/indegree fields are used in BFS and DFS,
 * just like we saw in the TP1 exercises.
 *
 * @tparam T Type of the value stored in this node
 */
template <class T>
class Vertex {
public:
    T info;                    ///< The value stored in this node
    std::vector<Edge<T>> adj;  ///< Edges going out from this node

    // These fields are used during graph traversals (BFS, DFS)
    // They are the same fields from TP1
    bool visited    = false;   ///< Used in BFS - was this node visited?
    bool processing = false;   ///< Used in DFS - is this node being processed?
    int  indegree   = 0;       ///< Number of edges coming IN (used in topological sort)

    // Used for Tarjan SCC algorithm (from TP1)
    int low = 0;
    int num = 0;

    /**
     * @brief Creates a vertex with the given value.
     * @param in The value to store
     */
    explicit Vertex(T in) : info(in) {}

    /**
     * @brief Adds an outgoing edge from this vertex to dest.
     * @param dest     The destination vertex
     * @param w        Weight / capacity of the edge
     * @param revIdx   Index of the reverse edge (needed for Max-Flow)
     * @complexity O(1)
     */
    void addEdge(Vertex<T>* dest, double w, int revIdx = -1);

    friend class Graph<T>;
};


// ============================================================
// EDGE
// ============================================================

/**
 * @brief An edge in the graph going from one vertex to another.
 *
 * In TP1 an edge only had dest and weight.
 * We added three new fields for the Max-Flow algorithm:
 *
 *   capacity     = how much flow this edge can carry (same as weight at start)
 *   flow         = how much flow is going through right now
 *   reverseIndex = where to find the reverse edge in dest->adj
 *
 * The reverse edge is required by Ford-Fulkerson / Edmonds-Karp.
 * When we send flow A->B, we need a reverse edge B->A so the
 * algorithm can cancel that flow later if a better path is found.
 *
 * @tparam T Type of vertex value
 */
template <class T>
class Edge {
public:
    Vertex<T>* dest;     ///< Where this edge goes
    double weight;       ///< Weight of the edge

    // Max-Flow fields (added by us, not in original TP1 Edge)
    double capacity;     ///< Max flow this edge can carry
    double flow;         ///< Current flow going through this edge
    int reverseIndex;    ///< Index of the reverse edge in dest->adj

    /**
     * @brief Creates an edge to dest with the given weight/capacity.
     * @param d   Destination vertex
     * @param w   Weight / capacity
     * @param rev Index of reverse edge in dest->adj (-1 if none)
     */
    Edge(Vertex<T>* d, double w, int rev = -1)
        : dest(d), weight(w), capacity(w), flow(0.0), reverseIndex(rev) {}

    /**
     * @brief How much more flow can go through this edge?
     * @return capacity - flow
     */
    double residual() const { return capacity - flow; }

    friend class Graph<T>;
    friend class Vertex<T>;
};


// ============================================================
// GRAPH
// ============================================================

/**
 * @brief Directed graph using adjacency lists.
 *
 * This is the main data structure for the project.
 * It is based on the Graph<T> from TP1 lectures.
 *
 * We use Graph<int> where each integer represents a node in the
 * flow network:
 *   0         = source (where flow starts)
 *   1..P      = one node per submission
 *   P+1..P+R  = one node per reviewer
 *   P+R+1     = sink (where flow ends)
 *
 * @tparam T Type of vertex value (we use int)
 */
template <class T>
class Graph {
public:
    std::vector<Vertex<T>*> vertexSet; ///< All vertices in the graph

    /**
     * @brief Destructor - frees all vertex memory.
     */
    ~Graph() {
        for (auto v : vertexSet)
            delete v;
    }

    /**
     * @brief Finds a vertex by its value.
     * @param in The value to search for
     * @return Pointer to the vertex, or nullptr if not found
     * @complexity O(V)
     */
    Vertex<T>* findVertex(const T& in) const {
        for (auto v : vertexSet)
            if (v->info == in) return v;
        return nullptr;
    }

    /**
     * @brief Returns the number of vertices in the graph.
     */
    int getNumVertex() const {
        return (int)vertexSet.size();
    }

    /**
     * @brief Adds a new vertex with the given value.
     * Does nothing if a vertex with this value already exists.
     * @param in The value for the new vertex
     * @return true if added, false if it already existed
     * @complexity O(V)
     */
    bool addVertex(const T& in) {
        if (findVertex(in) != nullptr) return false;
        vertexSet.push_back(new Vertex<T>(in));
        return true;
    }

    /**
     * @brief Adds a directed edge from src to dest.
     * This is the original TP1 method - no reverse edge.
     * @param src    Source vertex value
     * @param dest   Destination vertex value
     * @param weight Edge weight
     * @return true if added, false if either vertex does not exist
     * @complexity O(V)
     */
    bool addEdge(const T& src, const T& dest, double weight) {
        auto v1 = findVertex(src);
        auto v2 = findVertex(dest);
        if (v1 == nullptr || v2 == nullptr) return false;
        v1->addEdge(v2, weight);
        return true;
    }

    /**
     * @brief Adds a flow edge (forward) AND its reverse residual edge.
     *
     * This is our addition to the TP1 Graph for Max-Flow.
     *
     * For every real edge src->dest with capacity w, we also create
     * a reverse edge dest->src with capacity 0.
     * The reverse edge lets the algorithm "undo" flow later.
     *
     * Both edges store where the other one is (reverseIndex) so
     * updating them during the algorithm is O(1).
     *
     * @param src    Source vertex value
     * @param dest   Destination vertex value
     * @param weight Capacity of the forward edge
     * @return true if added successfully
     * @complexity O(V)
     */
    bool addFlowEdge(const T& src, const T& dest, double weight) {
        auto v1 = findVertex(src);
        auto v2 = findVertex(dest);
        if (v1 == nullptr || v2 == nullptr) return false;

        // The reverse edge will be at position [v2->adj.size()] in v2->adj
        // and the forward edge at [v1->adj.size()] in v1->adj
        int revIdx = (int)v2->adj.size();
        int fwdIdx = (int)v1->adj.size();

        v1->adj.emplace_back(v2, weight, revIdx); // forward: full capacity
        v2->adj.emplace_back(v1, 0.0,    fwdIdx); // reverse: starts at 0

        return true;
    }

    /**
     * @brief Resets visited and processing flags on all vertices.
     * We call this before each BFS or DFS, same as in TP1.
     * @complexity O(V)
     */
    void resetVisited() {
        for (auto v : vertexSet) {
            v->visited    = false;
            v->processing = false;
        }
    }

    /**
     * @brief Resets all edge flows to zero (keeps capacities).
     * We use this before re-running the flow algorithm (e.g. in risk analysis).
     * @complexity O(V + E)
     */
    void resetFlow() {
        for (auto v : vertexSet)
            for (auto& e : v->adj)
                e.flow = 0.0;
    }
};


// ============================================================
// Vertex::addEdge implementation
// Has to be here (not in .cpp) because it's a template
// ============================================================

template <class T>
void Vertex<T>::addEdge(Vertex<T>* dest, double w, int revIdx) {
    adj.emplace_back(dest, w, revIdx);
}

#endif // GRAPH_H
