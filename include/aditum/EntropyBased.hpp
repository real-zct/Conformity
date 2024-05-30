// #ifndef ENTROPYBASED_H
// #define ENTROPYBASED_H
// #include <iostream>
// #include <aditum/AditumAttributeDriven.hpp>
// #include <aditum/AditumBuilder.hpp>
// #include <aditum/Utility.hpp>
// #include <utility>
// #include <vector>
// #include <numeric>
// #include <absl/container/flat_hash_map.h>
// #include <string>

// namespace Aditum {

// /**
//  * Class for the entropy based algorithm.
//  */
// template <typename SetGenerator>
// class EntropyBased: public AditumAttributeDriven<SetGenerator, EntropyBased<SetGenerator>>
// {
// public:

//     using Base = AditumAttributeDriven<SetGenerator, EntropyBased<SetGenerator>>;
//     using symbol = typename Base::symbol;


//     /*
//     * Representation of each categorical symbol
//     * after the preprocessing step.
//     * Each symbol will be associated with a numerical code,
//     * and a probability value, which corresponds to the one relative frequency in the dataset
//     */
//     struct SymbolRepresentation
//     {
// 	/*!< Numerical code  */
// 	int code;

// 	/*!< Frequency of the symbol */
// 	double prob;
//     };
    
//     // /*!< map that associates each symbol to its new representation  */
//     std::vector<absl::flat_hash_map<symbol, SymbolRepresentation>> symbolVectorMap;
    
//     /*!< number of different symbols for each attribute  */
//     std::vector<int> numOfSymbols;

//     /*!< Each position of the profile string is associated with a and the its corresponding attribute position */
//     std::vector<std::pair<symbol, int>> symbolPositions; 

    
// public:    

//     EntropyBased(AditumGraph& graph, Distribution dist,
// 		  std::vector<std::vector<symbol>> userAttributes,
// 		  int k,double alpha = 1, double epsilon = 1, double l = 1):
// 	Base(graph, dist, userAttributes, k, alpha, epsilon,l),
// 	symbolVectorMap((userAttributes.begin())->size()),
// 	numOfSymbols((userAttributes.begin())->size())
//     {}


//     void buildSeedSetWithDiversity()
//     {
// 	//encode each symbol
// 	transformSymbols();

// 	//vector containing the string associated with the profile
// 	//of each node currently in the seed set - the orientation of each seeds
// 	//profile is changed.
// 	//Example: if a node is added with the profile "010100" it will be stored
// 	//as [010100]^T. Therefore, in seedsProfiles[i] is tored the matching
// 	//sequence of every node currently in the seed set with respect to the symbol
// 	//corresponding to the i-th position inside the profile string.
// 	// Let's mcall seedsProfiles[i] the matching string associated with the
// 	// symbol corresponging to the i-th position in the profile string
// 	std::vector<std::string> seedsProfiles(symbolPositions.size());

// 	//profile string cache
// 	absl::flat_hash_map<unsigned int, std::string> cache;
// 	auto getString = [&](unsigned int node) -> std::string {
// 	    std::string nodeString;
// 	    try{
// 		nodeString = cache.at(node);
// 	    }catch (std::exception &e){
// 		nodeString = getStringRepresentation(node);
// 		cache[node] = nodeString;
// 	    }
// 	    return nodeString;
// 	};
	
// 	auto updateSeedsRepresentation = [&](unsigned int node)
// 	{
// 	    std::string nodeString = getString(node);
// 	    int pos = 0;
// 	    for(char &c : nodeString) 
// 		seedsProfiles[pos++].push_back(c);
// 	};

// 	auto computeMarginalDiversity = [&](unsigned int node)
// 	{
// 	    std::string nodeString = getString(node);

// 	    //compute joint and conditional  probabilities
// 	    absl::flat_hash_map<char, absl::flat_hash_map<std::string, double>> jointProbs, condProbs;

// 	    //iterate through every character of the curent node profile string
// 	    for(int i=0 ; i<nodeString.length() ; i++)
// 	    {
// 		//get the symbol corresponging to the i-th position
// 		symbol currentSymbol = symbolPositions[i].first;

