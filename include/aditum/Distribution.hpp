#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <vector>
#include <SFMT.h>

namespace Aditum
{

    /**
       Class responsible for sampling
       nodes from the distribution
       defined by the nodes capital scores.
       An object of type distribtion stores only those elements
       in the sample space -- usually nodes in the graph -- that have non-zero
       probability
    */
    class Distribution
    {
    private:
        /**
         * It represents a point the sample space.
         * Each point has  an id -- it may be the id of a node  --
         * and the porbability associated with it
         */
        struct SampleObject
        {
            uint32_t id = 0;
            double probs = 0;
            SampleObject() = default;
            SampleObject(uint32_t i, double p);
            SampleObject(SampleObject &&) noexcept = default;
            SampleObject(const SampleObject &) = default;
            SampleObject &operator=(SampleObject &&) = default;
            SampleObject &operator=(SampleObject &) = default;
        };

        /*!< vector containing the pointwise（单个节点的）资本分数的值  */
        std::vector<SampleObject> discreteProbs;

        /*!< vector containing the cumulative distribution，这里的；累积概率主要是资本分数累积概率 */
        std::vector<double> cumulativeProbs;

        /*!<  max value of the cumulative distribution*/
        double maxValue;

        /*!< random number generator */
        sfmt_t gen;
        
        // 记录每个点出边个数和资本分数的乘积
        std::vector<SampleObject> cAndODiscreteProbs;

        // 记录每个点出边个数和资本分数的乘积的累加值
        std::vector<double> cAndOCumulativeProbs;

        /*出边个数和资本分数的乘积的累加最大值*/
        double cAndOMaxValue;

    public:
        /**
         *  @brief Constructor
         *
         *  It takes the pointwise discrete probability
         *  ditribution.
         *
         *  The i-th object of the array will be mapped to a
         *  score object with id i and prob discreteProbs[i] if
         *  the probability is greater than 0, otherwise it will be discarded
         *  @param discreteProb
         *  
         */
        Distribution(int isBaseAlgo,std::vector<double> discreteProbs);
        /**
         *  @brief Constructor
         *
         *  It takes the pointwise discrete probability
         *  ditribution.
         *
         *  The i-th object of the array will be mapped to a
         *  score object with id i and prob discreteProbs[i] if
         *  the probability is greater than 0, otherwise it will be discarded
         *
         *  @param discreteProb
         *  
         */
        Distribution(std::vector<double> discreteProbs);
        /**
         *  @brief Constructor
         *
         *  It takes the pointwise discrete probability
         *  distribution plus the seed for the random number generator.
         *
         *  @param discreteProb
         *  @param seed
         */
        Distribution(std::vector<double> discreteProbs,unsigned int seed);
        /**
         *  @brief Constructor
         *
         *  It takes the pointwise discrete probability
         *  ditribution.
         *
         *  The i-th object of the array will be mapped to a
         *  score object with id i and prob discreteProbs[i] if
         *  the probability is greater than 0, otherwise it will be discarded
         *
         *  @param discreteProb
         *  @param cAndDScore
         */
        Distribution(std::vector<double> discreteProbs,std::vector<double> cAndDScore);

        /**
         *  @brief Constructor
         *
         *  It takes the pointwise discrete probability
         *  distribution plus the seed for the random number generator.
         *
         *  @param discreteProb
         *  @param seed
         */
        Distribution(std::vector<double> discreteProbs, std::vector<double> cAndDScore,unsigned int seed);
        // Distribution(std::vector<double> cAndDScore,int flag);
        // /**
        //  *  @brief Constructor
        //  *
        //  *  该构造器用来计算结点的资本分数与节点出度的乘积，用来后续选结点选择概率中。
        //  *
        //  *  @param cAndDScore
        //  *  @param flag 表示当前是结点cAndDScore的计算
        //  * 
        //  */
        // Distribution(std::vector<double> cAndDScore, unsigned int seed,int flag);

        // /**
        //  *  @brief 该构造器用来计算结点的资本分数与节点出度的乘积，用来后续选结点选择概率中。
        //  *  @param cAndDScore
        //  *  @param flag 表示当前是结点cAndDScore的计算
        //  *
        //  *  @return 得到结点的资本分数与节点出度的乘积的数组
        //  */
        /**
         *  @brief Sample from the distribution
         *
         *  It draws samples from the cumulative distribution.
         *
         *
         *  @param size, number of sampels to be drawn
         *  @return points samples from the distribution
         */
        
        std::vector<uint32_t> sample(unsigned int size);

        /**
         *  @brief Sample from the distribution
         *
         *  It draws a sampl from the cumulative distribution.
         *
         *  @return a point sample from the distribution
         */
        std::vector<uint32_t> sample(unsigned int size,int flag);

        /**
         *  @brief 根据结点资本分数与出度乘积来进行源节点采样
         */
        uint32_t sample();

        /**
         * @brief      Return the cumulative sum of the nodes' scores
         *
         * @details    Return the cumulative sum of the nodes' scores
         *
         * @return     nodes cumulative sum
         */
        double getMaxValue() const;

        /**
         * @brief      Return the number of points in the sample space
         *
         * @details    Return the number of points in the sample space
         *
         * @return     number of points
         */
        int getSize() const;

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
        uint32_t find(double sample) const;
    };

}
#endif
