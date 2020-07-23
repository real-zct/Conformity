#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/io/EdgeListReader.hpp>

#include <iostream>

TEST_CASE( "Quick check", "[main]" ) {
    NetworKit::EdgeListReader reader(' ', 0, "#", true, true);
    NetworKit::Graph g =  reader.read("/home/antonio/garbage/graph.txt");

    REQUIRE (g.numberOfNodes() == 4);


}

