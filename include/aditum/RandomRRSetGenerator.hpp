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
		operator()(const Graph &g, node root, F f, C stopCondition)
		{
			std::stack<node> s;
			absl::flat_hash_set<node> visited = {root};
			s.push(root);
			do
			{
				const auto v = s.top();
				s.pop();

				// check if the stop condition is matched
				if (stopCondition(v))
					break;

				double random = sfmt_genrand_real1(&gen);
				g.forInEdgesOf(v, [&](node, node src, edgeweight weight)
							   {
		    //if random < 0 v has already been activated by one of
		    //its in-neighbor in a previous iteration 
		    if (random<0)
			return;
		    random -= weight;
		    //src is the one to activate v
		    if(random<0 && !visited.contains(src)){
			f(src, v, weight);
			s.push(src);
			visited.emplace(src);
		    } });

			} while (!s.empty());
		}
	};
}

#endif
