/*
 http.cpp
 */

#include "http.hpp"
#include <iostream>
using namespace std;

// Quote a help string appropriately for DOS/bash
std::string Q(const std::string& str)
{
	std::string result = str; 
#ifdef _MSC_VER
	std::string from("\\");
	std::string to("^");
#else
	std::string from = "\\\"";
	std::string to   = "\'"  ;
#endif
	size_t index = 0;
	while (index != string::npos) {
        index = result.find(from, index);
	    if (index != string::npos) {
		   result.replace(index,from.length(), to);
		   index += to.length();
	    }
	}
	return result ;
}

int main(int argc,const char** argv)
{
    if ( argc < 3 ) {
        cout << "usage:   " << argv[0] << " [key value]+" << endl;
        cout << "example: " << argv[0] << Q(" -server clanmills.com -page LargsPanorama.jpg -header \\\"Range: bytes=0-200\\\"") << endl;
        cout << "useful  keys: -verb {GET|HEAD|PUT}  -page str -server str -port number -version [-header something]+ " << endl;
        cout << "default keys: -server clanmills.com -page robin.shtml -port 80 -version 1.0 -header ''" << endl;
        return 0;
    }

    dict_t      response;
    dict_t      request;
    std::string errors;

    request["page"  ]   = "robin.shtml";
    request["server"]   = "clanmills.com";

    // convert the command-line arguments into the request dictionary
    for ( int i = 1 ; i < argc-1 ; i +=2 ) {
        const char* arg = argv[i];
        // skip past the -'s on the key
        while ( arg[0] == '-' ) arg++;

        if ( string(arg) == "header" ) {
            string header = argv[i+1];
            if ( ! strchr(argv[i+1],'\n') ) {
                header += "\r\n";
            }
            request[arg] += header;
        } else {
            request[arg]=argv[i+1];
        }
    }

    int result = http(request,response,errors);
    std::cout << "result = " << result << std::endl;
    std::cout << "errors = " << errors << std::endl;
    std::cout << endl;

    for ( dict_i it = response.begin() ; it != response.end() ; it++ ) {
        std::cout << it->first << " -> ";
        if ( it->first ==  "body") cout << "# " << it->second.length();
        else                       cout << it->second;
        cout << endl;
    }

    return 0;
}
