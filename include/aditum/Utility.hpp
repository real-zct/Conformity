/**
 *   \file Utility.hpp
 *   \brief File containing several utility functions
 *
 *  File containing a number of utility functions and objects
 *
 */
#ifndef UTILITY_H
#define UTILITY_H

#include <malloc.h>

namespace Aditum
{
    
    namespace Utility
    {

	
	/**
	 *  \brief Return the current timestamp
	 *
	 *  Compute the current timestamp
	 *  \return current timestamp
	 */    
	unsigned int now();


	/**
	 *  \brief Aligned memory allocation
	 *
	 *  This method allocate a pointer whose size
	 *  is the closest multiple of 16 greater or equal than
	 *  the size provided as input
	 *
	 *  \param atLeastsize is the (least) number of element in the final arrayg
	 *  \param arraySize is the actual size of the allocated array
	 *  \return return type
	 */
	template<typename T>
	T* alignedMemory(unsigned int atLeastSize, int *alignedSize)
	{
	    //find the closest multiple of 16
	     auto closestMultiple16 = [](unsigned int n) -> int {
		int mod = n%16;
		if(mod == 0)
		    return n;
		return n + 16-mod;
	    };

	    auto typeSize = sizeof(T);
	    //compute the number of bytes required to store atLeastSize instance of T
	    
	    *alignedSize = closestMultiple16(atLeastSize);

	    auto atLeastSizeBytes = typeSize * *alignedSize;
	    T* alignedPointer = (T*) memalign(16, atLeastSizeBytes);
	    return alignedPointer;
	}


	/**
	 * @brief      Class for lazy forward greedy selection
	 *
	 * @details    This class keep the inforamtion required to build the seed set 
	 *
	 */
	struct ScoreObject
	{
	    /*!< Weighting factor between capital and diversity */
	    inline static double alpha = 0;

	    // static void setAlpha(double newAlpha);


	    /*!< node the score refers to */
	    unsigned int node;

	    /*!< the iteration the score refers to */
	    unsigned int iteration;

	    /*!< the capital score of the node */
	    double capitalScore;

	    /*!< the diversity score of the node */
	    double diversityScore;

	    ScoreObject() = default;
	    ScoreObject(ScoreObject&&) noexcept = default;
	    ScoreObject& operator=(ScoreObject&&) = default; // force a move assignment anyway 

    	    bool operator<(const ScoreObject &other) const
	    {
		    return (alpha*capitalScore + (1-alpha)*diversityScore) <
			(alpha*other.capitalScore+ (1-alpha)*other.diversityScore);
	    }
	    
	};


	template<typename ... Ts>
	struct visitor : Ts... {
	    using Ts::operator()...; 
	};
	template<typename ... Ts> visitor(Ts...) -> visitor<Ts...>;


    }
}
#endif


