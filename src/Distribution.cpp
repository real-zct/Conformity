#include <aditum/Distribution.hpp>
#include <aditum/Utility.hpp>
#include <memory>
#include <algorithm>

namespace Aditum
{

    Distribution::SampleObject::SampleObject(uint32_t i, double p): id{i}, probs{p}{}

    Distribution::Distribution(std::vector<double> discreteP, unsigned int seed)
    {
	//init the random number generator
	sfmt_init_gen_rand(&gen, seed);
	double cumProb = 0;

	for (uint32_t i=0 ; i<discreteP.size() ; ++i )
	{
	    if(double prob = discreteP[i];
	       prob>0)
	    {
		//create an new entry into the SampleObject vector
		discreteProbs.emplace_back(i, prob);
		cumProb += prob;
		cumulativeProbs.emplace_back(cumProb);
	    }
	}
	maxValue = cumProb;
    }
    
    Distribution::Distribution(std::vector<double> discreteProbs):
	Distribution(discreteProbs, Utility::now()){}

    std::vector<uint32_t> Distribution::sample(unsigned int size) 
    {
        //the required size must be at least SFMT_32
	auto maxSize = SFMT_N32 > size ? SFMT_N32 : size;

	//create the aligned memory
	int actualSize = 0;
	auto sampleArray = Utility::alignedMemory<uint32_t>(maxSize,&actualSize);
	//generate the random numbers
	sfmt_fill_array32(&gen, sampleArray, actualSize);
	
	//convert each random number in a double value in the range [0-maxValue]
	//put each sampled number in a ... in increasing order
	std::vector<double> randomSamples(size);
	for(int i=0 ; i<size ; i++)
	    randomSamples[i] = sfmt_to_real1(sampleArray[i])*maxValue;

	//sort in ascending order - O(size x log(size))
	std::sort(randomSamples.begin(),randomSamples.end());

	//delete sampleArray
	free(sampleArray);

	//convert a double into the corresponding index
	std::vector<uint32_t> randomPoints(size,0);

	//find the node corresponding to each sample -- O(size)
	int currentNode = 0;
	for(int i=0 ; i<size ; i++)
	{
	    while(randomSamples[i] > cumulativeProbs[currentNode])
		currentNode++;
	    randomPoints[i] = discreteProbs[currentNode].id;
	}

	return randomPoints;
    }

    uint32_t Distribution::sample() 
    {
	double random = sfmt_genrand_real1(&gen) * maxValue;
	return find(random);
    }

    double Distribution::getMaxValue() const
    {
	return maxValue;
    }

    int Distribution::getSize() const
    {
	return discreteProbs.size();
    }
    
    uint32_t Distribution::find(double sample) const
    {
	int id = 0;
	while(sample>cumulativeProbs[id])
	    id++;
	
	return discreteProbs[id].id;
    }
    
};
