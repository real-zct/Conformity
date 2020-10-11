#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp> 
#include <aditum/RandomRRSetGenerator.hpp>
#include <aditum/AditumGraph.hpp>
#include <aditum/AditumAlgo.hpp>
#include <aditum/Distribution.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/io/EdgeListReader.hpp>


TEST_CASE( "Aditum Base", "Ig Loading" ) {
    Aditum::AditumGraphBuilder builder;
    builder = builder.setGraphPath("/home/antonio/Garbage/InstagramLCC/graph_lt.inf")
	.setScoresPath("/home/antonio/Garbage/InstagramLCC/lurker_score.txt");
    Aditum::AditumGraph g(builder.build());
    INFO(g.score(0));

    Aditum::AditumBuilder algoBuilder;
    algoBuilder.setGraph(g)
	.setK(10)
	.setTargetThreshold(0.42);
    
    auto algo = algoBuilder.build<Aditum::LTRandomRRSetGenerator>();
    algo.run();

    auto seeds = algo.getSeeds();

    for(auto x : seeds)
	INFO(x);
    
    
    
}

