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
#include <aditum/ClassBased.hpp>
#include <absl/container/flat_hash_map.h>
#include <aditum/EntropyBased.hpp>

template<typename ... Ts>
struct visitor : Ts... {
    using Ts::operator()...; 
};
template<typename ... Ts> visitor(Ts...) -> visitor<Ts...>;

TEST_CASE( "Attr Wise", "[.]" ) {
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

TEST_CASE( "Class Based", "[.]" ) {
    Aditum::AditumGraphBuilder builder;
    builder = builder.setGraphPath("/home/antonio/Garbage/InstagramLCC/graph_lt.inf")
	.setScoresPath("/home/antonio/Garbage/InstagramLCC/lurker_score.txt");

    Aditum::UserAttributesFileReader a("\\s+");
    std::string path = "/home/antonio/Garbage/InstagramLCC/graph_lt_exponential_10_user_attributes.txt";
    std::vector<std::vector<std::variant<int, std::string>>> data{a.read(path)};
    Aditum::AditumGraph g(builder.build());
    Aditum::AditumBuilder<Aditum::ClassBasedBuilder> algoBuilder;
    algoBuilder.setGraph(g)
	.setAlpha(0.2)
	.setEpsilon(1)
	.setK(50)
	.setTargetThreshold(0.42)
	.setAttributes(data);
    
    auto algo = algoBuilder.build<Aditum::LTRandomRRSetGenerator,
				  Aditum::ClassBased<Aditum::LTRandomRRSetGenerator>>();
    algo->run();

    auto seeds = algo->getSeeds();

    for(auto x : seeds)
	std::cout << x << "\n";
    
}

TEST_CASE("Entropy Based", "run") {
        Aditum::AditumGraphBuilder builder;
    builder = builder.setGraphPath("/home/antonio/Garbage/InstagramLCC/graph_lt.inf")
	.setScoresPath("/home/antonio/Garbage/InstagramLCC/lurker_score.txt");

    Aditum::UserAttributesFileReader a("\\s+");
     std::string path = "/home/antonio/Garbage/InstagramLCC/graph_lt_exponential_10_user_attributes.txt";
       // std::string path = "/home/antonio/Garbage/SynData/synDataNormal10.csv";
    std::vector<std::vector<std::variant<int, std::string>>> data{a.read(path)};
    INFO(data.size());
    Aditum::AditumGraph g(builder.build());

    Aditum::AditumBuilder<Aditum::EntropyBasedBuilder> algoBuilder;
    algoBuilder.setGraph(g)
	.setAlpha(0.2)
	.setEpsilon(1)
	.setK(50)
	.setTargetThreshold(0.42)
	.setAttributes(data);

    auto algo = algoBuilder.build<Aditum::LTRandomRRSetGenerator,
				  Aditum::EntropyBased<Aditum::LTRandomRRSetGenerator>>();
	
    algo->run();

    auto seeds = algo->getSeeds();
    std::cout << seeds.size() << "\n";

    for(auto x : seeds)
	std::cout << x << "\n";

}

TEST_CASE( "Entropy Computation", "[.]" ) {
    Aditum::AditumGraphBuilder builder;
    builder = builder.setGraphPath("/home/antonio/Garbage/InstagramLCC/graph_lt.inf")
	.setScoresPath("/home/antonio/Garbage/InstagramLCC/lurker_score.txt");

    Aditum::UserAttributesFileReader a("\\s+");
     std::string path = "/home/antonio/Garbage/InstagramLCC/graph_lt_exponential_10_user_attributes.txt";
       // std::string path = "/home/antonio/Garbage/SynData/synDataNormal10.csv";
    std::vector<std::vector<std::variant<int, std::string>>> data{a.read(path)};
    INFO(data.size());
    Aditum::AditumGraph g(builder.build());

    Aditum::AditumBuilder<Aditum::EntropyBasedBuilder> algoBuilder;
    algoBuilder.setGraph(g)
	.setAlpha(0.2)
	.setEpsilon(1)
	.setK(5)
	.setTargetThreshold(0.42)
	.setAttributes(data);
    
    using symbol = std::variant<int, std::string>;
    auto algo = algoBuilder.build<Aditum::LTRandomRRSetGenerator,
				  Aditum::EntropyBased<Aditum::LTRandomRRSetGenerator>>();
    std::cout << " building" << "\n";
    auto castedAlgo = ((Aditum::EntropyBased<Aditum::LTRandomRRSetGenerator>*) algo);
    castedAlgo->transformSymbols();
    for(int i=0 ; i < 10 ; i++) {
	int j=0;
	for(auto item : data[i]){
	    std::visit( visitor {
		[](int arg) { std::cout << arg << " "; },
		[](std::string arg) {std::cout << arg << " ";}
	    },item);
	    auto repr = castedAlgo->symbolVectorMap[j][item];
	    std::cout << repr.code << " "<<repr.prob<< "|";
	    j++;
	}
	std::cout<< "\n";
	std::cout << castedAlgo->getStringRepresentation(i) <<"\n";
	std::cout << "===================================\n";
    }
    
    absl::flat_hash_map<unsigned int, std::string> cache;
    auto getString = [&](unsigned int node) -> std::string {
	std::string nodeString;
	try{
	    nodeString = cache.at(node);
	}catch (std::exception &e){
	    nodeString = castedAlgo->getStringRepresentation(node);
	    cache[node] = nodeString;
	}
	return nodeString;
    };

    std::vector<std::string> seedsProfiles(castedAlgo->symbolPositions.size());
    auto updatesSeedsRepresentation = [&](unsigned int node)
    {
	std::string nodeString = getString(node);
	int pos = 0;
	for(char &c : nodeString) 
	    seedsProfiles[pos++].push_back(c);
    };

    updatesSeedsRepresentation(0);
    updatesSeedsRepresentation(1);
    updatesSeedsRepresentation(2);
    //for(auto s : seedsProfiles) {
    // 	std::cout << s << "\n";
    // }


    std::string nodeString = getString(3);
    
    //compute joint probabilities
    absl::flat_hash_map<char, absl::flat_hash_map<std::string, double>> jointProbs, condProbs;
    //iterate through each char int the nodeString
    for(int i=0 ; i<nodeString.length() ; i++)
    {
	symbol currentSymbol = castedAlgo->symbolPositions[i].first;
	int currentAttribute = castedAlgo->symbolPositions[i].second;
	//get symbol probability
	double currentSymbolProb = castedAlgo->symbolVectorMap[currentAttribute][currentSymbol].prob;
	jointProbs[nodeString[i]][seedsProfiles[i]] += currentSymbolProb;
    }

    for(auto entry: jointProbs)
    {
	std::cout << entry.first << "\n";
	for(auto __entry : entry.second)
	{
	    std::cout << __entry.first <<": "<<__entry.second<< "\n";
	}
	std::cout << "++++++++++++" << "\n";
    }


    std::cout << "marg probs" << "\n";

    //compute the marginal probabilities
    absl::flat_hash_map<std::string, double> xProbs, yProbs;
    for(auto &charEntry : jointProbs)
	{
	    for(auto &stringEntry : charEntry.second)
		{
		    xProbs[stringEntry.first] += stringEntry.second;
		    yProbs[std::string(1,charEntry.first)] += stringEntry.second;
		}
	}

    for(auto entry : xProbs) {
	std::cout << entry.first <<": "<< entry.second << "\n";
    }
    std::cout << "++++++++++++" << "\n";
    for(auto entry : yProbs) {
	std::cout << entry.first <<": "<< entry.second << "\n";
    }
    
     //compute the conditional probabilities
     for(auto &jointEntry : jointProbs) {
     	for(auto &joinMap : jointEntry.second) {
     	    condProbs[jointEntry.first][joinMap.first] = joinMap.second/xProbs[joinMap.first];
     	}
     }

    std::cout << "++++++CondProbs+++++" << "\n";
    for(auto entry: condProbs)
    {
	std::cout << entry.first << "\n";
	for(auto __entry : entry.second)
	{
	    std::cout << __entry.first <<": "<<__entry.second<< "\n";
	}
	std::cout << "++++++++++++" << "\n";
    }


    double h = 0;
    for(auto &margEntry : xProbs)
	h += margEntry.second * castedAlgo->H({condProbs['0'][margEntry.first], condProbs['1'][margEntry.first]});
    std::cout << "Entropy: "<< h << "\n";

     



    
    
}


