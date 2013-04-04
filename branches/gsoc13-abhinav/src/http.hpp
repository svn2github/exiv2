#ifndef HTTP_H_
#define HTTP_H_

#include <string>
#include <map>
#include <algorithm>    // find

#include <exiv2/exiv2.hpp>

typedef std::map<std::string,std::string> dict_t;
typedef dict_t::iterator                  dict_i;

EXIV2API int http(dict_t& request,dict_t& response,std::string& errors);


// http://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform
EXIV2API class Uri
{
public:
	std::string QueryString;
	std::string Path;
	std::string Protocol;
	std::string Host;
	std::string Port;

	static Uri Parse(const std::string &uri);
};

#endif


