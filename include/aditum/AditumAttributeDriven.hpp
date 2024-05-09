#ifndef ADITUMATTRIBUTEDRIVEN_H
#define ADITUMATTRIBUTEDRIVEN_H
#include <vector>
#include <string>
#include <variant>
#include <algorithm>
#include <aditum/AditumAlgo.hpp>
#include <aditum/FunctionTraits.hpp>
namespace Aditum
{
	/**
	 *  Super class for every Attribute driven
	 *  aditum implementation
	 */
	template <typename SetGenerator, typename DiversityAwareAlgo>
	class AditumAttributeDriven : public AditumBase<SetGenerator, DiversityAwareAlgo>
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
		AditumAttributeDriven(AditumGraph &graph, Distribution dist,Distribution nodeCAndDDistribution,
							  std::vector<std::vector<symbol>> userAttributes,
							  int k, double alpha = 1, double epsilon = 1, double l = 1) : AditumBase<SetGenerator, DiversityAwareAlgo>(graph, dist,nodeCAndDDistribution, k, alpha, epsilon, l), userAttributes{userAttributes}
		{
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
			for (unsigned int i = 0; i < this->nodesAchievedCapital.size(); ++i)
				this->nodesAchievedCapital[i] /= maxCapital;

			return maxCapital;
		}

		std::vector<Utility::ScoreObject> getInitialScoreVector(double maxCapital, double maxDiversity)
		{
			// initialize the score vector
			// and set the correct weighting factor for the ScoreObject class
			std::vector<Utility::ScoreObject> q(this->nodesAchievedCapital.size());

#pragma omp parallel
			for (unsigned int i = 0; i < q.size(); ++i)
				q[i] = Utility::ScoreObject{i, 0, this->nodesAchievedCapital[i], 1};

			// sort q
			std::make_heap(q.begin(), q.end());

			return q;
		}

		// function traits
		template <typename InsertCallback, typename UpdateCallback>
		std::enable_if_t<
			Utility::function_traits<InsertCallback>::arity == 1 &&
			Utility::function_traits<UpdateCallback>::arity == 1>
		selectionLoop(std::vector<Utility::ScoreObject> &q,
					  InsertCallback &&insertCallback,
					  UpdateCallback &&updateDiversity,
					  double maxCapital, double maxDiversity)
		{
			this->seedSet.clear();

			while (this->seedSet.size() < this->k)
			{
				std::pop_heap(q.begin(), q.end());
				auto &item = q.back();

				if (item.iteration == this->seedSet.size())
				{
					for (auto setId : this->nodeSetIndexes[item.node])
						for (auto node : this->rrsets[setId])
							this->nodesAchievedCapital[node] -= this->aGraph.score(this->setRoot[setId]) / maxCapital;

					// add the symbols covered by this node
					insertCallback(item.node);
					this->seedSet.emplace(item.node);
					q.pop_back();
				}
				else
				{
					item.capitalScore = this->nodesAchievedCapital[item.node];
					item.diversityScore = updateDiversity(item.node) / maxDiversity;
					item.iteration = this->seedSet.size();
					std::push_heap(q.begin(), q.end());
				}
			}
		}
	};
}

#endif
