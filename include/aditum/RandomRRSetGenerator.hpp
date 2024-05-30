#ifndef RANDOMRRSETGENERATOR_H
#define RANDOMRRSETGENERATOR_H

#include <chrono>
#include <stack>
#include <networkit/graph/Graph.hpp>
#include <SFMT.h>
#include <stdlib.h>
#include <absl/container/flat_hash_set.h>
#include <aditum/FunctionTraits.hpp>
#include <aditum/Utility.hpp>

namespace Aditum
{

	using Graph = NetworKit::Graph;
	using node = NetworKit::node;
	using edgeweight = NetworKit::edgeweight;
	using random_generator = sfmt_t;

	/**
	   Base class for every rr set generator.
	*/
	template <typename T>
	class RandomRRSetGenerator
	{
	protected:
		// random numbers generator
		random_generator gen;

	public:
		RandomRRSetGenerator(unsigned int seed)
		{
			sfmt_init_gen_rand(&gen, seed);
		}

		RandomRRSetGenerator()
		{
			sfmt_init_gen_rand(&gen, Utility::now());
		}

		template <typename F>
		std::enable_if_t<Utility::function_traits<F>::arity == 3>
		operator()(const Graph &g, node root, F &&f)
		{
			static_cast<T *>(this)->operator()(g, root, std::forward<F>(f), [](node)
											   { return false; });
		}

		template <typename F, typename C>
		std::enable_if_t<
			Utility::function_traits<C>::arity == 1 &&
			std::is_same<typename Utility::function_traits<C>::return_type, bool>::value &&
			Utility::function_traits<F>::arity == 3>
		operator()(const Graph &g, node root, F &&f, C &&c)
		{
			static_cast<T *>(this)->operator()(g, root, std::forward<F>(f), std::forward<C>(c));
		}
	};

	class ICRandomRRSetGenerator : public RandomRRSetGenerator<ICRandomRRSetGenerator>
	{
	public:
		template <typename F, typename C>
		std::enable_if_t<
			Utility::function_traits<C>::arity == 1 &&
			std::is_same<typename Utility::function_traits<C>::return_type, bool>::value &&
			Utility::function_traits<F>::arity == 3>
		operator()(const Graph &g, node root, F &&f, C &&stopCondition)
		{
			std::stack<node> s;
			absl::flat_hash_set<node> visited = {root};
			s.push(root);
			do
			{
				const auto v = s.top();
				s.pop();
				// check if the stopCondition is matched
				if (stopCondition(v))
					break;

				g.forInEdgesOf(v, [&](node, node src, edgeweight weight)
							   {
		    //if src has been already processed then return
		    if(visited.contains(src))
			return;
			
		    //check whether or not the edge is live
		    if(sfmt_genrand_real1(&gen)<=weight)
			{
			    // execute the call back over the live edge
			    f(src, v, weight);
			    s.push(src);
			    visited.emplace(src);
			} });

			} while (!s.empty());
		}
	};

	class LTRandomRRSetGenerator : public RandomRRSetGenerator<LTRandomRRSetGenerator>
	{
	public:
		template <typename F, typename C>
		std::enable_if_t<
			Utility::function_traits<C>::arity == 1 &&
			std::is_same<typename Utility::function_traits<C>::return_type, bool>::value &&
			Utility::function_traits<F>::arity == 3>
		/**
		 * std::enable_if_t 是用于模板元编程的 SFINAE（Substitution Failure Is Not An Error）技术。它确保仅当满足某些条件时才启用该操作符：
			Utility::function_traits<C>::arity == 1：确保 stopCondition 函数对象只接受一个参数。
			std::is_same<typename Utility::function_traits<C>::return_type, bool>::value：确保 stopCondition 函数对象的返回类型是 bool。
			Utility::function_traits<F>::arity == 3：确保 F 函数对象接受三个参数。
		*/
		operator()(const Graph & g, node root, F f, C stopCondition)
		{//生成一个RR集并返回
			std::stack<node> s;
			absl::flat_hash_set<node> visited = {root};//visited用来存储已经访问到的点
			s.push(root);
			do
			{
				const auto v = s.top();
				s.pop();

				// check if the stop condition is matched
				if (stopCondition(v))
					break;

				double random = sfmt_genrand_real1(&gen);//生成一个 [0, 1) 范围内的随机浮点数，并将其赋值给变量 random
				g.forInEdgesOf(v, [&](node, node src, edgeweight weight)
				{//每做一次forInEdgesOf方法，就是从结点v反向遍历一步
		    		//if random < 0 v has already been activated by one of
		    		//its in-neighbor in a previous iteration 
		    		if (random<0)
						return;
		    		random -= weight;//随机选中一个入边来反向遍历
		    			//src is the one to activate v
		    		if(random<0 && !visited.contains(src)){
						f(src, v, weight);
						s.push(src);
						visited.emplace(src);
		    		} 
				});

			} while (!s.empty());
		}
	};
}

#endif
