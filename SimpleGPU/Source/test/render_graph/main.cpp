#include <iostream>
#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/Graph_Boost.hpp"
#include <string>

struct vertex_prop_map_key_t {
    using kind = boost::vertex_property_tag;
};

int main()
{
    /* RenderGraph* graph = RenderGraph::Create([=](){
        std::cout << "create rendergraph setup" << std::endl;
    });
    graph->Compile();
    graph->Execute();
    RenderGraph::Destroy(graph); */

    using vertexProp = boost::property<vertex_prop_map_key_t, std::string>;
    struct edgeProp {
        std::string name;
    };
    using Graph = BoostGraph::Graph<vertexProp, edgeProp>;
    using GraphVertex = BoostGraph::GraphVertex<vertexProp, edgeProp>;

    Graph g(5);

    auto E20 = boost::add_edge(2, 0, g).first;
    g[E20] = { "E20" };
    auto E10 = boost::add_edge(1, 0, g).first;
    g[E10] = { "E10" };
    auto E21 = boost::add_edge(2, 1, g).first;
    g[E21] = { "E21" };
    auto E12 = boost::add_edge(1, 2, g).first;
    g[E12] = { "E12" };
    auto E42 = boost::add_edge(4, 2, g).first;
    g[E42] = { "E42" };
    std::cout << "num vertices: " << boost::num_vertices(g) << std::endl;

    GraphVertex from(2);
    GraphVertex to(0);
    auto out_edges = boost::out_edges(from, g);
    auto iter = out_edges.first;
    while (iter != out_edges.second)
    {
        if (iter->m_target == to)
        {
            auto&& prop = g[*iter];
            std::cout << prop.name.c_str() << std::endl;
        }
        iter++;
    }

    using Graph2       = BoostGraph::Graph<int, edgeProp>;
    using GraphVertex2 = BoostGraph::GraphVertex<int, edgeProp>;
    GraphVertex2 from2(2);
    Graph2 g2;
    boost::add_vertex(12, g2);
    boost::add_vertex(12, g2);
    boost::add_vertex(32, g2);
    auto nd = g2[from2];
    std::cout << "Node ID: " << nd << std::endl;

    boost::property_map<Graph, vertex_prop_map_key_t>::type val = boost::get(vertex_prop_map_key_t(), g);
    boost::put(val, 0, "0");
    boost::put(val, 1, "1");
    boost::put(val, 2, "2");
    boost::put(val, 3, "3");
    boost::put(val, 4, "4");

    GraphVertex vert(2);

    std::cout << "out degrees of vertex " << vert << " is: " << boost::out_degree(vert, g) << std::endl;
    std::cout << "in degrees of vertex " << vert << " is: " << boost::in_degree(vert, g) << std::endl;
    std::cout << "out degrees of vertex " << boost::vertex(4, g) << " is: " << boost::out_degree(boost::vertex(4, g), g) << std::endl;
    
    using IndexMap     = boost::property_map<Graph, boost::vertex_index_t>::type;
    IndexMap index_map = get(boost::vertex_index, g);

    auto inedges = boost::in_edges(vert, g);
    std::cout << "list of in edges of vertex " << vert << ": " << std::endl;
    for (auto iter = inedges.first; iter != inedges.second; iter++)
    {
        std::cout << "    " << boost::source(*iter, g) << "->" << vert << std::endl;
    }
    auto neigs = boost::adjacent_vertices(vert, g);
    std::cout << "neigs of vertex " << vert << ": ";
    for (auto iter = neigs.first; iter != neigs.second; iter++)
    {
        std::cout << BoostGraph::VertexNumber(*iter, g) << ", ";
        //std::cout << index_map[*iter] << ", ";
    }

    using PropertyMap = typename boost::property_map<Graph, vertex_prop_map_key_t>::type;
    auto inv_neigs    = boost::inv_adjacent_vertices(vert, g);
    std::cout << std::endl
              << "inv neigs of vertex " << vert << ": ";
    for (auto iter = inv_neigs.first; iter != inv_neigs.second; iter++)
    {
        std::cout << BoostGraph::GetVertexProperty<vertex_prop_map_key_t>(*iter, g) << ", ";
        /* PropertyMap prop = get(vertex_prop_map_key_t(), g);
        std::cout << prop[*iter] << ", "; */
    }
    std::cout << std::endl;
    return 0;
}