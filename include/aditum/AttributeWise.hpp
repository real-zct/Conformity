#ifndef ATTRIBUTEWISE_H
#define ATTRIBUTEWISE_H


#include <aditum/AditumAttributeDriven.hpp>
#include <absl/container/flat_hash_map.h>
#include <aditum/AditumBuilder.hpp>

namespace Aditum
{

/**
 * Class for the attribute wise algorithm.
 */
template <typename SetGenerator>
class AttributeWise: public AditumAttributeDriven<SetGenerator, AttributeWise<SetGenerator>>
{
private:
    /*!< power coefficient */
    double lam;

    using symbol = typename AditumAttributeDriven<SetGenerator, AttributeWise<SetGenerator>>::symbol;
    
public:

    using Base = AditumAttributeDriven<SetGenerator, AttributeWise<SetGenerator>>;
    
    AttributeWise(AditumGraph& graph, Distribution dist,
		  std::vector<std::vector<symbol>> userAttributes,
		  int k, double lambda=1, double alpha = 1, double epsilon = 1, double l = 1):
	Base(graph, dist, userAttributes, k, alpha, epsilon,l), lam{lambda} {
    }


    void buildSeedSetWithDiversity() 
    {
	//initialize the map for storing the number of nodes
	//having a particular attribute in their profile
	std::vector<absl::flat_hash_map<symbol, int>> coveredAttributes(this->numberOfAttributes);

	//function for updating the covered symbols
	auto updateCoveredSymbols = [&](unsigned int node)
	{
	    //update the of covered symbols
	    auto &attributes = this -> userAttributes[node];
	    auto itSymbol = attributes.begin();
	    for(unsigned int i=0; itSymbol!=attributes.end(); ++i, ++itSymbol)
		coveredAttributes[i][*itSymbol] += 1;
	};
	
	auto computeMarginalDiversity = [&](unsigned int node)
	{
	    double marginalGain = 0;
	    auto &attributes = this->userAttributes[node];
	    auto itSymbols = attributes.begin();
	    for(unsigned int i=0; itSymbols!=attributes.end(); ++i, ++itSymbols)
		try
		{
		    int alreadyPresent = coveredAttributes[i].at(*itSymbols);
		    marginalGain += std::pow(alreadyPresent+1, -lam);
		}
		catch (std::exception &e)
		{
		    marginalGain+=1;
		}
	    return marginalGain;
	};

	//compute the maximum value for the marginal gain
	//wrt to the capital score of a node and its diversity
	//-- the maximum marginal gain is simply the greatest score as a singleton
	double maxCapital = this->normalizeCapital();
	double maxDiversity = (this->userAttributes.begin())->size();

        //initialize the score vector	
	auto q = this->getInitialScoreVector(maxCapital, maxDiversity);
        this->selectionLoop(q, updateCoveredSymbols, computeMarginalDiversity, maxCapital, maxDiversity);
     }
};


/**
 * @brief      Builder for the attribute wise algorithm
 *
 * @details    Builder for the attribute wise algorithm
 */    
class AttributeWiseBuilder: public AditumBuilder<AttributeWiseBuilder>
{
public:    
    template<typename SetGenerator, typename DiversityAwareAlgo>
    AditumBase<SetGenerator, DiversityAwareAlgo>* build()
   {
	Distribution nodeDistribution = computeDistribution();
	return new AttributeWise<SetGenerator>(*aGraph,
					       nodeDistribution,
					       userAttributes,
					       k,
					       lambda,
					       alpha,
					       epsilon,
					       l);
    }
};
}
#endif
