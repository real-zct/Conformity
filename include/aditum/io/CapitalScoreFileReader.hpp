#ifndef CAPITALSCOREFILEREADER_H
#define CAPITALSCOREFILEREADER_H

#include <regex>
#include <vector>

namespace Aditum
{
class CapitalScoreFileReader
{
private:
    std::regex lineRegex;

public:

    CapitalScoreFileReader(std::string stringRegex =
			   "([[:digit:]]+):([+-]?([0-9]*[.])?[0-9]+)");
    std::vector<double> read(std::string path);    
    
};
}

#endif
