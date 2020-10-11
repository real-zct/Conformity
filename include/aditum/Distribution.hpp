#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <vector>
#include <SFMT.h>

namespace Aditum
{

/**
   Class responsible for sampling 
   nodes from the distribution 
   defined by the nodes capital scores
*/
class Distribution
{
private:

    /*!< vector containing the pointwise probabilities */
    std::vector<double> discreteProbs;

    /*!< vector containing the cumulative distribution */
    std::vector<double> cumulativeProbs;

    /*!<  max value of the cumulative distribution*/
    double maxValue;

    /*!< random number generator */
    sfmt_t gen;

public:

    /** 
     *  @brief Constructor
     *
     *  It takes the pointwise discrete probability
     *  ditribution
     *
     *  @param discreteProb 
     */
    Distribution(std::vector<double> discreteProbs);


    /**
     *  @brief Constructor
     *
     *  It takes the pointwise discrete probability
     *  distribution plus the seed for the random number generator
     *
     *  @param discreteProb 
     *  @param seed
     */
    Distribution(std::vector<double> discreteProbs, unsigned int seed);


    /**
     *  @brief Sample from the distribution
     *
     *  It draws samples from the cumulative distribution.
     *  
     *
     *  @param size, number of sampels to be drawn
     *  @return points samples from the distribution
     */
    std::vector<int> sample(unsigned int size);


    /**
     *  @brief Sample from the distribution
     *
     *  It draws a sampl from the cumulative distribution.
     *
     *  @return a point sample from the distribution
     */
    int sample();


    /**
     * @brief      Return the cumulative sum of the nodes' scores
     *
     * @details    Return the cumulative sum of the nodes' scores
     *
     * @return     nodes cumulative sum
     */
    double getMaxValue() const;

private:

    /**
     *  @brief return the index associated with the given sample
     *
     *  Given a number this method returns the first index in the cumulativeprobs
     * array that it less or equal to the number provided as input
     *
     *  @param sample is a random number in the range [0, maxValue]
     *  @return return the index of the sampled element
     */
    int find(double sample) const;
};

}    
#endif

