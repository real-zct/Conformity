// #ifndef DISTANCEBASED_H
// #define DISTANCEBASED_H

// #include <aditum/AditumAttributeDriven.hpp>
// #include <absl/container/flat_hash_map.h>
// #include <aditum/AditumBuilder.hpp>
// #include <iostream>
// namespace Aditum
// {

// /**
//  * Class for the attribute wise algorithm.
//  */
// template <typename SetGenerator>
// class DistanceBased: public AditumAttributeDriven<SetGenerator, DistanceBased<SetGenerator>>
// {
// private:
//     /*!< radius for the hamming ball definition */
//     int radius;

//     using symbol = typename AditumAttributeDriven<SetGenerator, DistanceBased<SetGenerator>>::symbol;
    
// public:

//     using Base = AditumAttributeDriven<SetGenerator, DistanceBased<SetGenerator>>;
    
//     DistanceBased(AditumGraph& graph, Distribution dist,
// 		  std::vector<std::vector<symbol>> userAttributes,
// 		  int k, int radius=1, double alpha = 1, double epsilon = 1, double l = 1):
// 	Base(graph, dist, userAttributes, k, alpha, epsilon,l), radius{radius} {}


//     void buildSeedSetWithDiversity() 
//     {
// 	//function for updating the covered symbols
// 	auto updateCoveredSymbols = [&](unsigned int node, auto &coveredAttributes)
// 	{
// 	};
	
// 	auto computeMarginalDiversity = [&](unsigned int node, auto &coveredAttributes)
// 	{
// 	};

// 	//compute the maximum value for the marginal gain
// 	//wrt to the capital score of a node and its diversity
// 	//-- the maximum marginal gain is simply the greatest score as a singleton
// 	double maxCapital = this->normalizeCapital();
// 	double maxDiversity = (this->userAttributes.begin())->size();
	

// 	//initialize the score vector
// 	//and set the correct weighting factor for the ScoreObject class
// 	std::vector<Utility::ScoreObject> q(this->nodesAchievedCapital.size());
	
// 	#pragma omp parallel
// 	for (unsigned int i = 0;  i<q.size() ; ++i) 
// 	    q[i] = Utility::ScoreObject {i, 0, this->nodesAchievedCapital[i], 1};
	
// 	//sort q
// 	std::make_heap(q.begin(), q.end());

// 	//initialize the map for storing the number of nodes
// 	//having a particular attribute in their profile
// 	std::vector<absl::flat_hash_map<symbol, int>> coveredAttributes(this->numberOfAttributes);

// 	this->seedSet.clear();

// 	while(this->seedSet.size()<this->k)
// 	{
// 	    std::pop_heap(q.begin(), q.end());
// 	    auto &item = q.back();

// 	    if(item.iteration == this->seedSet.size())
// 	    {	
// 		for(auto setId : this->nodeSetIndexes[item.node])
// 		    for(auto node : this->rrsets[setId])
// 			this->nodesAchievedCapital[node] -= this->aGraph.score(this->setRoot[setId])/maxCapital;

// 		//add the symbols covered by this node
// 		updateCoveredSymbols(item.node, coveredAttributes);
// 		this->seedSet.emplace(item.node);
// 		q.pop_back();
// 	    }
// 	    else
// 	    {
// 		item.capitalScore = this->nodesAchievedCapital[item.node];
// 		item.diversityScore = computeMarginalDiversity(item.node, coveredAttributes)/maxDiversity;
		
// 		item.iteration = this->seedSet.size();
// 		std::push_heap(q.begin(), q.end());
// 	    }
// 	}
//     }

// private:

   
// };

// }

// #endif

