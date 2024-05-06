#include <aditum/io/CapitalScoreFileReader.hpp>
#include <absl/container/flat_hash_map.h>
#include <aditum/io/FileReader.hpp>
#include <iostream>
namespace Aditum
{

    CapitalScoreFileReader::CapitalScoreFileReader(std::string stringRegex) : lineRegex{stringRegex} {}

    std::vector<double> CapitalScoreFileReader::read(std::string path){
        //read的功能是根据设置的path读取节点资本分数，返回一个vector<double>类型的数组capitalScores，capitalcores[0]表示节点编号0的资本分数。
        absl::flat_hash_map<uint, double> scores;
        uint maxNode = 0;
        auto f = [&](std::string line){ 
            //解析一行文本，并从中提取节点编号和资本得分，然后将它们存储到scores中 
            std::smatch cm;
            std::regex_search(line, cm, lineRegex);//使用正则表达式 lineRegex 在输入的 line 中搜索匹配项，并将匹配结果存储在 cm 中
            auto node = atoi(cm[1].str().c_str());//从匹配结果中提取第一个子匹配项（通常是节点编号），将其转换为整数并存储在 node 中。
            auto capitalScore = atof(cm[2].str().c_str());//atof()转换为浮点型。从匹配结果中提取第二个子匹配项（通常是节点的资本得分），将其转换为浮点数并存储在 capitalScore 中

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
