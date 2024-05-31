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
	class AttributeWise : public AditumAttributeDriven<SetGenerator, AttributeWise<SetGenerator>>
	{
	private:
		/*!< power coefficient */
		double lam;

		using symbol = typename AditumAttributeDriven<SetGenerator, AttributeWise<SetGenerator>>::symbol;
		//symbol是std::variant<int, std::string>变体
	public:
		using Base = AditumAttributeDriven<SetGenerator, AttributeWise<SetGenerator>>;

		AttributeWise(AditumGraph &graph, Distribution dist,
					  std::vector<std::vector<symbol>> userAttributes,
					  int k, double lambda = 1, double alpha = 1, double epsilon = 1, double l = 1) : Base(graph, dist, userAttributes, k, alpha, epsilon, l), lam{lambda}
		{
		}
		double countSim(unsigned int node,unsigned int root){
			if(node==root){
				return 1.0;
			}
			double simCount=0;
			for(unsigned int i = 0;i < this->numberOfAttributes;i++){
				//if (std::holds_alternative<int>(this->userAttributes[node][i]) && std::holds_alternative<int>(this->userAttributes[root][i])) {
    				simCount+=(std::get<int>(this->userAttributes[node][i]) * std::get<int>(this->userAttributes[root][i]));
				//}
				
			}
			return simCount/this->numberOfAttributes;
			
		}
		void buildSeedSetWithConformity()
		{
			// initialize the map for storing the number of nodes 
			// having a particular attribute in their profile
			// std::vector<absl::flat_hash_map<symbol, int>> coveredAttributes(this->numberOfAttributes);//coveredAttributes存储节点属性中某个符号的计数
			
			// // 更新节点id为node的属性符号计数
			// auto updateCoveredSymbols = [&](unsigned int node)
			// {
			// 	// update the of covered symbols
			// 	auto &attributes = this->userAttributes[node];
			// 	auto itSymbol = attributes.begin();
			// 	for (unsigned int i = 0; itSymbol != attributes.end(); ++i, ++itSymbol)
			// 		coveredAttributes[i][*itSymbol] += 1;
			// };

			// auto computeMarginalDiversity = [&](unsigned int node)
			// {//计算结点id为node的边际增益并返回
			// 	double marginalGain = 0;
			// 	auto &attributes = this->userAttributes[node];
			// 	auto itSymbols = attributes.begin();
			// 	for (unsigned int i = 0; itSymbols != attributes.end(); ++i, ++itSymbols)
			// 		try
			// 		{
			// 			int alreadyPresent = coveredAttributes[i].at(*itSymbols);
			// 			marginalGain += std::pow(alreadyPresent + 1, -lam);
			// 		}
			// 		catch (std::exception &e)
			// 		{
			// 			marginalGain += 1;
			// 		}
			// 	return marginalGain;
			// };

			// compute the maximum value for the marginal gain
			// wrt to the capital score of a node and its diversity
			//-- the maximum marginal gain is simply the greatest score as a singleton
			double maxCapital = this->normalizeCapital();//maxCapital为资本分数的最大值
			double maxConformity = this->normalizeConformity();

			// initialize the score vector
			auto q = this->getInitialScoreVector();
			this->selectionLoop(q, maxCapital, maxConformity);//通过结合资本分数和多样性分数，从候选节点中选择一组种子节点。
		}
		// void buildSeedSetWithDiversity()
		// {
		// 	// initialize the map for storing the number of nodes 
		// 	// having a particular attribute in their profile
		// 	std::vector<absl::flat_hash_map<symbol, int>> coveredAttributes(this->numberOfAttributes);//coveredAttributes存储节点属性中某个符号的计数

		// 	// 更新节点id为node的属性符号计数
		// 	auto updateCoveredSymbols = [&](unsigned int node)
		// 	{
		// 		// update the of covered symbols
		// 		//对于每个属性符号，在 coveredAttributes 向量中的对应映射中增加计数。i是对应属性序号，*itSymbol 解引用迭代器以获取当前的 symbol（属性值，int或者String）。这个 symbol 的计数在 absl::flat_hash_map 中加1。
		// 		auto &attributes = this->userAttributes[node];
		// 		auto itSymbol = attributes.begin();
		// 		for (unsigned int i = 0; itSymbol != attributes.end(); ++i, ++itSymbol)
		// 			coveredAttributes[i][*itSymbol] += 1;
		// 	};

		// 	auto computeMarginalDiversity = [&](unsigned int node)
		// 	{//计算结点id为node的边际增益并返回
		// 		double marginalGain = 0;
		// 		auto &attributes = this->userAttributes[node];
		// 		auto itSymbols = attributes.begin();
		// 		for (unsigned int i = 0; itSymbols != attributes.end(); ++i, ++itSymbols)
		// 			try
		// 			{
		// 				int alreadyPresent = coveredAttributes[i].at(*itSymbols);
		// 				marginalGain += std::pow(alreadyPresent + 1, -lam);
		// 			}
		// 			catch (std::exception &e)
		// 			{
		// 				marginalGain += 1;
		// 			}
		// 		return marginalGain;
		// 	};

		// 	// compute the maximum value for the marginal gain
		// 	// wrt to the capital score of a node and its diversity
		// 	//-- the maximum marginal gain is simply the greatest score as a singleton
		// 	double maxCapital = this->normalizeCapital();//maxCapital为资本分数的最大值
		// 	double maxDiversity = (this->userAttributes.begin())->size();//maxDiversity为用户属性集合中符号的最大数量

		// 	// initialize the score vector
		// 	auto q = this->getInitialScoreVector();
		// 	this->selectionLoop(q, updateCoveredSymbols, computeMarginalDiversity, maxCapital, maxDiversity);//通过结合资本分数和多样性分数，从候选节点中选择一组种子节点。
		// }
	};

	/**
	 * @brief      Builder for the attribute wise algorithm
	 *
	 * @details    Builder for the attribute wise algorithm
	 */
	class AttributeWiseBuilder : public AditumBuilder<AttributeWiseBuilder>
	{
	public:
		template <typename SetGenerator, typename DiversityAwareAlgo>
		AditumBase<SetGenerator, DiversityAwareAlgo> *build()
		{
			Distribution nodeDistribution = computeDistribution();
			// Distribution nodeCAndDDistribution = computeCAndDDistribution();
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
