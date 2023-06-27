#include "render_graph/include/DependencyGraph.hpp"

class DependencyGraphImp : public DependencyGraph
{
public:
    virtual dep_graph_handle_t Insert(DependencyGraphNode* pNode) override;
    virtual bool Link(DependencyGraphNode* pFrom, DependencyGraphNode* pTo, DependencyGraphEdge* pEdge) override;
};

dep_graph_handle_t DependencyGraphImp::Insert(DependencyGraphNode* pNode)
{
    //boos::add_vertex
    pNode->m_pGraph = this;
    return pNode->mId;
}

bool DependencyGraphImp::Link(DependencyGraphNode* pFrom, DependencyGraphNode* pTo, DependencyGraphEdge* pEdge)
{
    //boost::add_edge

    if (pEdge)
    {
        pEdge->m_pGraph  = this;
        pEdge->mFromNode = pFrom->GetId();
        pEdge->mToNode   = pTo->GetId();
    }

    return true;
}

DependencyGraph* DependencyGraph::Create()
{
    return new DependencyGraphImp();
}

void DependencyGraph::Destroy(DependencyGraph* graph)
{
    if (graph) delete graph;
}