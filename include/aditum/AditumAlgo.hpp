#ifndef ADITUMALGO_H
#define ADITUMALGO_H

#include <aditum/AditumGraph.hpp>
#include <aditum/Distribution.hpp>

namespace Aditum
{

    using node = NetworKit::node;

    /**
     * Base class for every ADITUM
     * alorithm
     */
    class AditumAlgorithm
    {
    protected:

	AditumGraph aGraph;
    
	Distribution nodeDistribution;
    
	double alpha;

	bool hasRun = false;
    
    public:

	AditumAlgorithm(AditumGraph aGraph, double alpha, Distribution nodeDist);

	void run();

    protected:

	//! Destructor
	virtual ~AditumAlgorithm();

	//da usare nella parte finale quando vogliamo calcolare anche la
	//diversity 
	template<typename F>
	void generateRRsets(int size, F f);

	void generateRRSets(int size);

	double estimateActivationProbability();

	double kptEstimation(int k);

	virtual void buildSeedSet(int k);

	
	virtual double computeDiversity() const;

	virtual double computeMarginalDiversity(node v);

    
    };
}


#endif

