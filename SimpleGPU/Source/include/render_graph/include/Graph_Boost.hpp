#pragma once

#include "boost/graph/adjacency_list.hpp"

namespace BoostGraph
{
    template <
        class VertexProperty = boost::no_property,
        class EdgeProperty   = boost::no_property,
        class GraphProperty  = boost::no_property,
        class EdgeListS      = boost::listS>
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VertexProperty, EdgeProperty, GraphProperty, EdgeListS>;

    template <
        class VertexProperty = boost::no_property,
        class EdgeProperty   = boost::no_property,
        class GraphProperty  = boost::no_property>
    using GraphVertex = typename boost::graph_traits<Graph<VertexProperty, EdgeProperty, GraphProperty>>::vertex_descriptor;

    template <
        class VertexProperty = boost::no_property,
        class EdgeProperty   = boost::no_property,
        class GraphProperty  = boost::no_property>
    using GraphEdge = typename boost::graph_traits<Graph<VertexProperty, EdgeProperty, GraphProperty>>::edge_descriptor;
}