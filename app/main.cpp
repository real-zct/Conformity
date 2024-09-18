#include <fstream>
#include <boost/program_options.hpp>
#include <aditum/RandomRRSetGenerator.hpp>
#include <aditum/AditumGraph.hpp>
#include <aditum/AditumAlgo.hpp>
#include <aditum/Distribution.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/io/EdgeListReader.hpp>
#include <aditum/AttributeWise.hpp>
#include <aditum/AditumBuilder.hpp>
#include <aditum/io/UserAttributesFileReader.hpp>
// #include <aditum/ClassBased.hpp>
// #include <aditum/EntropyBased.hpp>
#include <iostream>

namespace po = boost::program_options;

/**
 * Struct for parsing the algorithm command line argument
 */
struct algorithm
{
	std::string name;
	algorithm(const std::string &n) : name{n} {}
	enum code
	{
		wise = 0,
		clas,
		entropy,
		hamming
	};
	code toCode() const
	{
		if (name == "wise")
			return code::wise;
		if (name == "class")
			return code::clas;
		if (name == "entropy")
			return code::entropy;
		if (name == "hamming")
			return code::hamming;
		throw std::runtime_error("Unknown algorithm");
	}
	
};

/**
 * Struct ofor parsing the diffusion model command line argument
 */
struct diffusion
{
	diffusion(const std::string &n) : name{n} {}
	std::string name;
};

/**
 * Validate a generic input value
 */
template <typename T> // template 关键字告诉C++编译器 我要开始泛型了.你不要随便报错
std::string validatedValue(boost::any &v, std::vector<T> availableOptions, const std::vector<T> &values)
{
	// Make sure no previous assignment to 'a' was made.
	po::validators::check_first_occurrence(v);

	// Extract the first string from 'values'. If there is more than
	// one string, it's an error, and exception will be thrown.
	std::string s = po::validators::get_single_string(values);

	// convert to lower case
	std::for_each(s.begin(), s.end(), [](char &c)
				  { c = ::tolower(c); });

	// check if the given algo arg correspond to an available decomposition
	if (std::find(availableOptions.begin(),
				  availableOptions.end(), s) == availableOptions.end())
		throw po::validation_error(po::validation_error::invalid_option_value);
	return s;
}

/**
 * Validate the algorithm provided as input
 */
void validate(boost::any &v, const std::vector<std::string> &values, algorithm *, int)
{
	static std::vector<std::string> availableAlgorithms = {
		"aditum",
		"wise",
		"class",
		"entropy",
		"hamming",
	};

	auto s = validatedValue(v, availableAlgorithms, values);
	v = boost::any(algorithm(s)); //
}

/**
 * Validate the diffusion model provided as input
 */
void validate(boost::any &v, const std::vector<std::string> &values, diffusion *, int)
{
	static std::vector<std::string> availableModels = {
		"lt",
		"ic",
	};

	auto s = validatedValue(v, availableModels, values);
	v = boost::any(diffusion(s));
}

/**
 * Forward declation.
 * This function taks a generic AditumBuilder and set itse
 * member fields.
 * This function is only a placeholder that needs to be
 * specialized for each Aditum Algorithm
 *
 */
template <typename AlgoBuilder,
		  typename... Args>
void setRemaining(AlgoBuilder &instance, Args... args)
{
	throw std::runtime_error("template must be specialized");
}

/**
 * This function allows to run a seed selection process
 * regardles of the particular selected aditum algorithm (via type erasing)
 */
template <template <typename> class Algo,
		  typename SetGenerator,
		  typename AlgoBuilder,
		  typename... Args>
