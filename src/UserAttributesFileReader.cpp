#include <aditum/io/UserAttributesFileReader.hpp>
#include <aditum/io/FileReader.hpp>
#include <absl/container/flat_hash_map.h>
#include <variant>

namespace Aditum {

UserAttributesFileReader::UserAttributesFileReader(std::string stringRegex):
    lineRegex{stringRegex}{}
    
std::vector<std::vector<std::variant<int, std::string>>>
UserAttributesFileReader::read(std::string path)
{
    

    absl::flat_hash_map<uint, std::vector<std::variant<int, std::string>>> attributes;
    uint maxNode = 0;
    FileReader reader;
    auto f = [&](std::string line)
    {
	std::vector<std::variant<int, std::string>> lineData;
	std::vector<std::string> result { 
	    std::sregex_token_iterator(line.begin(), line.end(), lineRegex, -1), {}
	};
	//the first token must be the id of the node
	auto it = result.begin();
	uint node = std::stoi(*it);
	for(++it; it!=result.end(); ++it)
	{
	    std::string &x = *it;
	    try {
		//check if the categorical value can be converted as an integer
		int xint = std::stoi(x);
		lineData.emplace_back(xint);
	    }catch(std::invalid_argument &e) {
		//insert the attribute as a regular string
		lineData.emplace_back(x);
	    }
	}
	//copy the line vector inside the map
	attributes[node] = lineData;

	if(node>maxNode)
	    maxNode = node;
    };

    FileReader r;
    r.read<decltype(f)>(path, f);
    //convert the map into a vector
    //get the maximum value for hte keys
    std::vector<std::vector<std::variant<int, std::string>>> userAttributes(maxNode+1);

    for(auto &entry : attributes)
	userAttributes[entry.first] = entry.second;
    //get 
    return userAttributes;
}
}
