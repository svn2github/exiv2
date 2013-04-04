/*
 http.cpp
 */

#include "http.hpp"
#include <iostream>
using namespace std;

int main(int argc,const char** argv)
{
    if ( argc < 3 ) {
        cout << "usage:   " << argv[0] << " [key value]+" << endl;
        cout << "example: " << argv[0] << " -server clanmills.com -page /LargsPanorama.jpg -header \"Range: bytes=0-200\"" << endl;
        cout << "useful  keys: -verb {GET|HEAD|PUT}  -page str -server str -port number -version [-header something]+ " << endl;
        cout << "default keys: -verb GET -server clanmills.com -page robin.shtml -port 80 -version 1.0" << endl;
        cout << "export http_proxy=url eg http_proxy=http://64.62.247.244:80" << endl;
        return 0;
    }

    dict_t response;
    dict_t request;
    string errors;

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
    cout << "result = " << result << endl;
    cout << "errors = " << errors << endl;
    cout << endl;

    for ( dict_i it = response.begin() ; it != response.end() ; it++ ) {
        cout << it->first << " -> ";
        if ( it->first ==  "body") cout << "# " << it->second.length();
        else                       cout << it->second;
        cout << endl;
    }

    return 0;
}

// That's all Folks!
////
