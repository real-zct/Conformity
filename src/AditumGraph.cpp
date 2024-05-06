#include <aditum/AditumGraph.hpp>

namespace Aditum
{

    std::vector<double> const &AditumGraph::scores() const
    {
        return capitalScores;
    }

    Graph const &AditumGraph::graph() const
    {
        return *influenceGraph;
    }

    double AditumGraph::score(node v)
    {
        return capitalScores[v];
    }

    AditumGraphBuilder::AditumGraphBuilder(std::string graphPath, std::string scoresPath)
        : graphPath{graphPath}, scoresPath{scoresPath} {}

    AditumGraphBuilder &AditumGraphBuilder::setGraphPath(const std::string graphPath)
    {
        this->graphPath = graphPath;
        return *this;
    }

    AditumGraphBuilder &AditumGraphBuilder::setScoresPath(const std::string scoresPath)
    {
        this->scoresPath = scoresPath;
        return *this;
    }

    AditumGraph AditumGraphBuilder::build(NetworKit::GraphReader &&reader, CapitalScoreFileReader &&capReader)
    //  读取文件中的图和节点资本分数
    // NetworKit::GraphReader可以读取不同格式的图形文件，如GraphML、GML、Edge List等，并将它们转换为NetworKit内部表示的图形数据结构，以便进行进一步的分析和处理
    // build用来创建一个AditumGraph以及设置资本分数和影响力图。
    {
        auto g = std::make_unique<Graph>(reader.read(this->graphPath)); // make_unique用于在动态内存中创建对象并返回一个指向该对象的唯一指针
        // read的功能是读取特定格式的图形文件，并将其转换为NetworKit的Graph结构
        auto nodeScores = capReader.read(this->scoresPath);

        AditumGraph aGraph;
        std::swap(aGraph.capitalScores, nodeScores);
        aGraph.influenceGraph = std::move(g); // std::move()的功能是将一个左值强制转换为右值引用

        return aGraph;
    }

}
