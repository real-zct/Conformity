#include <aditum/io/CapitalScoreFileReader.hpp>
#include <absl/container/flat_hash_map.h>
#include <aditum/io/FileReader.hpp>
#include <iostream>
namespace Aditum
{

    CapitalScoreFileReader::CapitalScoreFileReader(std::string stringRegex) : lineRegex{stringRegex} {}

    std::vector<double> CapitalScoreFileReader::read(std::string path)
    {
        absl::flat_hash_map<uint, double> scores;
        uint maxNode = 0;
        auto f = [&](std::string line)
        {
            std::smatch cm;
            std::regex_search(line, cm, lineRegex);
            auto node = atoi(cm[1].str().c_str());
            auto capitalScore = atof(cm[2].str().c_str());

            scores.emplace(node, capitalScore);

            // find the maximum node so that  we
            // can create a vector of the right size
            if (maxNode < node)
                maxNode = node;
        };

        FileReader r;
        r.read<decltype(f)>(path, f);

        // create the score vector
        std::vector<double> capitalScores(maxNode + 1, 0);
        for (auto e : scores)
            capitalScores[e.first] = e.second;

        return capitalScores;
    }
}
