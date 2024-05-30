// #ifndef CLASSBASED_H
// #define CLASSBASED_H

// #include <aditum/AditumAttributeDriven.hpp>
// #include <aditum/AditumBuilder.hpp>
// #include <absl/container/flat_hash_map.h>
// #include <algorithm>

// namespace Aditum {
// /**
//  * Class for the attribute wise algorithm.
//  * The class of each user is ecnoded in the 
//  * usserAttributes vector of the super class. 
//  * The first element of each vector represents the class of the user.
//  *
//  * The reward function is log(1+x)
//  */
// template <typename SetGenerator>
// class ClassBased: public AditumAttributeDriven<SetGenerator, ClassBased<SetGenerator>>
// {
// private:

//     /*!< The reward deriving by each user selection */
//     std::vector<double> userRewards;

// public:
//     using Base = AditumAttributeDriven<SetGenerator, ClassBased<SetGenerator>>;
//     using symbol = typename Base::symbol;
    
//     ClassBased(AditumGraph& graph, Distribution dist,
// 	       std::vector<std::vector<symbol>> userAttributes,
// 	       int k, std::vector<double> userRewards,
// 	       double alpha = 1, double epsilon = 1, double l = 1):
// 	Base(graph, dist, userAttributes, k, alpha, epsilon,l), userRewards{userRewards} {}

//     void buildSeedSetWithDiversity() 
//     {
// 	//rewards sum for each class 
// 	absl::flat_hash_map<symbol, double> coveredClasses;

// 	//function for updating the covered symbols
// 	auto updateCoveredClasses = [&](unsigned int node)
// 	{
// 	    //get the class of the user
// 	    auto &nodeClass = this -> userAttributes[node][0];
// 	    //update ht
// 	    coveredClasses[nodeClass] += userRewards[node];
// 	};

// 	//functioin for computing the margina gain of a node
// 	auto computeMarginalDiversity = [&](unsigned int node)
// 	{
// 	    auto &nodeClass = this->userAttributes[node][0];
// 	    double currentSum = coveredClasses[nodeClass];
// 	    return std::log(1+userRewards[node]/currentSum);
// 	};

// 	//compute the maximum value for the marginal gain
// 	//wrt to the capital score of a node and its diversity
// 	//-- the maximum marginal gain is simply the greatest score as a singleton
// 	double maxCapital = this->normalizeCapital();
// 	double maxDiversity = *(std::max_element(this->userRewards.begin(),
// 					       this->userRewards.end()));
// 	maxDiversity = std::log(1+maxDiversity);
// 	auto q = this->getInitialScoreVector(maxCapital, maxDiversity);
// 	this->selectionLoop(q, updateCoveredClasses, computeMarginalDiversity, maxCapital, maxDiversity);
//     }
// };


// /**
//  * @brief      Builder for the attribute wise algorithm
//  *
//  * @details    Builder for the attribute wise algorithm
//  */    
// class ClassBasedBuilder: public AditumBuilder<ClassBasedBuilder>
// {
// public:    
//     template<typename SetGenerator, typename DiversityAwareAlgo>
//     AditumBase<SetGenerator, DiversityAwareAlgo>* build()
//    {
//        Distribution nodeDistribution = computeDistribution();

//        //if the user does not specify a reward vector then each node has
//        //reward 1
//        if(this->userRewards.empty())
//        {
// 	   int vectorSize = this->aGraph->graph().upperNodeIdBound();
// 	   this->userRewards = std::move(std::vector<double>(vectorSize, 1));
//        }

//        	return new ClassBased<SetGenerator>(*aGraph,
// 					    nodeDistribution,
// 					    userAttributes,
// 					    k,
// 					    this->userRewards,
// 					    alpha,
// 					    epsilon,
// 					    l);

//     }
// };    
// }

// #endif
