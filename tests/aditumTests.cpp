#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <aditum/RandomRRSetGenerator.hpp>
#include <aditum/AditumGraph.hpp>
#include <aditum/AditumAlgo.hpp>
#include <aditum/Distribution.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/io/EdgeListReader.hpp>
#include <aditum/AttributeWise.hpp>
#include <aditum/AditumBuilder.hpp>
#include <aditum/io/UserAttributesFileReader.hpp>
// #include <aditum/ClassBased.hpp>
#include <absl/container/flat_hash_map.h>
// #include <aditum/EntropyBased.hpp>

template <typename... Ts>
struct visitor : Ts...
{
    using Ts::operator()...;
};
template <typename... Ts>
visitor(Ts...) -> visitor<Ts...>;

TEST_CASE("Attr Wise", "[.]")
{   
    //读取Graph以及节点资本分数
    Aditum::AditumGraphBuilder builder;
    builder = builder.setGraphPath("/home/zct/laboratory/project/data/graph.txt")
                  .setScoresPath("/home/zct/laboratory/project/data/capital.txt");
    Aditum::AditumGraph g(builder.build());
    
    //读取用户属性
    Aditum::UserAttributesFileReader a("\\s+");
    std::string path = "/home/zct/laboratory/project/data/attribute01.txt";
    std::vector<std::vector<std::variant<int, std::string>>> data{a.read(path)};
    
    //构造Aditum，设置所需参数以及扩散模型
    Aditum::AditumBuilder<Aditum::AttributeWiseBuilder> algoBuilder;
    algoBuilder.setGraph(g)
        .setAlpha(0.5)
        .setEpsilon(1)
        .setK(2)
        .setTargetThreshold(0)
        .setAttributes(data);//alpha为资本值系数
    auto algo = algoBuilder.build<Aditum::LTRandomRRSetGenerator,
                                  Aditum::AttributeWise<Aditum::LTRandomRRSetGenerator>>(1);//1表示一致性算法
    //选择种子节点并返回结果
    algo->run();
    auto seeds = algo->getSeeds();
    for (auto x : seeds)
        std::cout << x << "\n";
    
    //输出RIS下种子集合的资本分数
    auto capitalRIS=algo->getSeedsCapital_rootCapitalCovProb(0);
    std::cout << capitalRIS << "\n";
    capitalRIS=algo->getSeedsCapital_rrsetNumCovProb(0);
    std::cout << capitalRIS << "\n";
    capitalRIS=algo->getSeedsCapital_rrsetCovRootCapitalCum();
    std::cout << capitalRIS << "\n";

    //测试种子集合的资本分数
    auto capitalMC=algo-> ICMonteCarloEstimationOfCapital(0, seeds, 10000);
    std::cout << capitalMC<< "\n";

    //基线算法
    Aditum::AditumBuilder<Aditum::AttributeWiseBuilder> algoBuilderBase;
    algoBuilderBase.setGraph(g)
        .setAlpha(1)
        .setEpsilon(1)
        .setK(2)
        .setTargetThreshold(0)
        .setAttributes(data);//alpha为资本值系数,基线算法的alpha设置一定是1，采样源节点走无一致性的代码。
    auto algoBase = algoBuilderBase.build<Aditum::LTRandomRRSetGenerator,
                                  Aditum::AttributeWise<Aditum::LTRandomRRSetGenerator>>(0);//0表示基线算法
    //选择种子节点并返回结果
    algoBase->run();
    auto baseSeeds = algoBase->getSeeds();
    for (auto y : baseSeeds)
        std::cout << y << "\n";

    //测试种子集合的资本分数
    auto baseCapitalMC=algoBase-> ICMonteCarloEstimationOfCapital(0, baseSeeds, 10000);
    std::cout << baseCapitalMC<< "\n";

}
