#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp> 
#include <aditum/RandomRRSetGenerator.hpp>
#include <aditum/AditumGraph.hpp>
#include <aditum/AditumAlgo.hpp>
#include <aditum/Distribution.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/io/EdgeListReader.hpp>
#include <aditum/AttributeWise.hpp>
#include <aditum/AditumBuilder.hpp>
#include <aditum/io/UserAttributesFileReader.hpp>


TEST_CASE( "Aditum Base", "Ig Loading" ) {
    Aditum::AditumGraphBuilder builder;
    builder = builder.setGraphPath("/home/antonio/Garbage/InstagramLCC/graph_lt.inf")
	.setScoresPath("/home/antonio/Garbage/InstagramLCC/lurker_score.txt");

    Aditum::UserAttributesFileReader a("\\s+");
    std::string path = "/home/antonio/Garbage/InstagramLCC/graph_lt_exponential_10_user_attributes.txt";
    std::vector<std::vector<std::variant<int, std::string>>> data{a.read(path)};
    Aditum::AditumGraph g(builder.build());
    Aditum::AditumBuilder<Aditum::AttributeWiseBuilder> algoBuilder;
    algoBuilder.setGraph(g)
	.setAlpha(0.2)
	.setEpsilon(1)
	.setK(50)
	.setTargetThreshold(0.42)
	.setAttributes(data);
    
    using SetGenerator = Aditum::LTRandomRRSetGenerator;
    using DiversityAware = Aditum::AttributeWise<SetGenerator>;

    auto algo = algoBuilder.build< Aditum::LTRandomRRSetGenerator,
				  Aditum::AttributeWise<Aditum::LTRandomRRSetGenerator>>();
    algo->run();

    auto seeds = algo->getSeeds();

    for(auto x : seeds)
	std::cout << x << "\n";
    
}

