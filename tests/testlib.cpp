#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <networkit/graph/Graph.hpp>
#include <stdlib.h> 
#include <networkit/io/EdgeListReader.hpp>
#include <unordered_map>
#include <iostream>
#include <SFMT.h>
#include <absl/container/flat_hash_set.h>
#include <aditum/RandomRRSetGenerator.hpp>
#include <aditum/AditumGraph.hpp>
#include <aditum/Distribution.hpp>
#include <memory>
#include <algorithm>

TEST_CASE( "Graph Loading", "[.]" ) {
    INFO("test");
    NetworKit::EdgeListReader reader(' ', 0, "#", true, true);
    auto g = std::make_unique<NetworKit::Graph>(reader.read("/home/antonio/Garbage/graph.txt"));
    // NetworKit::Graph g =  reader.read("/home/antonio/Garbage/graph.txt");
    REQUIRE (g->numberOfNodes() == 8);
    
}

TEST_CASE("RANDOM", "[.]") {
    INFO("Random test");
    sfmt_t gen;
    int i, N = 100;
    sfmt_init_gen_rand(&gen, rand());
    for (i = 0; i < N; ++i) 
	REQUIRE(sfmt_genrand_real1(&gen));
}

TEST_CASE("ABSEIL", "[.]") {
    absl::flat_hash_set<int> s;
    s.emplace(3);
    s.emplace(5);
    REQUIRE(s.size()==2);
    for(auto x : s)
	INFO(x);
}

TEST_CASE("rr set", "[rr set generaton]"){
    NetworKit::EdgeListReader reader(' ', 0, "#", true, true);
    NetworKit::Graph g =  reader.read("/home/antonio/Garbage/graph.txt");
    REQUIRE (g.numberOfNodes() == 8);
    Aditum::LTRandomRRSetGenerator gen;
    absl::flat_hash_set<uint> nodes;
    static_cast<Aditum::RandomRRSetGenerator<decltype(gen)>>(gen)
	.genn(g,
	      0,
	     [&](NetworKit::node src, NetworKit::node trg, NetworKit::edgeweight ew){
		 nodes.emplace(src);
	     });
	     // [&](NetworKit::node v){return false;});
	

    // ((Aditum::RandomRRSetGenerator<decltype(gen)>) gen).genn(g,
    // 	     0,
    // 	     [&](NetworKit::node src, NetworKit::node trg, NetworKit::edgeweight ew){
    // 		 nodes.emplace(src);
    // 	     });
    // 	     // [&](NetworKit::node v){return false;});

    for(auto x : nodes)
	INFO(x);
}


TEST_CASE("aditum graph", "[.]")
{
    Aditum::AditumGraphBuilder builder;
    builder = builder.setGraphPath("/home/antonio/Garbage/graph.txt")
	.setScoresPath("/home/antonio/Garbage/scores.txt");
    Aditum::AditumGraph g(builder.build());
    REQUIRE(g.graph().numberOfNodes()==8);
    for(auto x : g.scores())
	INFO(x);

}

TEST_CASE("distribution single", "[.]")
{
    std::vector<double> p = {1,1,1,1,1};
    Aditum::Distribution dist(p, 0);
    std::unordered_map<int, int> map;
    for(int i=0 ; i<100 ; i++){
	map[dist.sample()]++;
    }

    for(auto it : map)
	std::cout << it.first << " " << it.second << "\n";
}

TEST_CASE("distribution vector", "[.]")
{
    std::vector<double> p = {1,1,1,1,1};
    Aditum::Distribution dist(p, 0);
    std::vector<int> v = dist.sample(10000);
    std::unordered_map<int, int> map;
    for(int i=0 ; i<v.size() ; i++){
	map[v[i]]++;
    }
    for(auto it : map)
	std::cout << it.first << " " << it.second << "\n";
}


