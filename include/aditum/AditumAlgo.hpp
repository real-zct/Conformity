
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

		// 记录结点的一致性分数
		std::vector<double> nodesAchievedConformity;

		// 记录结点在RR集中与源节点的一致性分数，一维数表示node id,二维表示root id，存储两结点相似度
		std::vector<std::vector<double>> nodeRootSim;

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
		// AditumBase(AditumGraph &graph, Distribution dist, int k,
		// 		   double alpha = 1, double epsilon = 1, double l = 1)
		// 	: nodeSetIndexes{std::vector<absl::flat_hash_set<int>>(aGraph.graph().upperNodeIdBound())},
		// 	  nodesAchievedCapital{std::vector<double>(aGraph.graph().upperNodeIdBound())},
		// 	  nodeDistribution{dist},
		// 	  alpha{alpha},
		// 	  aGraph{graph}, k{k}, epsilon{epsilon}, l{l} {}
		AditumBase(AditumGraph &graph, Distribution dist, int k,
				   double alpha = 1, double epsilon = 1, double l = 1)
			: nodeSetIndexes{std::vector<absl::flat_hash_set<int>>(aGraph.graph().upperNodeIdBound())},
			  nodesAchievedCapital{std::vector<double>(aGraph.graph().upperNodeIdBound())},
			  nodesAchievedConformity{std::vector<double>(aGraph.graph().upperNodeIdBound())},
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

			// function to compute the algorithm of the binomial coefficient (n k)
			auto logcnk = [=](int n, int k) { // lambda函数，用于计算二项式系数的对数
				double ans = 0;
				for (int i = n - k + 1; i <= n; i++)
					ans += std::log(i);

				for (int i = 1; i <= k; i++)
					ans -= std::log(i);
				return ans;
			};
			// cambia epsilon con epsPrime，用 epsPrime 替换 epsilon，epsPrime是要生成的额外RR集的数量
			double epsPrime = 5 * std::pow(epsilon * epsilon / (k + 1), 1 / 3);

			// set alpha for the score object so that the diversity is neglected
			//  Utility::ScoreObject::setAlpha(1);
			Utility::ScoreObject::alpha = 1;

			// 估算所需RR集的数量，theta为求得的所需RR集的数量
			double kpt = refineKpt(epsPrime);
			double n = aGraph.graph().numberOfNodes();
			double theta = (8 + 2 * epsilon) * n * (l * std::log(n) + logcnk(n, k) + std::log(2)) / (epsilon * epsilon * kpt);

			// enable the diversity aware selection only if
			// alpha has weight less than 1 -- alpha==1 means: only capital
			diversityAware = alpha < 1; // diversityAware为布尔值，用来判定当前是否要加入多样性考量
			// Utility::ScoreObject::setAlpha(this->alpha);
			Utility::ScoreObject::alpha = this->alpha;
			if (diversityAware)
			{
				nodeSelectionWithConformity(theta);
			}
			else
			{
				nodeSelection(theta);
			}

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
			// 生成RR集然后计算最终的种子集
			//  reset all data structures
			reset();

			auto roots = nodeDistribution.sample(theta); // 根据结点资本分数进行源节点采样,选择theta个源节点
			// expand the vector for storing the rrsets
			int offset = 0;
			rrsets.resize(roots.size());
			setRoot.resize(roots.size());

			for (auto root : roots)
			{							// 有一次循环就生成一个RR集合
				setRoot[offset] = root; // root是结点id，offset可以看作是RR集id，setRoot存储RR集源节点
				static_cast<RandomRRSetGenerator<SetGenerator> *>(this)->
				operator()(aGraph.graph(),
						   root,
						   [&](node src, node, edgeweight)
						   {
							   rrsets[offset].emplace(src);						// 为RR集增加结点
							   nodeSetIndexes[src].emplace(offset);				// 给结点标记所属RR集的id
							   nodesAchievedCapital[src] += aGraph.score(root); // nodesAchievedCapital存储结点覆盖RR集的分数
																				//    nodesAchievedConformity[src] +=();
						   });
				++offset;
			}
			buildSeedSet();
		}
		void nodeSelectionWithConformity(double theta)
		{
			// 生成RR集然后计算最终的种子集
			//  reset all data structures
			reset();
			//test，设置为选点概率仅和capital有关
			//auto roots = nodeDistribution.sample(theta); // 根据结点资本分数进行源节点采样,选择theta个源节点
			auto roots = nodeDistribution.sample(theta, 1); // 根据结点资本分数与出度乘积来进行源节点采样,选择theta个源节点
			// expand the vector for storing the rrsets
			int offset = 0;
			rrsets.resize(roots.size());
			setRoot.resize(roots.size());
			this->nodeRootSim.resize(aGraph.graph().upperNodeIdBound(), std::vector<double>(roots.size(), -1.0));
			for (auto root : roots)
			{							// 有一次循环就生成一个RR集合
				setRoot[offset] = root; // root是结点id，offset可以看作是RR集id，setRoot存储RR集源节点
				static_cast<RandomRRSetGenerator<SetGenerator> *>(this)->
				operator()(aGraph.graph(),
						   root,
						   [&](node src, node, edgeweight)
						   {
							   rrsets[offset].emplace(src);						// 为RR集增加结点
							   nodeSetIndexes[src].emplace(offset);				// 给结点标记所属RR集的id
							   nodesAchievedCapital[src] += aGraph.score(root); // nodesAchievedCapital存储结点覆盖RR集的分数
							   this->countSim(src, root);
							   nodesAchievedConformity[src] += nodeRootSim[src][root];
						   });
				++offset;
			}
			buildSeedSetWithConformity();
			// buildSeedSetWithDiversity();
		}
		double countSim(unsigned int node, unsigned int root)
		{
			if (this->nodeRootSim[root][node] != -1.0){
				this->nodeRootSim[node][root] = this->nodeRootSim[root][node];
			}else{
				this->nodeRootSim[node][root] = static_cast<DiversityAwareAlgo *>(this)->countSim(node, root);
			}
			
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
			return this->seedSet;
		}
		double getSeedsCapital_rootCapitalCovProb(double targetThreshold)
		{
			//print_rrsets_with_roots(rrsets,setRoot);
			// 记录种子集合的资本分数
			// double cumulateCapital=0.0;
			std::set<node> seeds = this->getSeeds();
			std::vector<int> rrsetsVisited(this->rrsets.size(), 0);
			// std::vector<int> targetNodeVisited(aGraph.graph().upperNodeIdBound(),0);
			double rootCapitalCov = 0.0;
			// double rrsetCovNum=0.0;
			for (auto seed : seeds)
			{
				for (auto setId : nodeSetIndexes[seed])
				{ // setId：item所属RR集id的集合
					if (rrsetsVisited[setId] == 0)
					{
						// rrsetCovNum+=1;
						rootCapitalCov += aGraph.score(setRoot[setId]);
						rrsetsVisited[setId] = 1;
					}
				}
			}
			double rootCapitalTotal = 0.0;
			for (auto i = 0; i < this->rrsets.size(); i++)
			{
				rootCapitalTotal += aGraph.score(setRoot[i]);
			}
			double capitalTotal = 0.0;
			for (auto i = 0; i < aGraph.graph().upperNodeIdBound(); i++)
			{
				auto item = aGraph.score(i);
				if (item >= targetThreshold)
				{
					capitalTotal += item;
				}
			}
			return capitalTotal * rootCapitalCov / rootCapitalTotal;
		}
		double getSeedsCapital_rrsetNumCovProb(double targetThreshold)
		{
			//print_rrsets_with_roots(rrsets,setRoot);
			// 记录种子集合的资本分数
			// double cumulateCapital=0.0;
			std::set<node> seeds = this->getSeeds();
			std::vector<int> rrsetsVisited(this->rrsets.size(), 0);
			// std::vector<int> targetNodeVisited(aGraph.graph().upperNodeIdBound(),0);
			double rrsetCovNum=0.0;
			for (auto seed : seeds)
			{
				for (auto setId : nodeSetIndexes[seed])
				{ // setId：item所属RR集id的集合
					if (rrsetsVisited[setId] == 0)
					{
						rrsetCovNum+=1;
						rrsetsVisited[setId] = 1;
					}
				}
			}
			double capitalTotal = 0.0;
			for (auto i = 0; i < aGraph.graph().upperNodeIdBound(); i++)
			{
				auto item = aGraph.score(i);
				if (item >= targetThreshold)
				{
					capitalTotal += item;
				}
			}
			return capitalTotal * rrsetCovNum / rrsets.size();
		}
		double getSeedsCapital_rrsetCovRootCapitalCum()
		{
			//print_rrsets_with_roots(rrsets,setRoot);
			// 记录种子集合的资本分数
			double cumulateCapital=0.0;
			std::set<node> seeds = this->getSeeds();
			std::vector<int> rrsetsVisited(this->rrsets.size(), 0);
			std::vector<int> targetNodeVisited(aGraph.graph().upperNodeIdBound(),0);
			
			for (auto seed : seeds)
			{
				for (auto setId : nodeSetIndexes[seed])
				{ // setId：item所属RR集id的集合
					if (rrsetsVisited[setId] == 0)
					{
						auto rootId=setRoot[setId];
						if(targetNodeVisited[rootId]==0){
							cumulateCapital+=aGraph.score(rootId);
							targetNodeVisited[rootId]=1;
						}
						rrsetsVisited[setId] = 1;
					}
				}
			}
			return cumulateCapital;
		}
		void print_rrsets_with_roots(const std::vector<absl::flat_hash_set<node>> &rrsets, const std::vector<double> &setRoot)
		{
			if (rrsets.size() != setRoot.size())
			{
				std::cerr << "Error: rrsets and setRoot must be of the same size." << std::endl;
				return;
			}

			for (size_t i = 0; i < rrsets.size(); ++i)
			{
				std::cout << "Set " << i << " (Root: " << setRoot[i] << "): {";
				const auto &set = rrsets[i];
				for (auto it = set.begin(); it != set.end(); ++it)
				{
					if (it != set.begin())
					{
						std::cout << ", ";
					}
					std::cout << *it;
				}
				std::cout << "}" << std::endl;
			}
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
		void buildSeedSetWithConformity()
		{
			static_cast<DiversityAwareAlgo *>(this)->buildSeedSetWithConformity();
		}
		// void buildSeedSetWithDiversity()
		// {
		// 	static_cast<DiversityAwareAlgo *>(this)->buildSeedSetWithDiversity();
		// }
		double LTMonteCarloEstimationOfCapital(double targetThreshold, const std::set<node> &S, int IMC)
		{
			double curr_C = 0.0;						  // 记录当前累积激活的概率分数
			std::unordered_map<node, bool> isActive;	  // 标记是否是激活状态
			std::unordered_map<node, double> receivedInf; // 标记结点积累的概率值
			std::unordered_map<node, double> vThreshold;  // 记录结点随机生成的激活概率
			std::mt19937 gen(std::random_device{}());
			std::uniform_real_distribution<> dis(0.0, 1.0);

			auto reset = [&](const std::set<node> &S)
			{
				aGraph.graph().forNodes([&](node v)
										{
					if (S.find(v) == S.end()) // 只对不在集合S中的节点进行操作。
					{						  // 如果未找到该元素，S.find(v)结果会指向 S.end()
						isActive[v] = false;
						receivedInf[v] = 0;
						vThreshold[v] = dis(gen);
					}else {
                		isActive[v] = true;
                		receivedInf[v] = 1.0; // 确保种子节点一开始就是激活的
                		vThreshold[v] = 0.0; // 种子节点不需要阈值
            		} });
			};
			for (int j = 0; j < IMC; ++j)
			{
				reset(S);
				std::queue<node> temp;
				for (node u : S)
				{
					isActive[u] = true;
					temp.push(u);
				}

				while (!temp.empty())
				{
					node u = temp.front();
					temp.pop();

					aGraph.graph().forNeighborsOf(u, [&](node v)
												  {
                		if (!isActive[v]) {
                    		receivedInf[v] += aGraph.graph().weight(u, v);
                    		if (receivedInf[v] >= vThreshold[v]) {
                        		isActive[v] = true;
                        		temp.push(v);
                        		if (aGraph.score(v) >= targetThreshold) {
                            		curr_C += aGraph.score(v);
                        		}
                    		  }
                		} });
				}
			}

			return curr_C / IMC;
		}
		double ICMonteCarloEstimationOfCapital(double targetThreshold, const std::set<node> &S, int IMC)
		{
			double total_C = 0.0;					 // 记录当前累积激活的概率分数
			std::unordered_map<node, bool> isActive; // 标记是否是激活状态
			std::mt19937 gen(std::random_device{}());
			std::uniform_real_distribution<> dis(0.0, 1.0);
			auto reset = [&](const std::set<node> &S)
			{
				aGraph.graph().forNodes([&](node v)
										{
            		if (S.find(v) == S.end()) {// 如果未找到该元素，S.find(v)结果会指向 S.end().只对不在集合S中的节点进行操作。
                		isActive[v] = false;
            		}else{
						isActive[v] = true; // 确保种子节点是激活的
					} });
			};

			for (int j = 0; j < IMC; ++j)
			{
				double cur_C=0.0;
				reset(S);
				std::queue<node> temp;
				for (node u : S)
				{
					isActive[u] = true;
					temp.push(u);
				}

				while (!temp.empty())
				{
					node u = temp.front();
					temp.pop();
					aGraph.graph().forNeighborsOf(u, [&](node v)
												  {
                		if (!isActive[v]) {
                    		double prob = aGraph.graph().weight(u, v);
                    		if (dis(gen) <= prob) {
                        		isActive[v] = true;
                        		temp.push(v);
                        		if (aGraph.score(v) >= targetThreshold) {
                            		cur_C += aGraph.score(v);
                        		}
                    		}
                		} });
				}
				total_C+=cur_C;
			}

			return total_C / IMC;
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
			// 首先生成数量相对较少的 RR 集，并在此基础上计算出预期传播的初始近似值。反复增加 RR 集的数量，直到估计值符合一定的误差范围。
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
			// 根据 KPTEstimation 最后一次迭代中生成的随机 RR 集计算初始种子集，并根据新选择的 RR 集估算初始种子集的扩散；
			// 这些新 RR 集的数量保持在合理的高数量，以确保最后一次估算的准确性。最后，RefineKPT会返回第一个近似值和最后一个近似值中的最大值。
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

			// if (diversityAware)
			// {
			// 	buildSeedSetWithConformity();
			// 	return;
			// }

			std::vector<Utility::ScoreObject> q(nodesAchievedCapital.size());
			std::vector<int> rrsetsVisited(this->rrsets.size(), 0);
// init without diversity
#pragma omp parallel
			for (unsigned int i = 0; i < q.size(); ++i)
				q[i] = Utility::ScoreObject{i, 0, nodesAchievedCapital[i], 0}; // node，iteration，capitalScore，diversityScore

			std::make_heap(q.begin(), q.end()); // 将向量 q 转换为一个最大堆，堆顶元素是范围内的最大值
			seedSet.clear();
			while (seedSet.size() < k)
			{
				std::pop_heap(q.begin(), q.end());	   // pop_heap用于将堆顶元素移动到堆的末尾，并重新调整剩余的元素，使其仍然满足堆的性质
				Utility::ScoreObject &item = q.back(); // back用于访问向量（vector）中的最后一个元素,item为分数最大的点的ScoreObject对象。
				if (item.iteration == seedSet.size())
				{ // 说明当前item的数值为最新值
					// reduce the score of each node belonging to the same
					// rrset this node belongs to
					for (auto setId : nodeSetIndexes[item.node])
					{ // setId：item所属RR集id的集合
						if (rrsetsVisited[setId] == 0)
						{
							for (auto node : rrsets[setId])
							{
								nodesAchievedCapital[node] -= aGraph.score(setRoot[setId]); // 为所有受新种子结点影响的结点更新资本分数
							}
							rrsetsVisited[setId] = 1;
						}
					}
					seedSet.emplace(item.node); // 将item加入到种子集合中去
					q.pop_back();
				}
				else
				{ // 懒惰更新，iteration和seedSet.size()不等说明item不是最新值，需要更新
					item.capitalScore = nodesAchievedCapital[item.node];
					item.iteration = seedSet.size();
					std::push_heap(q.begin(), q.end()); // 维护堆的性质。范围 [begin, end-1) 已经是一个有效的堆。它会将范围内的最后一个元素（即位于 end-1 的元素）插入到堆中合适的位置，从而使整个范围 [begin, end) 变成一个有效的堆。
				}
			}
		}
	};

}

#endif
