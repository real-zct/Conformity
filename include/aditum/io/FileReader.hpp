#ifndef FILEREADER_H
#define FILEREADER_H

#include <aditum/FunctionTraits.hpp>
#include <string>
#include <stdlib.h>
#include <stdexcept>

namespace Aditum
{

	class FileReader
	{
	public:
		template <typename T,
				  typename F = Utility::function_traits<T>,
				  typename A = typename F::template argument<0>::type>
		typename std::enable_if_t<
			std::is_same<A, std::string>::value &&
			F::arity == 1>
		read(std::string path, T f)
		{
			// open the file
			FILE *fp;
			fp = fopen(path.c_str(), "r");
			if (fp == NULL)
				throw std::runtime_error("unable to open the file");

			// read the file line by line. The callback is applied upon every line
			char *line = NULL;
			size_t len = 0;
			while ((getline(&line, &len, fp)) != -1)
				f(std::string{line});

			// close the file
			fclose(fp);
			if (line)
				free(line);
		}
	};
}

#endif
