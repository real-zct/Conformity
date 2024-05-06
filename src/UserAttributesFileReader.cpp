#include <aditum/io/UserAttributesFileReader.hpp>
#include <aditum/io/FileReader.hpp>
#include <absl/container/flat_hash_map.h>
#include <variant>

namespace Aditum
{

	UserAttributesFileReader::UserAttributesFileReader(std::string stringRegex) : lineRegex{stringRegex} {}

	std::vector<std::vector<std::variant<int, std::string>>> UserAttributesFileReader::read(std::string path){
		/**
		 * UserAttributesFileReader类的read方法用于从文件中读取用户属性，并将用户属性转换为一个二维向量
		 * 文件中一行代表一个用户的属性，第一个值为用户编号，第二个以及后面的值是属性值，用空格分开
		 * read返回的结果为二维数组，userAttributes[a]为一个vector数组表示用户编号为a的属性数组，其类型为std::variant<int, std::string>，
		 * 即数组元素可能是int也可能是String。
		 */
		absl::flat_hash_map<uint, std::vector<std::variant<int, std::string>>> attributes;//absl::flat_hash_map类似无序map，uint为不带符号的int
		uint maxNode = 0;
		FileReader reader;
		auto f = [&](std::string line){//lambda函数，功能：接受一个std::string类型的参数line，并执行一系列操作
			std::vector<std::variant<int, std::string>> lineData;
			std::vector<std::string> result{std::sregex_token_iterator(line.begin(), line.end(), lineRegex, -1), {}};
			//使用正则表达式lineRegex来将字符串line分割成多个子字符串，并将这些子字符串存储在名为result的std::vector<std::string>中
			// the first token must be the id of the node
			auto it = result.begin();
			uint node = std::stoi(*it);//stoi作用是将String转化为int，line的第一个数为节点编号，后面的字符串是属性值
			for (++it; it != result.end(); ++it)
			{
				std::string &x = *it;
				try
				{
					// check if the categorical value can be converted as an integer
					int xint = std::stoi(x);//stoi作用是将String转化为int
					lineData.emplace_back(xint);//转换成功就将int数插入lineData
				}
				catch (std::invalid_argument &e)
				{
					// insert the attribute as a regular string
					lineData.emplace_back(x);//转换失败就将String插入lineData
				}
			}
			// copy the line vector inside the map
			attributes[node] = lineData;

			if (node > maxNode)
				maxNode = node;
		};

		FileReader r;
		r.read<decltype(f)>(path, f);//从文件中读取数据并将其传递给lambda函数f进行处理。
		// convert the map into a vector
		// get the maximum value for hte keys
		std::vector<std::vector<std::variant<int, std::string>>> userAttributes(maxNode + 1);

		for (auto &entry : attributes)
			userAttributes[entry.first] = entry.second;//将attributes转到userAttributes中，将用户属性存放到二维数组中。
		// get
		return userAttributes;
	}
}
