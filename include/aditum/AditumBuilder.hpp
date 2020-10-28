#ifndef ADITUMBUILDER_H
#define ADITUMBUILDER_H

#include <aditum/AditumAlgo.hpp>

namespace Aditum
{

/**
 * This class helps the construction of an aditum algorithm instance
 * 
 */
template<typename ConcreteBuilder>
class AditumBuilder
{
protected:
    /*!< the aditum graph */
    AditumGraph *aGraph = nullptr;

    /*!< the budget */
    int k;

    /*!< weight of the capital contribution */
    double alpha = 1;

    /*!< approximation ratio */
    double epsilon = 1;
    
    /*!< accuracy */
    double l = 1;

    /*!< target threshold for a node to be regarded as target */
    double targetThreshold;

    /*!< vector containing the users categorical attributes */
    std::vector<std::vector<std::variant<int, std::string>>> userAttributes;

    /*!< Coefficient for the attribute wise diversity algorithm */
    double lambda = 1;

    /*!< Radius for the hamming ball diversity algorithm */
    int radius = 0;

public:

    /**
     * Set the graph for the algorithm
     */
    auto& setGraph(AditumGraph& graph)
    {
	aGraph = &graph;
	return *this;
    }

    /**
     * Set the budget for the algorithm
     */
    auto&  setK(int k)
    {
	this->k = k;
	return *this;
    }

    /**
     * Set the weight of the capital constribution
     */
    auto& setAlpha(double alpha)
    {
	this->alpha = alpha;
	return *this;
    }


    /**
     * Set the approximation ratio
     */
    auto& setEpsilon(double epsilon)
    {
	this->epsilon = epsilon;
	return *this;
    }

    /**
     * Set the accuracy of the algorithm
     */
    auto& setL(double l)
    {
	this->l = l;
	return *this;
    }


    /**
     * Set the target threshold
     */
    auto& setTargetThreshold(double threshold)
    {
	this->targetThreshold = threshold;
	return *this;
    }

    /**
     * Set the users categorical attributes vector
     */
    auto& setAttributes(std::vector<std::vector<std::variant<int, std::string>>> &attributes)
    {
	userAttributes = attributes;
	return *this;
    }

    auto& setLambda(double lambda)
    {
	lambda = lambda;
	return *this;
    }

    /**
     * Set the radius for the hamming based algorithm
     */
    auto& setRadius(int radius)
    {
	radius = radius;
	return *this;
    }

    /**
     * Create the aditum algorithm
     */
    template<typename SetGenerator, typename DiversityAwareAlgo>
    AditumBase<SetGenerator, DiversityAwareAlgo>* build()
    {
	return static_cast<ConcreteBuilder*> (this) -> template build<SetGenerator, DiversityAwareAlgo>();
    }

protected:

    Distribution computeDistribution() {
	//create the distribution
	std::vector<double>scoreVector(aGraph->scores().size());

	#pragma omp parallel
	for(unsigned int i=0 ; i<scoreVector.size() ; i++)
	    if(double iScore = aGraph->score(i);
	       iScore >= targetThreshold)
		scoreVector[i] = iScore;
	
	Distribution nodeDistribution(scoreVector);
	return nodeDistribution;
    }

};

    
};


#endif