// 		//get the attribute corresponding to the current symbol
// 		int currentAttribute = symbolPositions[i].second;

// 		//get symbol probability
// 		double currentSymbolProb = symbolVectorMap[currentAttribute][currentSymbol].prob;

// 		//update the joint probability associated to the entry:
// 		// nodeString[i] -> it can be either 0 or 1 (1 if the current node has the current symbol in its profile)
// 		// seedsProfiles[i] -> it represents the string of matching in the current selection of seeds.
// 		// If three seeds have been already selected, than seedsProfiles[i] is a string of three characters
// 		// to be interpreted as follows:
// 		// e.g., seedsProfiles[i] = "010" -> the first and the third selected sedds do not have
// 		// not have the symbol corresponging to the i-th position of the profile string, while the second
// 		// selected seed has it.
// 		jointProbs[nodeString[i]][seedsProfiles[i]] += currentSymbolProb;
// 	    }

// 	    //compute the marginal probabilities.
// 	    // xProbs is the marginal probability associated with the
// 	    // current node profile, while xProbs is the marginal probability associated with
// 	    // the current seed nodes profiles
// 	    // xProbs has the following structure: {<a matching string -> its probability>...}
// 	    // yProbs has the following strucutre: {<0 -> prob. of a mismatch, 1 -> prob. of a match>}
// 	    absl::flat_hash_map<std::string, double> xProbs, yProbs;
// 	    for(auto &charEntry : jointProbs)
// 	    {
// 		for(auto &stringEntry : charEntry.second)
// 		{
// 		    xProbs[stringEntry.first] += stringEntry.second;
// 		    yProbs[std::string(1,charEntry.first)] += stringEntry.second;
// 		}
// 	    }
	    

// 	    //from the marginal probabilities computed earlier we derive
// 	    //the conditional probabilities. The conditional probability is given by: jointProbs/xProbs
// 	    for(auto &jointEntry : jointProbs) {
// 		for(auto &joinMap : jointEntry.second) {
// 		    condProbs[jointEntry.first][joinMap.first] = joinMap.second/xProbs[joinMap.first];
// 		}
// 	    }

// 	    //finally we can compute the conditional entropy.
// 	    //it represents the marginal gain of the current node
// 	    double h = 0;
// 	    for(auto &margEntry : xProbs)
// 		h += margEntry.second * H({condProbs['0'][margEntry.first], condProbs['1'][margEntry.first]});

// 	    return h;
// 	};


// 	/*!< User attriubtes represented as a one hot encoding vector */
// 	double maxCapital = this->normalizeCapital();
// 	double maxDiversity = 1;
			
// 	auto q = this->getInitialScoreVector(maxCapital, maxDiversity);
	
// 	this->selectionLoop(q, updateSeedsRepresentation, computeMarginalDiversity, maxCapital, maxDiversity);
//     }


//     /**
//      * @brief      It computes the entropy associated with the probability distribution provided as input
//      *
//      * @details    It computes the entropy associated with the probability distribution provided as input
//      *
//      * @param      probability distribution
//      *
//      * @return     the entropy of the probability vectors provided as input
//      */
//     inline double H(std::vector<double>  probs)
//     {
// 	double h = 0;
// 	#pragma omp parallel
// 	for(auto p : probs)
// 	    if(p>0)
// 		h += p*log2(p);
// 	return -h;
//     }


//     /**
//      * @brief      It computes the string representation of user profile
//      *
//      * @details    Each vector of symbols is translated  into a string.
//      *             The size of the string matches the total number of different
//      *             symbols encountered within the dataset provided as input
//      *
//      *             Each position of the string corresponds to a particular symbol 
//      *             regardless of the attribute it belongs to. Each element of the string either be 1 or 0.
//      *
//      *             Example: the following string "1001001" means that the user profile contains:
//      *             the first, third and the seventh symbol. Clearly each original symbol is associated with a position
//      *             --  see the variable symbolPositions 
//      * 
//      * @param      the node we are interested in
//      *
//      * @return     a string represeting the node's profile
//      */
//     inline std::string getStringRepresentation(unsigned int node)
//     {
// 	//get the size of the entire porfile string
// 	int profileLength = symbolPositions.size();
// 	//initialize a string with only 0
// 	std::string profileString(profileLength, '0');
	
