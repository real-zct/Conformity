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

    AditumGraph AditumGraphBuilder::build(NetworKit::GraphReader &&reader,
                                          CapitalScoreFileReader &&capReader)
    {
        auto g = std::make_unique<Graph>(reader.read(this->graphPath));
        auto nodeScores = capReader.read(this->scoresPath);

        AditumGraph aGraph;
        std::swap(aGraph.capitalScores, nodeScores);
        aGraph.influenceGraph = std::move(g);

        return aGraph;
    }

}
