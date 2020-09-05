/**
 *   \file Utility.hpp
 *   \brief File containing several utility functions
 *
 *  File containing a number of utility functions
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
	 *  \param atLeastsize is the (least) number of element in the final array
	 *  \param arraySize is the actual size of the allocated array
	 *  \return return type
	 */
	template<typename T>
	T* alignedMemory(unsigned int atLeastSize, int &&arraySize = 0)
	{
	    //find the closest multiple of 16
	    auto closestMultiple16 = [](int n) -> int {
		if(n%16==0)
		    return n;

		n = n + 16/2;
		n = n - (n%16) + 16;
		return n;
	    };
	    auto typeSize = sizeof(T);
	    //compute the number of bytes required to store atLeastSize instance of T
	    auto alignedSize = closestMultiple16(atLeastSize);
	    auto atLeastSizeBytes = typeSize * alignedSize;
	    arraySize = atLeastSizeBytes/typeSize;
	    T* alignedPointer = (T*) memalign(16, atLeastSizeBytes);
	    return alignedPointer;
	}
    
    }
}
#endif


