#ifndef ADITUMALGO_H
#define ADITUMALGO_H

#include <aditum/AditumGraph.hpp>
#include <aditum/Distribution.hpp>
#include <aditum/RandomRRSetGenerator.hpp>
#include <vector>
#include <absl/container/flat_hash_set.h>
#include <math.h>
#include <set>
#include <algorithm>


namespace Aditum
{
    using node = NetworKit::node;
    using edgeweight = NetworKit::edgeweight;

    /**
     * Base class for every ADITUM
     * alorithm
     */
    template <typename SetGenerator>
    class AditumBase: private RandomRRSetGenerator<SetGenerator>
    {
    protected:

	/*!< the aditum graph */
	AditumGraph &aGraph;

	/*!< the capital-aware node distribution */
	Distribution nodeDistribution;

	/*!< budget for the selection */
	int k = 0;

	/*!< balance */
	double alpha = 0;

	/*!< approximation factor */
	double epsilon;

	/*!< probability factor */
	double l;

	/*!< if false the algorthim has not computed the seed set yet */
	bool hasRun = false;

	/*!< seedSet  */
	std::set<node> seedSet;

	/*!< setRootscore[i] is the node root of the i-th RRSet*/ 
	std::vector<double> setRoot;

	/*!< rrsets[i] returns the set of nodes belonging to the i-th RRSET*/ 
	std::vector<absl::flat_hash_set<node>> rrsets;

	/*!< nodeSetIndexes[v] returns the ids of every RRSet v is into */
	std::vector<absl::flat_hash_set<int>> nodeSetIndexes;

	/*!< scores associated with each node */
	std::vector<double> nodeScores;
    
    public:

	AditumBase(AditumGraph aGraph, double alpha, Distribution nodeDist)
	    :nodeSetIndexes {std::vector<absl::flat_hash_set<int>>(aGraph.graph().upperNodeIdBound())},
	     nodeScores {std::vector<double>(aGraph.graph().upperNodeIdBound())}
	{}

	void run()
	{
	    auto logcnk = [=](int n, int k) 
	    {
		double ans = 0;
		for (int i = n - k + 1; i <= n; i++) 
		    ans += std::log(i);

		for (int i = 1; i <= k; i++) 
		    ans -= std::log(i);
		return ans;
	    };

	    // cambia epsilon con epsPrime
	    double epsPrime = 5 * std::pow(epsilon*epsilon/(k+1), 1/3);
	    double kpt = std::max(kptEstimation(), refineKpt(epsPrime));
	    double n = aGraph.graph().numberOfNodes();
	    double theta = (8+2) * n  * (l * std::log(n) + logcnk(n, k) + std::log(2)) / (epsilon * epsilon * kpt);

	    //delete all the rrs sets generated so far
	    reset();

	    // nodeSelection(theta);
	    
	}

	//! Destructor
	virtual ~AditumBase() = default;

    private:

	double kptEstimation() 
	{
	    double m = aGraph.graph().numberOfEdges();
	    double n = aGraph.graph().numberOfNodes();
	    double log2N = log2(n);
	    double logN = log(n);
	    double multi2 = 1;

	    for(int i=0; i<logN-1 ; i++)
	    {
		double ci = (6*logN + 6*log(log2N))*(multi2);
		double sum = 0;

		//generate ci different rrset roots
		auto roots = nodeDistribution.sample(ci);
		
		//expand the vector for storing the rrsets
		int offset = rrsets.size();
		rrsets.resize(offset+roots.size());
		setRoot.resize(offset+roots.size());

		//compute the rrset for each root
		for(auto root : roots)
		{
		    setRoot[offset] = root;
		    int setWidth = 0;
		    static_cast<SetGenerator*>(this) ->
			operator()(aGraph.graph(),
				   root,
				   [&](node src, node, edgeweight){
				       setWidth++;
				       rrsets[offset].emplace(src);
				       nodeSetIndexes[src].emplace(offset);
				       nodeScores[src] += aGraph.score(root);
				   });

		    double kappa = 1 - pow(1-setWidth/m, k);
		    sum += kappa;
		}

		if(sum/ci > 1/multi2)
		    return n * sum / (2*ci); //this is the value of KPT* 
		
		multi2 *= 2;
	    }
	    return 1;
	}


	double refineKpt(double epsPrime)
	{
	    buildSeedSet();
	    reset();
	    int n = aGraph.graph().numberOfNodes();
	    double lambdaPrime = (2+epsPrime)*l*n*log(n)*pow(epsPrime, -2);
	    double kptStar = kptEstimation();
	    double thetaPrime = lambdaPrime/kptStar;

	    //generate thetaPrime new RRSEts and compute the fraction
	    //that are covered by the current seedSet
	    int coveredSets = 0;
	    auto roots = nodeDistribution.sample(thetaPrime);
	    for(auto r : roots)
	    {
		generateRRsets(thetaPrime,
			       [&](node , node, edgeweight){},
			       [&](node v) {
				   auto isContained = seedSet.count(v);
				   if(isContained) coveredSets++;
				   return isContained;
			       });
	    }

	    double kptPrime;
	}


	template <typename F>
	void generateRRsets(int theta, F&& callBack)
	{
	    auto roots = nodeDistribution.sample(theta);
	    for(auto r : roots)
	    {
		static_cast<SetGenerator*>(this) ->
		    operator()(aGraph.graph(), r, std::forward(callBack));
	    }
	}


	/**
	 * @brief      Remove all the steored rrsets
	 *
	 * @details    Reset all the data structures responsible for storing the RRSets
	 *
	 * @return     void
	 */
	inline void reset()
	{
	    rrsets.clear();
	    seedSet.clear();
	    setRoot.clear();

	    for(auto &nodeIndexes : nodeSetIndexes)
		nodeIndexes.clear();
	}

	/**
	 * @brief      Seed Set construction
	 *
	 * @details    It constructs the Seed Set based on the information
	 *
	 * @param      param
	 *
	 * @return     return type
	 */
	void buildSeedSet() 
	{
	    std::vector<Utility::ScoreObject> q(nodeScores.size());
	    //init without diversity
	    for (unsigned int i = 0; i < q.size(); ++i)
		q[i] = Utility::ScoreObject {i, 0, nodeScores[i], 0};

	    //clear the seed set for safety
	    seedSet.clear();

	    while(seedSet.size()<k)
            {
		std::pop_heap(q.begin(), q.end()); //move the largest to end
		Utility::ScoreObject &item = q.back();
		if (item.iteration == seedSet.size())
                {
		    //reduce the score of each node belonging to the same
		    //rrset this node belongs to
		    for(auto setId : nodeSetIndexes[item.node])
			for(auto node : rrsets[setId])
			    nodeScores[node] -= aGraph.score(setRoot[setId]);
		    seedSet.emplace(item.node);
		    q.pop_back();
		}
		else
		{
		    item.capitalScore = nodeScores[item.node];
		    item.iteration = seedSet.size();
		    std::push_heap(q.begin(), q.end());
		}
	    }
	}

	

    };
}


#endif