std::tuple<std::set<Aditum::node>, double,double,double, double> run(Aditum::AditumGraph &g,
						   int k,
						   double alpha,
						   double epsilon,
						   double accuracy,
						   double targetThreshold,
						   std::vector<std::vector<std::variant<int, std::string>>> userAttributes,
						   int flag,
						   Args &&...args)
{
	AlgoBuilder algoBuilder;
	algoBuilder.setGraph(g)
		.setK(k)
		.setAlpha(alpha)
		.setEpsilon(epsilon)
		.setL(accuracy)
		.setTargetThreshold(targetThreshold)
		.setAttributes(userAttributes);

	// init the additional -- algorithm specifica arguments
	setRemaining(algoBuilder, std::forward<Args>(args)...);

	// crate the instance of the algorithm
	auto algo = algoBuilder.template build<SetGenerator, Algo<SetGenerator>>();
	// run the seed selection process
	algo->run();
	auto seeds = algo->getSeeds();
	//计算RIS下种子集合的资本分数
    auto capitalRIS=algo->getSeedsCapital_rootCapitalCovProb(targetThreshold);
	auto capitalRIS1=algo->getSeedsCapital_rrsetNumCovProb(targetThreshold);
	auto capitalRIS2=algo->getSeedsCapital_rrsetCovRootCapitalCum();
	//计算MC下种子集合的资本分数
	double capitalMC=0.0;
	if(flag==1){
		capitalMC=algo-> ICMonteCarloEstimationOfCapital(targetThreshold, seeds, 10000);
	}else{
		capitalMC=algo-> LTMonteCarloEstimationOfCapital(targetThreshold, seeds, 10000);
	}
	
	return std::make_tuple(seeds, capitalRIS,capitalRIS1,capitalRIS2, capitalMC);

}

// you must provide a tempalte specialization for every
// diversity based algorithm
template <>
void setRemaining(Aditum::AttributeWiseBuilder &instance, double lambda)
{
	instance.setLambda(lambda);
}

// template <>
// void setRemaining(Aditum::ClassBasedBuilder &instance, std::vector<double> &userRewards)
// {
// 	instance.setRewards(userRewards);
// }

// template <>
// void setRemaining(Aditum::EntropyBasedBuilder &instance)
// {
// }

/**
 * Main Function
 */
