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
        .setAlpha(1)
        .setEpsilon(1)
        .setK(3)
        .setTargetThreshold(0.1)
        .setAttributes(data);//alpha为资本值系数
    auto algo = algoBuilder.build<Aditum::LTRandomRRSetGenerator,
                                  Aditum::AttributeWise<Aditum::LTRandomRRSetGenerator>>();
    //选择种子节点并返回结果
    algo->run();
    auto seeds = algo->getSeeds();
    for (auto x : seeds)
        std::cout << x << "\n";
    
    //输出RIS下种子集合的资本分数
    auto capitalRIS=algo->getSeedsCapital(0.1);
    std::cout << capitalRIS << "\n";
    //测试种子集合的资本分数
    auto capitalMC=algo-> LTMonteCarloEstimationOfCapital(0.1, seeds, 10000);
    std::cout << capitalMC;

}
