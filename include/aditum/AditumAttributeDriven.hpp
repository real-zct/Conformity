#ifndef ADITUMATTRIBUTEDRIVEN_H
#define ADITUMATTRIBUTEDRIVEN_H
#include <vector>
#include <string>
#include <variant>
#include <algorithm>
#include <aditum/AditumAlgo.hpp>

namespace Aditum
{
/**
 *  Super class for every Attribute driven 
 *  aditum implementation
 */
    template<typename SetGenerator, typename  DiversityAwareAlgo>    
    class AditumAttributeDriven: public AditumBase<SetGenerator, DiversityAwareAlgo>
{
protected:
    /*!< Definition of categorical attribute */
    using symbol = std::variant<int, std::string>;


    /*!< Vectors of attributes associated with each node in the graph */
    std::vector<std::vector<symbol>> userAttributes;

    /*!< Number of categorical attriubtes for each user */
    int numberOfAttributes;
    
public:

    /*!< constructor */
    AditumAttributeDriven(AditumGraph& graph, Distribution dist,
			  std::vector<std::vector<symbol>> userAttributes,
			  int k, double alpha = 1, double epsilon = 1, double l = 1):
	AditumBase<SetGenerator, DiversityAwareAlgo>(graph, dist, k, alpha, epsilon,l), userAttributes{userAttributes} {
	numberOfAttributes = (userAttributes.begin())->size();
    }

protected:
    /**
     * @brief      Normalize the capital score so that each value is in the range [0,1]
     *
     * @details    Normalize the capital score so that each value is in the range [0,1]
     *
     * @return     return type
     */
    double normalizeCapital()
    {
	double maxCapital = *(std::max_element(this->nodesAchievedCapital.begin(),
					       this->nodesAchievedCapital.end()));
	#pragma omp parallel
	for(unsigned int i=0 ; i<this->nodesAchievedCapital.size() ; ++i)
	    this->nodesAchievedCapital[i] /= maxCapital;

	return maxCapital;
    }

    // /**
    //  * @brief      Build the seed set accountin for the diversity factor
    //  *
    //  * @details    It delegates the derived class for the computation
    //  *
    //  * @param      param
    //  *
    //  * @return     return type
    //  */
    // void buildSeedSetWithDiversity()
    // {
    // 	static_cast<DiversityAwareAlgo*>(this) -> buildSeedSetWithDiversity();
    // }
};
}

#endif