int main(int argc, char const *argv[])
{
	try
	{

		po::options_description desc("Allowed Options");
		desc.add_options()("help", "produce help message")("algorithm,a", po::value<algorithm>(),
														   "Aditum algorithm\n"
														   "Options:\n"
														   "    [*] wise - attribute wise\n"
														   "    [*] class - class based\n"
														   "    [*] entropy - entropy based\n"
														   "    [*] hamming - hamming ball based");

		// aditum common parameters
		po::options_description commonArgs("Common Arguments");
		commonArgs.add_options()("graph", po::value<std::string>(), "input graph")("capital", po::value<std::string>(), "capital score")("target-threshold", po::value<double>(), "\u03C4")("output,out", po::value<std::string>(), "where seeds have to be stored")("diffusion-model,dm", po::value<diffusion>(),
																																																																	 "Diffusion model according to which the RR Sets must be generated.\n"
																																																																	 "Options:\n"
																																																																	 "    [*] lt - linear threshold model\n"
																																																																	 "    [*] ic - independent cascade model")("epsilon, eps", po::value<double>()->default_value(1), "\u03B5")("accuracy, l", po::value<double>()->default_value(1), "accuracy - l")("alpha, a", po::value<double>()->default_value(1), "\u03B1")("k", po::value<int>(), "number of seeds to be discovered");

		// attribute driven arguments
		po::options_description attrArgs("Attribute Arguments");
		attrArgs.add_options()("attributes", po::value<std::string>(), "file containing users attributes")("regex", po::value<std::string>()->default_value("\\s+"), "regex to be used when reading the user attributes file");

		// attribute wise diversity arguments
		po::options_description attrWiseArgs("AttributeWise Arguments");
		attrWiseArgs.add_options()("lambda, lam", po::value<double>()->default_value(1.0), "\u03BB");

		// // class based diversity arguments
		// po::options_description classBasedArgs("ClassBased Arguments");
		// classBasedArgs.add_options()("rewardsFile, r",
		// 							 po::value<std::string>()->default_value(""),
		// 							 "File containing the rewards associated to each node in the graph.\n"
		// 							 "Leaving this option empty means that every user has the same reward value");

		// // hamming based diversity arguments
		// po::options_description hammingBased("Hamming Arguments");
		// hammingBased.add_options()("radius, r", po::value<std::string>()->default_value(""), "\u03BE")("reach", po::value<int>()->default_value(0), "Influence Reach Strategy.\n"
		// "Options:\n"
		// "    [0] - RRSet,  based on the computed RRSets\n"
		// "    [1] - Reach, based on the graph topology\n");

		// desc.add(commonArgs).add(attrArgs).add(attrWiseArgs).add(classBasedArgs).add(hammingBased);
		desc.add(commonArgs).add(attrArgs).add(attrWiseArgs);

		// read command line arguments
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(),vm);
		//po::command_line_parser 是一个用于解析命令行参数的类。argc和argv，分别表示命令行参数的数量和参数的实际值（通常在 main 函数中传入）。
		//options 方法将解析器配置为使用 desc 描述的选项。desc 是一个包含所有可用命令行选项描述的对象
		//run 方法执行命令行参数解析，并返回一个 boost::program_options::parsed_options 对象，该对象包含解析结果。
		//po::store 函数将解析结果存储到 vm 中，vm 是一个 boost::program_options::variables_map对象，用于保存命令行参数的值。存储过程会将命令行参数映射到相应的变量中，以便后续代码使用。
		po::notify(vm);
		//notify将解析后的命令行选项值通知给 variables_map 对象中的变量
		if (vm.count("help"))
		{
			std::cout << desc << std::endl;
			return 0;
		}

		// read common arguments
		auto aditumAlgo = vm["algorithm"].as<algorithm>().toCode();
		auto inputGraph = vm["graph"].as<std::string>();
		auto capitalScores = vm["capital"].as<std::string>();
		auto targetThreshold = vm["target-threshold"].as<double>();
		auto outputFile = vm["output"].as<std::string>();
		auto diffusionModel = vm["diffusion-model"].as<diffusion>().name;
		auto epsilon = vm["epsilon"].as<double>();
		auto accuracy = vm["accuracy"].as<double>();
		auto alpha = vm["alpha"].as<double>();
		auto k = vm["k"].as<int>();
		auto attributes = vm["attributes"].as<std::string>();

		// read the graph and the capital score
		Aditum::AditumGraphBuilder builder;
		builder = builder.setGraphPath(inputGraph)
					  .setScoresPath(capitalScores);
		Aditum::AditumGraph g(builder.build());

		// read the user attributes file
		Aditum::UserAttributesFileReader reader(vm["regex"].as<std::string>());
		std::cout << "reading user attriubtes";
		std::vector<std::vector<std::variant<int, std::string>>> userAttributes{reader.read(attributes)};

		std::set<Aditum::node> seeds;
		std::tuple<std::set<Aditum::node>, double, double,double,double> result;
		// this is ugly -- I know!
		switch (aditumAlgo)
		{
		case algorithm::code::wise: // Attribute Wise Algorithm
			std::cout << "running wise" << std::endl;
			if (diffusionModel == "ic")
				result = run<Aditum::AttributeWise, Aditum::ICRandomRRSetGenerator,
							Aditum::AttributeWiseBuilder, double>(g, k, alpha, epsilon, accuracy,
																  targetThreshold,
																  userAttributes,
																  1,
																  double{vm["lambda"].as<double>()});
			else
				result = run<Aditum::AttributeWise, Aditum::LTRandomRRSetGenerator,
							Aditum::AttributeWiseBuilder, double>(g, k, alpha, epsilon, accuracy,
																  targetThreshold,
																  userAttributes,
																  0,
																  double{vm["lambda"].as<double>()});
			break;

			
		}

		// store the seeds file into the file provided as input
		seeds=std::get<0>(result);
		std::ofstream f;
		f.open(outputFile);
		f << "seeds" << "\n";
		for (auto x : seeds)
			f << x << "\n";

		//种子的结点分数
		f << "CapitalRIS_rootCapitalCovProb:";
		f << std::get<1>(result) << "\n";
		f << "CapitalRIS_rrsetNumCovProb:";
		f << std::get<2>(result) << "\n";
		f << "CapitalRIS_rrsetCovRootCapitalCum:";
		f << std::get<3>(result) << "\n";
		f << "CapitalMC:";
    	f << std::get<4>(result);
		f.close();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << "\n";
		return 1;
	}
	return 0;
}
