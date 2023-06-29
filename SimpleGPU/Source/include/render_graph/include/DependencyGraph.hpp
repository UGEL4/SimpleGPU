#pragma once
#include <functional>

typedef uint64_t dep_graph_handle_t;

class DependencyGraph;
class DependencyGraphNode
{
public:
    friend class DependencyGraphImp;
    DependencyGraphNode() = default;
    const dep_graph_handle_t GetId() const { return mId; }
protected:
    DependencyGraph*   m_pGraph;
    dep_graph_handle_t mId;
};

class DependencyGraphEdge
{
public:
    friend class DependencyGraphImp;
    DependencyGraphEdge() = default;

    DependencyGraphNode* From();
    DependencyGraphNode* To();
protected:
    DependencyGraph*   m_pGraph;
    dep_graph_handle_t mFromNode;
    dep_graph_handle_t mToNode;
};


//insert : boost::add_vertex
//link : boost::add_edge
class DependencyGraph
{
public:
    using Node = DependencyGraphNode;
    using Edge = DependencyGraphEdge;
    static DependencyGraph* Create();
    static void Destroy(DependencyGraph* graph);
    virtual ~DependencyGraph() = default;

    virtual dep_graph_handle_t Insert(DependencyGraphNode* pNode) = 0;
    virtual bool Link(DependencyGraphNode* pFrom, DependencyGraphNode* pTo, DependencyGraphEdge* pEdge) = 0;
    virtual Node* AccessNode(dep_graph_handle_t handle) = 0;
    virtual Node* NodeAt(dep_graph_handle_t id) = 0;
    virtual bool Clear() = 0;
};

inline DependencyGraphNode* DependencyGraphEdge::From()
{
    return m_pGraph->NodeAt(mFromNode);
}

inline DependencyGraphNode* DependencyGraphEdge::To()
{
    return m_pGraph->NodeAt(mToNode);
}