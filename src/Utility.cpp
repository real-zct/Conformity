#include <aditum/Utility.hpp>
#include <chrono>
#include <iostream>
namespace  Aditum
{

    namespace Utility
    {

	unsigned int now()
	{
	    namespace sc = std::chrono;
	    auto time = sc::system_clock::now(); // get the current time

	    auto since_epoch = time.time_since_epoch(); // get the duration since epoch

	    // I don't know what system_clock returns
	    // I think it's uint64_t nanoseconds since epoch
	    // Either way this duration_cast will do the right thing
	    auto millis = sc::duration_cast<sc::milliseconds>(since_epoch);
	    return millis.count();
	}


	// T* alignedMemory(unsigned int atLeastSize, int *arraySize)
	// {
	//     std::cout << "=======aligned=====" << "\n";
	//     //find the closest multiple of 16
	//     std::cout << atLeastSize << "\n";

	//     auto closestMultiple16 = [](int n) -> int {
	// 	int mod = n%16;
	// 	if(mod)
	// 	    return n;
	// 	return n + 16-mod;
	// 	// n = n + 16/2;
	// 	// n = n - (n%16) + 16;
	// 	// return n;
	//     };
	    
	//     auto typeSize = sizeof(T);
	//     //compute the number of bytes required to store atLeastSize instance of T
	//     auto alignedSize = closestMultiple16(atLeastSize);
	//     std::cout << alignedSize << "\n";

	//     auto atLeastSizeBytes = typeSize * alignedSize;
	//     *arraySize = atLeastSizeBytes/typeSize;
	//     T* alignedPointer = (T*) memalign(16, atLeastSizeBytes);
	//     return alignedPointer;
	// }

    }



}
