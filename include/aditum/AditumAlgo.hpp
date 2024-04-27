
/**
 *   \file AditumAlgo.hpp
 *   \brief File containing the base version of the aditum algorithm
 *
 * This file contains the base version of the aditum algorithm
 *
 */

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
#include <iostream>

namespace Aditum
{

	using node = NetworKit::node;
	using edgeweight = NetworKit::edgeweight;

	/**
	 * Base class for every ADITUM
	 * alorithm
	 */
	template <typename SetGenerator, typename DiversityAwareAlgo>
	class AditumBase : private RandomRRSetGenerator<SetGenerator>
	{
	protected:
		/*!< the aditum graph */
		AditumGraph &aGraph;

		/*!< the capital-aware node distribution */
		Distribution nodeDistribution;

		/*!< if false the seeds selection does not need to account for the diversity */
		bool diversityAware = false;

		/*!< budget for the selection */
		int k = 0;

		/*!< balance between capital and diversity */
		double alpha = 0;

		/*!< approximation factor */
		double epsilon = 1;

		/*!< probability factor */
		double l = 1;

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
		std::vector<double> nodesAchievedCapital;

		/**
		 * @brief      Constructor
		 *
		 * @details    Constructor
		 *
		 * @param      graph AditumGraph
		 * @param      dist Distribution of nodes which takes into account the capital score of each node
		 * @param      k budget for the selection of the seed nodes
		 * @param      alpha weight of the capital score contribution for the objective function
		 * @param      epsilon approximation ration factor
		 * @param      l accuracy of the algoritm

		 *
		 * @return     return type
		 */
		AditumBase(AditumGraph &graph, Distribution dist, int k,
				   double alpha = 1, double epsilon = 1, double l = 1)
			: nodeSetIndexes{std::vector<absl::flat_hash_set<int>>(aGraph.graph().upperNodeIdBound())},
			  nodesAchievedCapital{std::vector<double>(aGraph.graph().upperNodeIdBound())},
			  nodeDistribution{dist},
			  alpha{alpha},
			  aGraph{graph}, k{k}, epsilon{epsilon}, l{l} {}

	public:
		/**
		 * @brief      Destructor
		 *
		 * @details    Destructor
		 */
		virtual ~AditumBase() = default;

		/**
		 * @brief      Method that computes the seed set
		 *
		 * @details    It exectutes the entire aditum algorithm
		 *
		 */
		void run()
		{

			// function to compute the logarithm of the binomial coefficient (n k)
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
			double epsPrime = 5 * std::pow(epsilon * epsilon / (k + 1), 1 / 3);

			// set alpha for the score object so that the diversity is neglected
			//  Utility::ScoreObject::setAlpha(1);
			Utility::ScoreObject::alpha = 1;
			double kpt = refineKpt(epsPrime);

			// //compute the number of final RR sets required
			double n = aGraph.graph().numberOfNodes();

			double theta = (8 + 2 * epsilon) * n * (l * std::log(n) + logcnk(n, k) + std::log(2)) / (epsilon * epsilon * kpt);
			// enable the diversity aware selection only if
			// alpha has weight less than 1 -- alpha==1 means: only capital
			diversityAware = alpha < 1;
			// Utility::ScoreObject::setAlpha(this->alpha);
			Utility::ScoreObject::alpha = this->alpha;
			nodeSelection(theta);
			hasRun = true;
		}

		/**
		 * @brief      Final step of the aditum algorithm
		 *
		 * @details    It generates theta rrseta and then it computes the final seed set
		 *
		 * @param      theta, number of required RRSets
		 *
		 */
		void nodeSelection(double theta)
		{
			// reset all data structures
			reset();

			auto roots = nodeDistribution.sample(theta);
			// expand the vector for storing the rrsets
			int offset = 0;
			rrsets.resize(roots.size());
			setRoot.resize(roots.size());

			for (auto root : roots)
			{
				setRoot[offset] = root;
				static_cast<RandomRRSetGenerator<SetGenerator> *>(this)->
				operator()(aGraph.graph(),
						   root,
						   [&](node src, node, edgeweight)
						   {
							   rrsets[offset].emplace(src);
							   nodeSetIndexes[src].emplace(offset);
							   nodesAchievedCapital[src] += aGraph.score(root);
						   });
				++offset;
			}
			buildSeedSet();
		}

		/**
		 * @brief      It returns the final seed set
		 *
		 * @details    It returns the final seed set
		 *
		 * @param      param
		 *
		 * @return     return the seed set computed by the algorithm
		 */
		std::set<node> getSeeds()
		{
			if (!hasRun)
				throw std::runtime_error("You must call run() first!");
			return seedSet;
		}

		/**
		 * @brief      Build the seed set accounting for the diversity factor
		 *
		 * @details    This method implements the lazy forward selection as -- @see buildSeedSet.
		 *             Only this time the value of capital and diversity need to be normalized so
		 *             that they both are within the range [0,1]. This is needed as diversity and
		 *             capital usually have different scale
		 *
		 *
		 * @param      param
		 *
		 * @return     return type
		 */
		void buildSeedSetWithDiversity()
		{
			static_cast<DiversityAwareAlgo *>(this)->buildSeedSetWithDiversity();
		}

