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
        UserAttributesFileReader(std::string stringRegex = "\\s+");//匹配空格开头的字符串。\\s表示为转义之前的\s,匹配任何空白字符，包括空格、制表符、换页符等等。
        std::vector<std::vector<std::variant<int, std::string>>> read(std::string path);
    };
}
#endif
