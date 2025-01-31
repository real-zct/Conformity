#ifndef ADITUMBUILDER_H
#define ADITUMBUILDER_H

#include <aditum/AditumAlgo.hpp>
#include <variant>

namespace Aditum
{

    /**
     * This class helps the construction of an aditum algorithm instance
     *
     */
    template <typename ConcreteBuilder>
    class AditumBuilder
    {
    protected:
        /*!< the aditum graph */
        AditumGraph *aGraph = nullptr;

        /*!< the budget */
        int k;

        /*!< weight of the capital contribution */
        double alpha = 1;

        /*!< approximation ratio */
        double epsilon = 1;

        /*!< accuracy */
        double l = 1;

        /*!< target threshold for a node to be regarded as target */
        double targetThreshold;

        /*!< vector containing the users categorical attributes */
        std::vector<std::vector<std::variant<int, std::string>>> userAttributes;

        /*!< Coefficient for the attribute wise diversity algorithm */
        double lambda = 1;

        /*!< Radius for the hamming ball diversity algorithm */
        int radius = 0;

        /*!< Rewards fot the user selection */
        std::vector<double> userRewards;

    public:
        /**
         * Set the graph for the algorithm
         */
        auto &setGraph(AditumGraph &graph)
        {
            aGraph = &graph;
            return *this;
        }

        /**
         * Set the budget for the algorithm
         */
        auto &setK(int k)
        {
            this->k = k;
            return *this;
        }

        /**
         * Set the weight of the capital constribution
         */
        auto &setAlpha(double alpha)
        {
            this->alpha = alpha;
            return *this;
        }

        /**
         * Set the approximation ratio
         */
        auto &setEpsilon(double epsilon)
        {
            this->epsilon = epsilon;
            return *this;
        }

        /**
         * Set the accuracy of the algorithm
         */
        auto &setL(double l)
        {
            this->l = l;
            return *this;
        }

        /**
         * Set the target threshold
         */
        auto &setTargetThreshold(double threshold)
        {
            this->targetThreshold = threshold;
            return *this;
        }

        /**
         * Set the users categorical attributes vector
         */
        auto &setAttributes(std::vector<std::vector<std::variant<int, std::string>>> &attributes)
        {
            userAttributes = attributes;
            return *this;
        }

        auto &setLambda(double lambda)
        {
            lambda = lambda;
            return *this;
        }

        /**
         * Set the radius for the hamming based algorithm
         */
        auto &setRadius(int radius)
        {
            radius = radius;
            return *this;
        }

        /**
         * Rewards for the class based diversity
         */
        auto &setRewards(std::vector<double> &rewards)
        {
            rewards = rewards;
            return *this;
        }

        /**
         * Create the aditum algorithm
         */
        template <typename SetGenerator, typename DiversityAwareAlgo>
        AditumBase<SetGenerator, DiversityAwareAlgo> *build(int flag){
            //这段代码的作用是创建一个具有特定类型参数的 AditumBase 对象，并且返回指向该对象的指针
            return static_cast<ConcreteBuilder *>(this)->template build<SetGenerator, DiversityAwareAlgo>(flag);
        }

    protected:
        Distribution computeDistribution(double alpha)
        {
            
            if(alpha<1){
                // create the distribution
                std::vector<double> scoreVector(aGraph->scores().size());
                std::vector<double> cAndDScoreVector(aGraph->scores().size());

#pragma omp parallel
                // #pragma omp parallel 是一种用于并行编程的指令，用于在 OpenMP（Open Multi-Processing）
                // 环境中创建并行执行的代码段。这个指令告诉编译器，以下代码块应该并行执行。在执行时，该代码块中的工作将被分配给多个线程，并行地执行。
                for (unsigned int i = 0; i < scoreVector.size(); i++){
                    if (double iScore = aGraph->score(i); iScore >= targetThreshold){// 只保留节点资本分数大于targetThreshold的结点
                        scoreVector[i] = iScore;
                        cAndDScoreVector[i] = (aGraph->score(i))*(aGraph->graph().degreeIn(i)+1);
                    }
                    
                }
                Distribution nodeDistribution(scoreVector,cAndDScoreVector);
                return nodeDistribution;
            }else{
                std::vector<double> scoreVector(aGraph->scores().size());
#pragma omp parallel
                // #pragma omp parallel 是一种用于并行编程的指令，用于在 OpenMP（Open Multi-Processing）
                // 环境中创建并行执行的代码段。这个指令告诉编译器，以下代码块应该并行执行。在执行时，该代码块中的工作将被分配给多个线程，并行地执行。
                for (unsigned int i = 0; i < scoreVector.size(); i++){
                    if (double iScore = aGraph->score(i); iScore >= targetThreshold){// 只保留节点资本分数大于targetThreshold的结点
                        scoreVector[i] = iScore;
                    }
                    
                }
                Distribution nodeDistribution(scoreVector);
                return nodeDistribution;
            }
        }
        Distribution computeDistribution()
        {//基线算法
            
            std::vector<double> scoreVector(aGraph->scores().size());
#pragma omp parallel
            // #pragma omp parallel 是一种用于并行编程的指令，用于在 OpenMP（Open Multi-Processing）
            // 环境中创建并行执行的代码段。这个指令告诉编译器，以下代码块应该并行执行。在执行时，该代码块中的工作将被分配给多个线程，并行地执行。
            for (unsigned int i = 0; i < scoreVector.size(); i++){
                if (double iScore = aGraph->score(i); iScore >= targetThreshold){// 只保留节点资本分数大于targetThreshold的结点
                    scoreVector[i] = iScore;
                }
                    
            }
            Distribution nodeDistribution(1,scoreVector);
            return nodeDistribution;    
        }
        
            
    };

};

#endif