	private:
		/**
		 * @brief      Provides the first kpt estimation as in ALgorithm 2 of the TIM paper
		 *
		 * @details    Provides the first kpt estimation
		 *
		 * @return     first estimate on the number of active nodes
		 */
		double kptEstimation()
		{
			double m = aGraph.graph().numberOfEdges();
			double n = aGraph.graph().numberOfNodes();
			double log2N = log2(n);
			double logN = log(n);
			double multi2 = 1;

			for (int i = 0; i < log2N - 1; i++)
			{
				double ci = (6 * l * logN + 6 * log(log2N)) * (multi2);
				double sum = 0;

				// generate ci different rrset roots
				auto roots = nodeDistribution.sample(ci);

				// expand the vector for storing the rrsets
				int offset = 0;
				reset();
				rrsets.resize(roots.size());
				setRoot.resize(roots.size());

				// compute the rrset for each root
				for (auto root : roots)
				{
					setRoot[i] = root;

					int setWidth = 0;
					static_cast<RandomRRSetGenerator<SetGenerator> *>(this)->
					operator()(aGraph.graph(),
							   root,
							   [&](node src, node, edgeweight)
							   {
								   setWidth += aGraph.graph().degreeIn(src);
								   rrsets[i].emplace(src);
								   nodeSetIndexes[src].emplace(offset);
								   nodesAchievedCapital[src] += aGraph.score(root);
							   });

					double kappa = 1 - pow(1 - setWidth / m, k);
					sum += kappa;
					++offset;
				}

				if (sum / ci > 1 / multi2)
					return n * sum / (2 * ci); // this is the value of KPT*

				multi2 *= 2;
			}
			return 1;
		}

		/**
		 * @brief      It provides a better estimate of the number of active nodes, as in Algo. 3 of TIM
		 *
		 * @details    It improves the accuracy of the estimate provided by the kptEstimation method
		 *
		 * @param      epsPrime is the number of additional RR sets to be generated
		 *
		 * @return     the expected spread etimation
		 */
		double refineKpt(double epsPrime)
		{

			double kptStar = kptEstimation();
			buildSeedSet();

			// remove all the information related to these RR-Sets
			reset();

			int n = aGraph.graph().numberOfNodes();
			double lambdaPrime = (2 + epsPrime) * l * n * log(n) * pow(epsPrime, -2);

			double thetaPrime = lambdaPrime / kptStar;

			// generate new thetaPrime RRSEts and compute the fraction
			// that are covered by the current seedSet
			int coveredSets = 0;
			assert(seedSet.size() > 0);
			auto roots = nodeDistribution.sample(thetaPrime);
			for (auto root : roots)
			{
				static_cast<RandomRRSetGenerator<SetGenerator> *>(this)->
				operator()(aGraph.graph(), root, [&](node, node, edgeweight) {}, [&](node v)
						   {
				    auto isContained = seedSet.count(v)>0;
				    if(isContained)
					++coveredSets;
				    return isContained; });
			}
			double coveredFraction = coveredSets / thetaPrime;

			return std::max(coveredFraction * n / (1 + epsPrime), kptStar);
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
			setRoot.clear();
			rrsets.clear(); // remove all the rr sets

#pragma omp parallel
			for (auto &nodeIndexes : nodeSetIndexes)
				nodeIndexes.clear();

#pragma omp parallel
			for (auto &score : nodesAchievedCapital)
				score = 0;
		}

		/**
		 * @brief      Seed Set construction
		 *
		 * @details    It constructs the Seed Set based on the current RRSets
		 *             without considering the diversity contribution
		 *
		 * @param      param
		 *
		 * @return     return type
		 */
		void buildSeedSet()
		{

			if (diversityAware)
			{
				buildSeedSetWithDiversity();
				return;
			}

			std::vector<Utility::ScoreObject> q(nodesAchievedCapital.size());
// init without diversity
#pragma omp parallel
			for (unsigned int i = 0; i < q.size(); ++i)
				q[i] = Utility::ScoreObject{i, 0, nodesAchievedCapital[i], 0};

			std::make_heap(q.begin(), q.end());

			seedSet.clear();
			while (seedSet.size() < k)
			{
				std::pop_heap(q.begin(), q.end()); // move the largest to end
				Utility::ScoreObject &item = q.back();
				if (item.iteration == seedSet.size())
				{
					// reduce the score of each node belonging to the same
					// rrset this node belongs to
					for (auto setId : nodeSetIndexes[item.node])
						for (auto node : rrsets[setId])
							nodesAchievedCapital[node] -= aGraph.score(setRoot[setId]);
					seedSet.emplace(item.node);
					q.pop_back();
				}
				else
				{
					item.capitalScore = nodesAchievedCapital[item.node];
					item.iteration = seedSet.size();
					std::push_heap(q.begin(), q.end());
				}
			}
		}
	};

}

#endif