// 	int attributePosition = 0, offset = 0;
//         for(auto it = this->userAttributes[node].begin() ;
// 	    it!=this->userAttributes[node].end();
// 	    it++, attributePosition++)
// 	{
// 	    int symbolPosition = offset +  symbolVectorMap[attributePosition][*it].code;
// 	    profileString[symbolPosition] = '1';
// 	    offset += numOfSymbols[attributePosition];
// 	}

// 	return profileString;

//     }

//     /**
//      * @brief      Initialize the attribute representation
//      *
//      * @details    This method assign a numerical code to every symbol.
//      *             More specifically, anyu attribute at position i will have be
//      *             encoded with a numberical value ranging in [0, N] where N is
//      *             the number of different codes for the i-th attribute.
//      */
//     void transformSymbols()
//     {

// 	// //it keeps track of the last code assigned for each
// 	// //attribute position
// 	int attributeSize = this->userAttributes.begin()->size();
// 	double size = this->aGraph.graph().numberOfNodes() * attributeSize;

// 	//the next code to assign for each attribute
// 	std::vector<int> nextSymbolCode(attributeSize, 0);
	
// 	//iterate over all the vectors of user attributes and
// 	//assign a code to each symbol
// 	for(std::vector<symbol> &attributes : this->userAttributes)
// 	{
// 	    auto it = attributes.begin();
// 	    for(int i=0; it!=attributes.end(); ++it, ++i)
// 	    {
// 		try
// 	 	{
// 		    // check if the current symbol has already been encountered
// 	 	    SymbolRepresentation &sRepr = symbolVectorMap[i].at(*it);
// 		    //update the probability
// 		    sRepr.prob += 1/size;
// 		}
// 		catch (std::exception &e)
// 		{
// 		    //miss in the table. It means a new symbol has been encountered
// 		    //create a new SymbolRepresentation object and store it in the map
// 		    SymbolRepresentation sRepr{nextSymbolCode[i]++, 1/size};
// 		    symbolVectorMap[i][*it] = sRepr;

// 		    //increse the length of this attribute
// 		    numOfSymbols[i]++;
		    
// 		}
// 	    }
// 	}

// 	//We need to assign a position in the  profile string representation
// 	//to each symbol -- preserving the order defined by the vector of different attributes.
// 	// This means that a symbol belonging to the i-th attributes cannot appear before
// 	// a symbol belonging to the j-th attribute if i<j.
// 	//the position of a symbol s, belonging to the domain of the i-th attribute, is given by:
// 	// offset [i-th] + its numerical code
// 	std::vector<int> offset(numOfSymbols.size());
// 	std::partial_sum(numOfSymbols.begin(), numOfSymbols.end()-1, offset.begin()+1);
	
// 	//initialize the symbolPosition vector based on the content of the offset vectot
// 	symbolPositions.resize(*(offset.rbegin())+
// 			       *(numOfSymbols.rbegin()));
// 	int attributeIndex = 0;
// 	for(auto &symbolsMap: symbolVectorMap)
// 	{
// 	    for(auto &symbolEntry : symbolsMap)
// 		symbolPositions[offset[attributeIndex]+symbolEntry.second.code] = {symbolEntry.first, attributeIndex};
//        	    attributeIndex++;
// 	}
	    
//     }
    
// };


// /**
//  * @brief      Builder for the attribute wise algorithm
//  *
//  * @details    Builder for the attribute wise algorithm
//  */    
// class EntropyBasedBuilder: public AditumBuilder<EntropyBasedBuilder>
// {
// public:    
//     template<typename SetGenerator, typename DiversityAwareAlgo>
//     AditumBase<SetGenerator, DiversityAwareAlgo>* build()
//    {
//        Distribution nodeDistribution = computeDistribution();

//        	return new EntropyBased<SetGenerator>(*aGraph,
// 					    nodeDistribution,
// 					    userAttributes,
// 					    k,
// 					    alpha,
// 					    epsilon,
// 					    l);

//     }
// };    

// }

// #endif

