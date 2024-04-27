#ifndef USERATTRIBUTESFILEREADER_H
#define USERATTRIBUTESFILEREADER_H

#include <regex>
#include <vector>
#include <variant>
#include <string>

namespace Aditum
{
    class UserAttributesFileReader
    {
    private:
        std::regex lineRegex;

    public:
        UserAttributesFileReader(std::string stringRegex = "\\s+");
        std::vector<std::vector<std::variant<int, std::string>>> read(std::string path);
    };
}
#endif
