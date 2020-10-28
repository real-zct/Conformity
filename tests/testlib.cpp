#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp> 
#include <networkit/graph/Graph.hpp>
#include <stdlib.h> 
#include <networkit/io/EdgeListReader.hpp>
#include <unordered_map>
#include <iostream>
#include <SFMT.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/flat_hash_map.h>
#include <aditum/RandomRRSetGenerator.hpp>
#include <aditum/AditumGraph.hpp>
#include <aditum/AditumAlgo.hpp>
#include <aditum/Distribution.hpp>
#include <memory>
#include <algorithm>
#include <aditum/io/UserAttributesFileReader.hpp>
#include <variant>


template<typename ... Ts>
struct visitor : Ts... {
    using Ts::operator()...; 
};
template<typename ... Ts> visitor(Ts...) -> visitor<Ts...>;



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

TEST_CASE("abslmap", "mapp") {
    using symbol = std::variant<int, std::string>;
    absl::flat_hash_map<symbol, int> m;
    symbol s1{5};
    symbol s2{"ciao"};

    m[s1] = 10;
    std::cout << m[s1] << "\n";
    try{
    std::cout << m.at(s2) << "\n";
    }catch(std::exception &e) {std::cout<<"exception"<< "\n";}
    m[s2] += 1;
    std::cout << m[s2] << "\n";
}


TEST_CASE("rr set", "[.]"){
    NetworKit::EdgeListReader reader(' ', 0, "#", true, true);
    NetworKit::Graph g =  reader.read("/home/antonio/Garbage/graph.txt");
    REQUIRE (g.numberOfNodes() == 8);
    Aditum::LTRandomRRSetGenerator gen;
    absl::flat_hash_set<uint> nodes;
    nodes.emplace(1); int cov = 0;
    static_cast<Aditum::RandomRRSetGenerator<decltype(gen)>>(gen)
	.operator()(g,
	      0,
	     [&](NetworKit::node src, NetworKit::node trg, NetworKit::edgeweight ew){
		 nodes.emplace(src);
	     },
	     [&](NetworKit::node v)->bool{
		 if(nodes.count(v)>0){
		     cov++;
		     return true;
		 }
		 return false;
	     });
    INFO(cov);
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


TEST_CASE("score object", "[.]"){
    using SO = Aditum::Utility::ScoreObject;
    std::vector<SO> q;
    q.push_back(SO{0,0,1,0});
    q.push_back(SO{1,0,2,0});
    q.push_back(SO{2,0,3,0});

    std::make_heap(q.begin(),q.end());
    
    while(!q.empty()){
	std::pop_heap(q.begin(), q.end());
	SO &item = q.back();
	std::cout << item.node << " "<< item.capitalScore << "\n";
	q.pop_back();
    }
    std::cout << (SO{0,0,1,0} < SO{0,0,2,0}) << "\n";
}

TEST_CASE("user attributes", "uat")
{
    Aditum::UserAttributesFileReader a("\\s+");
    std::vector<std::vector<std::variant<int, std::string>>> data{a.read("/home/antonio/Garbage/InstagramLCC/attributes.txt")};
    

    for(auto v : data){
	for (auto item : v){
	    std::visit( visitor {
		    [](int arg) { std::cout << arg*2 << "\n"; },
		    [](std::string arg) {std::cout << arg << "s\n";}
		},item);
	  }
     }

}
