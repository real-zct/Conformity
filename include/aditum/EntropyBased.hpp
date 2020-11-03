#ifndef ENTROPYBASED_H
#define ENTROPYBASED_H

#include <aditum/AditumAttributeDriven.hpp>
#include <aditum/AditumBuilder.hpp>
#include <utility>
#include <vector>
#include <numeric>
#include <absl/container/flat_hash_map.h>

namespace Aditum {
/**
 * Class for the entropy based algorithm.
 */
template <typename SetGenerator>
class EntropyBased: public AditumAttributeDriven<SetGenerator, EntropyBased<SetGenerator>>
{
private:

    using Base = AditumAttributeDriven<SetGenerator, EntropyBased<SetGenerator>>;
    using symbol = typename Base::symbol;
    using SymbolPosition = std::pair<symbol, int>;

    /*
    * Representation of each categorical symbol
    * after the preprocessing step
    */
    struct SymbolRepresentation
    {
	/*!< Numerical code  */
	int code;
	/*!< Frequency of the symbol */
	double prob;

	/**
	 * @brief      Return the one hot representation
	 *
	 * @details    Return the one hot representation of the symbol
	 *
	 * @param      length vector
	 *
	 * @return     one hot vector having v[code] = 1 and the rest equal to 0
	 */
	std::vector<int> oneHot(int length)
	{
	    std::vector<int> v(length, 0);
	    v[code] = 1;
	    return v;
	}
	
    };
    
    /*!< map that associates an integer code and a probability to each symbol */
    std::vector<absl::flat_hash_map<symbol, SymbolRepresentation>> symbolVectorMap;

    /*!< symbol corresponding to a particular index in the global-one vector */
    std::vector<SymbolPosition> symbolPosition;
    
    /*!< number of required bits for each attribute */
    std::vector<int> attributesBinaryLength;

    
public:    

    EntropyBased(AditumGraph& graph, Distribution dist,
		  std::vector<std::vector<symbol>> userAttributes,
		  int k,double alpha = 1, double epsilon = 1, double l = 1):
	Base(graph, dist, userAttributes, k, alpha, epsilon,l) {}


    void buildSeedSetWithDiversity()
    {
	//encode each symbol
	initSymbolMap();
	//size of the binary representation of a user profile
	int binaryVectorSize = std::accumulate(attributesBinaryLength.begin(),
					       attributesBinaryLength.end(),
					       0);
	std::vector<std::vector<bool>> seedsOneHotSymbols;

	auto updatesSeedsRepresentation = [&](unsigned int node)
	{
	    std::vector<bool> nodeOneHotSymbols;

	    //add the node's attributes
	    int pos = 0;
	    for(symbol s : this->userAttributes[node])
	    {
		//transform each symbol in its one hot counterpart
		auto sVector = this->symbolVectorMap[pos][s].oneHot(this->attributesBinaryLength[pos]);
		//copy this vector into the global one
		std::move(sVector.begin(), sVector.end(), std::back_inserter(nodeOneHotSymbols));
	    }
	    //add the node vector to the vector of seeds' symbols
	    seedsOneHotSymbols.emplace_back(std::move(nodeOneHotSymbols));
	};
	/*!< User attriubtes represented as a one hot encoding vector */
	double maxCapital = this->normalizeCapital();
	double maxDiversity = *(std::max_element(this->userRewards.begin(),
					       this->userRewards.end()));
	auto q = this->getInitialScoreVector(maxCapital, maxDiversity);
	//this->selectionLoop(q, updateCoveredClasses, computeMarginalDiversity, maxCapital, maxDiversity);
    }



    /**
     * @brief      Initialize the attriubte representation
     *
     * @details    This method assign a numerical code to every symbol.
     *             More specifically, anyu attribute at position i will have be
     *             encoded with a numberical value ranging in [0, N] where N is
     *             the number of different codes for the i-th attribute.
     */
    void initSymbolMap()
    {
	//it keeps track of the last code assigned for each
	//attribute position
	int attributeSize = this->userAttributes.begin()->size();
	int size = this->aGraph->graph().numberOfNodes() * attributeSize ;
	//symbols position per attribute
	std::vector<std::vector<symbol>> symbolPositionVector(attributeSize);
	//the last code assigned for each attribute position
	std::vector<int> lastCodes(attributeSize, 0);

	//iterate over all the vectors of user attributes and
	//assign a code to each symbol
	for(std::vector<symbol> &attributes : this->userAttributes)
	{
	    auto it = attributes.begin();
	    for(int i=0; it!=attributes.end(); ++it, ++i)
	    {
		try
		{
		    SymbolRepresentation &sRepr = symbolVectorMap[i].at(*it);
		    //update the probability
		    sRepr.prob += 1/size;
		}
		catch (std::exception &e)
		{
		    //miss in the table. It means a new symbol has been encountered
		    //create a new SymbolRepresentation object and store it in the map
		    SymbolRepresentation sRepr{lastCodes[i]++, 1/size};
		    symbolVectorMap[i][*it] = sRepr;

		    //save it in  the vector symbolPosition
		    symbolPositionVector[i] = *it;
		}
	    }
	}
	//set the number of required bits for each attribute
	for(auto &maxCode : lastCodes)
	    maxCode = std::log2(maxCode);
	//copy the values into the member field
	std::swap(attributesBinaryLength, lastCodes);

	//initialize the vector of symbol positions
	int pos = 0;
	for(auto it = symbolPositionVector.begin(); it!=symbolPositionVector.end();it++){
	    
	    
	}
    }

    
};

}

#endif

