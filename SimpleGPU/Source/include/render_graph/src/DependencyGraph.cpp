#include "render_graph/include/DependencyGraph.hpp"
#include "render_graph/include/Graph_Boost.hpp"
#include "boost/graph/named_graph.hpp"

class DependencyGraphImp : public DependencyGraph, public DependencyGraphBase
{
    using DAGVertex = DependencyGraphBase::DAGVertex;
public:
    virtual dep_graph_handle_t Insert(DependencyGraphNode* pNode) override;
    virtual bool Link(DependencyGraphNode* pFrom, DependencyGraphNode* pTo, DependencyGraphEdge* pEdge) override;
    virtual Node* AccessNode(dep_graph_handle_t handle) override;
    virtual Node* NodeAt(dep_graph_handle_t id) final;
    virtual bool Clear() final;
    virtual uint32_t ForeachIncomingEdges(Node* node, const std::function<void(Node* from, Node* to, Edge* edge)>& func) final;
    virtual uint32_t ForeachIncomingEdges(dep_graph_handle_t handle, const std::function<void(Node* from, Node* to, Edge* edge)>& func) final;

protected:
    vertex_descriptor GetDescriptor(Node* node)
    {
        return (vertex_descriptor)node->mId;
    }
};

dep_graph_handle_t DependencyGraphImp::Insert(DependencyGraphNode* pNode)
{
    //boos::add_vertex
    pNode->mId      = (dep_graph_handle_t)boost::add_vertex(pNode, *this);
    pNode->m_pGraph = this;
    return pNode->mId;
}

bool DependencyGraphImp::Link(DependencyGraphNode* pFrom, DependencyGraphNode* pTo, DependencyGraphEdge* pEdge)
{
    //boost::add_edge
    auto&& result = boost::add_edge(GetDescriptor(pFrom), GetDescriptor(pTo), pEdge, *this);
    if (pEdge)
    {
        pEdge->m_pGraph  = this;
        pEdge->mFromNode = pFrom->GetId();
        pEdge->mToNode   = pTo->GetId();
    }
    return result.second;
}

DependencyGraph::Node* DependencyGraphImp::AccessNode(dep_graph_handle_t handle)
{
    return (*this)[DAGVertex(handle)];
}

DependencyGraph::Node* DependencyGraphImp::NodeAt(dep_graph_handle_t id)
{
    return (*this)[DAGVertex(id)];
}

bool DependencyGraphImp::Clear()
{
    BoostGraph::Graph<DependencyGraph::Node*, DependencyGraph::Edge*>::clear();
    return true;
}

uint32_t DependencyGraphImp::ForeachIncomingEdges(Node* node, const std::function<void(Node* from, Node* to, Edge* edge)>& func)
{
    return ForeachIncomingEdges(node->mId, func);
}

uint32_t DependencyGraphImp::ForeachIncomingEdges(dep_graph_handle_t handle, const std::function<void(Node* from, Node* to, Edge* edge)>& func)
{
    uint32_t count = 0;
    auto oedges = boost::in_edges((vertex_descriptor)handle, *this);
    for (auto iter = oedges.first; iter != oedges.second; ++iter)
    {
        func(NodeAt(iter->m_source), NodeAt(iter->m_target), (*this)[*iter]);
        count++;
    }
    return count;
}

//////////////////////////////////////////////////////////////////////////////
DependencyGraph* DependencyGraph::Create()
{
    return new DependencyGraphImp();
}

void DependencyGraph::Destroy(DependencyGraph* graph)
{
    if (graph) delete graph;
}

DependencyGraphBase* DependencyGraphBase::As(DependencyGraph* graph)
{
    return (DependencyGraphImp*)graph;
}