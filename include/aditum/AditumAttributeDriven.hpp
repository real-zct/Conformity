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
		AditumAttributeDriven(AditumGraph &graph, Distribution dist,
							  std::vector<std::vector<symbol>> userAttributes,
							  int k, double alpha = 1, double epsilon = 1, double l = 1) : AditumBase<SetGenerator, DiversityAwareAlgo>(graph, dist, k, alpha, epsilon, l), userAttributes{userAttributes}
		{
			numberOfAttributes = (userAttributes.begin())->size();
		}

	protected:
		/**
		 * @brief      Normalize the capital score so that each value is in the range [0,1]
		 *				normalizeCapital 函数的作用是将资本分数规范化到 [0, 1] 的范围内，并返回最大元素值
		 * @details    Normalize the capital score so that each value is in the range [0,1]
		 *
		 * @return     return type
		 */
		double normalizeCapital()
		{
			double maxCapital = *(std::max_element(this->nodesAchievedCapital.begin(),
												   this->nodesAchievedCapital.end()));//max_element用于找到给定范围内的最大元素
#pragma omp parallel
			for (unsigned int i = 0; i < this->nodesAchievedCapital.size(); ++i)
				this->nodesAchievedCapital[i] /= maxCapital;

			return maxCapital;
		}
		double normalizeConformity()
		{
			double maxConformity = *(std::max_element(this->nodesAchievedConformity.begin(),
												   this->nodesAchievedConformity.end()));//max_element用于找到给定范围内的最大元素
#pragma omp parallel
			for (unsigned int i = 0; i < this->nodesAchievedConformity.size(); ++i)
				this->nodesAchievedConformity[i] /= maxConformity;

			return maxConformity;
		}

		//初始化一个包含 Utility::ScoreObject 对象的向量，并将其转换为一个最大堆并返回。
		std::vector<Utility::ScoreObject> getInitialScoreVector()
		{
			// initialize the score vector
			// and set the correct weighting factor for the ScoreObject class
			std::vector<Utility::ScoreObject> q(this->nodesAchievedCapital.size());

#pragma omp parallel
			for (unsigned int i = 0; i < q.size(); ++i)
				q[i] = Utility::ScoreObject{i, 0, this->nodesAchievedCapital[i], this->nodesAchievedConformity[i]};//node，iteration，capitalScore，diversityScore

			// sort q
			std::make_heap(q.begin(), q.end());

			return q;
		}
		//通过结合资本分数和多样性分数，从候选节点中选择一组种子节点。
		void selectionLoop(std::vector<Utility::ScoreObject> &q,
					  double maxCapital, double maxConformity)
		{//insertCallback为一个传入函数，insertCallback(i)即更新结点id为i的属性计数
		//updateDiversity为一个传入函数，updateDiversity(i)即计算结点id为i的边际增益并返回
			this->seedSet.clear();

			while (this->seedSet.size() < this->k)
			{
				std::pop_heap(q.begin(), q.end());
				auto &item = q.back();

				if (item.iteration == this->seedSet.size())
				{
					for (auto setId : this->nodeSetIndexes[item.node])
						for (auto node : this->rrsets[setId]){
							this->nodesAchievedCapital[node] -= this->aGraph.score(this->setRoot[setId]) / maxCapital;
							this->nodesAchievedConformity[node] -= this->nodeRootSim[node][this->setRoot[setId]]/ maxConformity;
						}
					this->seedSet.emplace(item.node);
					q.pop_back();
				}
				else
				{
					item.capitalScore = this->nodesAchievedCapital[item.node];
					item.diversityScore = this->nodesAchievedConformity[item.node];//归一化
					item.iteration = this->seedSet.size();
					std::push_heap(q.begin(), q.end());
				}
			}
		}
		// //通过结合资本分数和多样性分数，从候选节点中选择一组种子节点。
		// template <typename InsertCallback, typename UpdateCallback>
		// std::enable_if_t<
		// 	Utility::function_traits<InsertCallback>::arity == 1 &&
		// 	Utility::function_traits<UpdateCallback>::arity == 1>
		// selectionLoop(std::vector<Utility::ScoreObject> &q,
		// 			  InsertCallback &&insertCallback,
		// 			  UpdateCallback &&updateDiversity,
		// 			  double maxCapital, double maxDiversity)
		// {//insertCallback为一个传入函数，insertCallback(i)即更新结点id为i的属性计数
		// //updateDiversity为一个传入函数，updateDiversity(i)即计算结点id为i的边际增益并返回
		// 	this->seedSet.clear();

		// 	while (this->seedSet.size() < this->k)
		// 	{
		// 		std::pop_heap(q.begin(), q.end());
		// 		auto &item = q.back();

		// 		if (item.iteration == this->seedSet.size())
		// 		{
		// 			for (auto setId : this->nodeSetIndexes[item.node])
		// 				for (auto node : this->rrsets[setId])
		// 					this->nodesAchievedCapital[node] -= this->aGraph.score(this->setRoot[setId]) / maxCapital;

		// 			// add the symbols covered by this node
		// 			insertCallback(item.node);
		// 			this->seedSet.emplace(item.node);
		// 			q.pop_back();
		// 		}
		// 		else
		// 		{
		// 			item.capitalScore = this->nodesAchievedCapital[item.node];
		// 			item.diversityScore = updateDiversity(item.node) / maxDiversity;//归一化
		// 			item.iteration = this->seedSet.size();
		// 			std::push_heap(q.begin(), q.end());
		// 		}
		// 	}
		// }
	};
}

#endif
