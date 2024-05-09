#include <aditum/Distribution.hpp>
#include <aditum/Utility.hpp>
#include <memory>
#include <algorithm>

namespace Aditum
{

	Distribution::SampleObject::SampleObject(uint32_t i, double p) : id{i}, probs{p} {}

	Distribution::Distribution(std::vector<double> discreteP, unsigned int seed)
	{
		// init the random number generator
		sfmt_init_gen_rand(&gen, seed);
		double cumProb = 0;

		for (uint32_t i = 0; i < discreteP.size(); ++i)
		{
			if (double prob = discreteP[i];
				prob > 0)
			{
				// create an new entry into the SampleObject vector
				discreteProbs.emplace_back(i, prob);//将节点编号和资本分数的概率放在discreteProbs中
				cumProb += prob;
				cumulativeProbs.emplace_back(cumProb);//将累计概率存储在cumulativeProbs中
			}
		}
		maxValue = cumProb;
	}
	Distribution::Distribution(std::vector<double> cAndDScore, unsigned int seed,int flag)
	{
		// init the random number generator
		sfmt_init_gen_rand(&gen, seed);
		double cumProb = 0;

		for (uint32_t i = 0; i < cAndDScore.size(); ++i)
		{
			if (double prob = cAndDScore[i];
				prob > 0)
			{
				// create an new entry into the SampleObject vector
				cAndODiscreteProbs.emplace_back(i, prob);//将节点编号和资本分数出度乘积的概率放在discreteProbs中
				cumProb += prob;
				cAndOCumulativeProbs.emplace_back(cumProb);//将累计概率存储在cumulativeProbs中
			}
		}
		cAndOMaxValue = cumProb;
	}

	Distribution::Distribution(std::vector<double> discreteProbs) : Distribution(discreteProbs, Utility::now()) {}
	
	Distribution::Distribution(std::vector<double> cAndDScore,int flag) : Distribution(cAndDScore, Utility::now(), flag) {}

	std::vector<uint32_t> Distribution::sample(unsigned int size)
	{//返回随机选中的源节点id
		// the required size must be at least SFMT_32，SFMT_N32是指面向SIMD（单指令多数据）的32位生成器的快速梅森旋转器。它是一种伪随机数生成算法
		auto maxSize = SFMT_N32 > size ? SFMT_N32 : size;

		// create the aligned memory
		int actualSize = 0;
		auto sampleArray = Utility::alignedMemory<uint32_t>(maxSize, &actualSize);//分配maxSize对应的内存数actualSize，并返回指针给sampleArray
		// generate the random numbers
		sfmt_fill_array32(&gen, sampleArray, actualSize);//通过 sfmt_fill_array32() 生成的随机数存储在 sampleArray 中，该数组是一个包含随机整数的内存块。

		// convert each random number in a double value in the range [0-maxValue]
		// put each sampled number in a ... in increasing order
		std::vector<double> randomSamples(size);
		for (int i = 0; i < size; i++)
			randomSamples[i] = sfmt_to_real1(sampleArray[i]) * maxValue;//将这些随机整数转换为双精度浮点数，并缩放到 [0, maxValue] 的范围内

		// sort in ascending order - O(size x log(size))
		std::sort(randomSamples.begin(), randomSamples.end());

		// delete sampleArray
		free(sampleArray);

		// convert a double into the corresponding index
		std::vector<uint32_t> randomPoints(size, 0);

		// find the node corresponding to each sample -- O(size)
		int currentNode = 0;
		for (int i = 0; i < size; i++)
		{
			while (randomSamples[i] > cumulativeProbs[currentNode])
				currentNode++;
			randomPoints[i] = discreteProbs[currentNode].id;//将累积概率大于随机数randomSamples[i]的点作为randomSamples[i]所选择的点。
		}

		return randomPoints;
	}

	std::vector<uint32_t> Distribution::sample(unsigned int size,int flag)
	{//返回随机选中的源节点id
		// the required size must be at least SFMT_32，SFMT_N32是指面向SIMD（单指令多数据）的32位生成器的快速梅森旋转器。它是一种伪随机数生成算法
		auto maxSize = SFMT_N32 > size ? SFMT_N32 : size;

		// create the aligned memory
		int actualSize = 0;
		auto sampleArray = Utility::alignedMemory<uint32_t>(maxSize, &actualSize);//分配maxSize对应的内存数actualSize，并返回指针给sampleArray
		// generate the random numbers
		sfmt_fill_array32(&gen, sampleArray, actualSize);//通过 sfmt_fill_array32() 生成的随机数存储在 sampleArray 中，该数组是一个包含随机整数的内存块。

		// convert each random number in a double value in the range [0-maxValue]
		// put each sampled number in a ... in increasing order
		std::vector<double> randomSamples(size);
		for (int i = 0; i < size; i++)
			randomSamples[i] = sfmt_to_real1(sampleArray[i]) * cAndOMaxValue;//将这些随机整数转换为双精度浮点数，并缩放到 [0, maxValue] 的范围内

		// sort in ascending order - O(size x log(size))
		std::sort(randomSamples.begin(), randomSamples.end());

		// delete sampleArray
		free(sampleArray);

		// convert a double into the corresponding index
		std::vector<uint32_t> randomPoints(size, 0);

		// find the node corresponding to each sample -- O(size)
		int currentNode = 0;
		for (int i = 0; i < size; i++)
		{
			while (randomSamples[i] > cAndOCumulativeProbs[currentNode])
				currentNode++;
			randomPoints[i] = cAndODiscreteProbs[currentNode].id;//将累积概率大于随机数randomSamples[i]的点作为randomSamples[i]所选择的点。
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
		while (sample > cumulativeProbs[id])
			id++;

		return discreteProbs[id].id;
	}

};
