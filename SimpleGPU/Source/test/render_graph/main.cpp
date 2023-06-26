#include <iostream>
#include "render_graph/include/backend/RenderGraphBackend.h"
int main()
{
    std::cout << "render_graph_test" << std::endl;

    RenderGraph* graph = RenderGraph::Create([=](){
        std::cout << "create rendergraph setup" << std::endl;
    });
    graph->Compile();
    graph->Execute();
    RenderGraph::Destroy(graph);
    return 0;
}