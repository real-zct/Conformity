#include <aditum/Distribution.hpp>
#include <aditum/Utility.hpp>
#include <memory>
#include <algorithm>
#include <iostream>
namespace Aditum {


    Distribution::Distribution(std::vector<double> discreteP, unsigned int seed):
	discreteProbs{discreteP}
    {
	//init the random number generator
	sfmt_init_gen_rand(&gen, Utility::now());

     	//create the cumulative distribution
	cumulativeProbs = std::vector<double>(discreteProbs.size(), 0);
	double cumProb = 0;	
	for (int i=0 ; i<discreteProbs.size() ; ++i )
	{
	    cumProb += discreteProbs[i];
	    cumulativeProbs[i] = cumProb;
	}
	maxValue = cumProb;
    }
    
    Distribution::Distribution(std::vector<double> discreteProbs):
	Distribution(discreteProbs, Utility::now()){}


    std::vector<int> Distribution::sample(unsigned int size) 
    {
        //the required size must be at least SFMT_32
	auto maxSize = SFMT_N32 > size ? SFMT_N32 : size;
	std::cout << maxSize << "\n";

	//create the aligned memory
	auto sampleArray = Utility::alignedMemory<uint32_t>(maxSize);
	//generate the random numbers
	sfmt_fill_array32(&gen, sampleArray, maxSize);
	

	//convert each random number in a double value in the range [0-maxValue]
	//put each sampled number in a ... in increasing order
	std::vector<double> randomSamples(size);
	for(int i=0 ; i<size ; i++)
	    randomSamples[i] = sfmt_to_real1(sampleArray[i])*maxValue;


	// std::cout << "\n=========================\n";

	//sort in ascending order - O(size x log(size))
	std::sort(randomSamples.begin(),randomSamples.end());

	//delete sampleArray
	free(sampleArray);

	//convert a double into the corresponding index
	std::vector<int> randomPoints(size,0);

	//find the node corresponding to each sample -- O(size)
	int currentNode = 0;
	for(int i=0 ; i<size ; i++)
	{
	    while(randomSamples[i] > cumulativeProbs[currentNode])
		currentNode++;
	    randomPoints[i] = currentNode;
	}

	return randomPoints;
    }

    int Distribution::sample() 
    {
	double random = sfmt_genrand_real1(&gen) * maxValue;
	return find(random);
    }


    int Distribution::find(double sample) const
    {
	int id = 0;
	while(sample>cumulativeProbs[id])
	    id++;
	return id;
    }
    
};
