/*
 * http.cpp
 */

#include "http.hpp"

#ifdef  _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#define SLEEP       1000
#define SNOOZE         0

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#if defined(__MINGW32__) || defined(__MINGW64__)
#define __MINGW__
#define  fopen_S(f,n,a)  f=fopen(n,a)
#endif

////////////////////////////////////////
// platform specific code
#if defined(WIN32) || defined(_MSC_VER)
#include <string.h>
#include <windows.h>
#include <io.h>
#ifndef  __MINGW__
#define  snprintf sprintf_s
#define  write    _write
#define  read     _read
#define  close    _close
#define  strdup   _strdup
#define  stat     _stat
#define  fopen_S(f,n,a)  fopen_s(&f,n,a)
#endif
#else
////////////////////////////////////////
// Unix or Mac

#define  closesocket close

#include <unistd.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define fopen_S(f,n,o) f=fopen(n,o)
#define WINAPI
typedef unsigned long DWORD ;

#define SOCKET_ERROR        -1
#define WSAEWOULDBLOCK  EINPROGRESS
#define WSAENOTCONN     EAGAIN

int WSAGetLastError()
{
    return errno ;
}

void Sleep(int millisecs)
{
    const struct timespec rqtp = { 0 , millisecs*1000000 } ;
    struct timespec   rmtp ;
    nanosleep(&rqtp,&rmtp);
}

#endif

////////////////////////////////////////
// code
const char* httpTemplate =
"%s %s HTTP %s\r\n"            // $verb $page $version
"User-Agent: exiv2http/1.0.0\r\n"
"Accept: */*\r\n"
"Host: %s\r\n"                 // $servername
"%s"                           // $header
"\r\n"
;

#define lengthof(x) (sizeof(x)/sizeof((x)[0]))
#define white(c) ((c == ' ') || (c == '\t'))

#define FINISH          -999
#define OK(s)    (200 <= s  && s < 300)

const char*   blankLines[] =
{       "\r\n\r\n"             // this is the standard
,       "\n\n"                 // this is commonly sent by CGI scripts
}  ;

int             snooze    = SNOOZE    ;
int             sleep_    =  SLEEP    ;

static void report(const char* msg,std::string& errors)
{
    errors += std::string(msg) + '\n';
}

static int forgive(int n,int& err)
{
    err  = WSAGetLastError() ;
    if ( !n && !err ) return FINISH ;
#ifndef WIN32
    if ( n == 0 ) return FINISH ; // server hungup
#endif
    bool bForgive = err == WSAEWOULDBLOCK || err == WSAENOTCONN ;
    bool bError   = n == SOCKET_ERROR ;
    if ( bError && bForgive ) return 0 ;
    return n ;
}

static int error(std::string errors,const char* msg,const char* x=NULL,const char* y=NULL,int z=0);
static int error(std::string errors,const char* msg,const char* x     ,const char* y     ,int z )
{
    char buffer[512] ;
    snprintf(buffer,sizeof buffer,msg,x,y,z) ;
    if ( errno ) {
        perror(buffer) ;
    } else {
        fprintf(stderr,"%s\n",buffer);
    }
    report(buffer,errors) ;
    return -1 ;
}

static void flushBuffer(const char* buffer,size_t start,int& end,std::string& file)
{
    file += std::string(buffer+start,end-start) ;
    end = 0 ;
}

static int makeNonBlocking(int sockfd)
{
#ifdef WIN32
  ULONG ioctl_opt = 1;
  return ioctlsocket(sockfd, FIONBIO, &ioctl_opt);
#else
  int    result  = fcntl(sockfd, F_SETFL, O_NONBLOCK);
  return result >= 0 ? result : SOCKET_ERROR ;
#endif
}

int http(dict_t& request,dict_t& response,std::string& errors)
{
    if ( !request.count("port")   ) request["port"  ]  = "80" ;
    if ( !request.count("verb")   ) request["verb"  ]  = "GET";
    if ( !request.count("header") ) request["header"]  = ""   ;
    if ( !request.count("version")) request["version"] = "1.0";
    
    std::string file;
    errors     = "";
    int result = 0;

    ////////////////////////////////////
    // Windows specific code
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    const char* servername = request["server" ].c_str();
    const char* page       = request["page"   ].c_str();
    const char* port       = request["port"   ].c_str();
    const char* verb       = request["verb"   ].c_str();
    const char* header     = request["header" ].c_str();
    const char* version    = request["version"].c_str();
    
    const char* servername_p = servername;
    const char* port_p       = port      ;
    std::string url = std::string("http://") + request["server"] + request["page"];

    // parse and change server if using a proxy
    const char* PROXI  = "HTTP_PROXY";
    const char* proxi  = "http_proxy";
    const char* PROXY  = getenv(PROXI);
    const char* proxy  = getenv(proxi);
    bool        proxb  = PROXY || proxy;
    const char* prox   = proxb ? (proxy?proxy:PROXY):"";
    Uri   Proxy = Uri::Parse(prox);
	if (  proxb ) {
		servername_p = Proxy.Host.c_str();
		port_p       = Proxy.Port.c_str();
        page         = url.c_str();
        std::string  p(proxy?proxi:PROXI);
        std::cerr << p << '=' << prox << " page = " << page << std::endl;
	}
	
    ////////////////////////////////////
    // open the socket
    int     sockfd = (int) socket(AF_INET , SOCK_STREAM,IPPROTO_TCP) ;
    if (    sockfd < 0 ) return error("unable to create socket\n",NULL,NULL,0) ;

    // connect the socket to the server
    int     server  = -1 ;

    // fill in the address
    struct  sockaddr_in serv_addr   ;
    int                 serv_len = sizeof(serv_addr);
    memset((char *)&serv_addr,0,serv_len);

    serv_addr.sin_addr.s_addr   = inet_addr(servername_p);
    serv_addr.sin_family        = AF_INET    ;
    serv_addr.sin_port          = htons(atoi(port_p));

    // convert unknown servername into IP address
    // http://publib.boulder.ibm.com/infocenter/iseries/v5r3/index.jsp?topic=/rzab6/rzab6uafinet.htm
    if (serv_addr.sin_addr.s_addr == (unsigned long)INADDR_NONE)
    {
        struct hostent* host = gethostbyname(servername_p);
        if ( !host )  return error("no such host",servername_p,NULL,0);
        memcpy(&serv_addr.sin_addr,host->h_addr,sizeof(serv_addr.sin_addr));
    }
    
    makeNonBlocking(sockfd) ;

    ////////////////////////////////////
    // and connect
    server = connect(sockfd, (const struct sockaddr *) &serv_addr, serv_len) ;
    if ( server == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK )
        return error(errors,"error - unable to connect to server = %s port = %s wsa_error = %d",servername_p,port_p,WSAGetLastError());

    char   buffer[32*1024+1];
    size_t buff_l= sizeof buffer - 1 ;

    ////////////////////////////////////
    // format the request
    int    n  = snprintf(buffer,buff_l,httpTemplate,verb,page,version,servername,header) ;
    buffer[n] = 0 ;
    response["requestheaders"]=std::string(buffer,n);


    ////////////////////////////////////
    // send the header (we'll have to wait for the connection by the non-blocking socket)
    while ( sleep_ >= 0 && send(sockfd,buffer,n,0) == SOCKET_ERROR /* && WSAGetLastError() == WSAENOTCONN */ ) {
        Sleep(snooze) ;
        sleep_ -= snooze ;
    }

    if ( sleep_ < 0 )
        return error(errors,"error - timeout connecting to server = %s port = %s wsa_error = %d",servername,port,WSAGetLastError());

    int    end   = 0         ; // write position in buffer
    bool   bSearching = true ; // looking for headers in the response
    int    status= 200       ; // assume happiness

    ////////////////////////////////////
    // read and process the response
    int err ;
    n=forgive(recv(sockfd,buffer,(int)buff_l,0),err) ;
    while ( n >= 0 && OK(status) ) {
        if ( n ) {
            end += n ;
            buffer[end] = 0 ;

            size_t body = 0         ; // start of body
            if ( bSearching ) {

                // search for the body
                for ( size_t b = 0 ; bSearching && b < lengthof(blankLines) ; b++ ) {
                    if ( strstr(buffer,blankLines[b]) ) {
                        bSearching = false ;
                        body   = (int) ( strstr(buffer,blankLines[b]) - buffer ) + strlen(blankLines[b]) ;
                        status = atoi(strchr(buffer,' ')) ;
                    }
                }

                // parse the response headers
                char* h = buffer;
                char  C = ':';
                char  N = '\n';
                h       = strchr(h,N)+1;
                response[""]=std::string(buffer).substr(0,h-buffer-2);
                result = atoi(strchr(buffer,' '));
                char* c = strchr(h,C);
                char* n = strchr(h,N);
                while ( c && n && c < n && h < buffer+body ) {
                    std::string key(h);
                    std::string value(c+1);
                    key   = key.substr(0,c-h);
                    value = value.substr(0,n-c-1);
                    response[key]=value;
                    h = n+1;
                    c = strchr(h,C);
                    n = strchr(h,N);
                }
            }

            // if the bufffer's full and we're still searching - give up!
            // this handles the possibility that there are no headers
            if ( bSearching && buff_l-end < 10 ) {
                bSearching = false ;
                body  = 0 ;
            }
            if ( !bSearching && OK(status) ) {
                flushBuffer(buffer,body,end,file);
            }
        }
        n=forgive(recv(sockfd,buffer+end,(int)(buff_l-end),0),err) ;
        if ( !n ) {
            Sleep(snooze) ;
            sleep_ -= snooze ;
            if ( sleep_ < 0 ) n = FINISH ;
        }
    }

    if ( n != FINISH || !OK(status) ) {
        snprintf(buffer,sizeof buffer,"wsa_error = %d,n = %d,sleep_ = %d status = %d"
                ,   WSAGetLastError()
                ,   n
                ,   sleep_
                ,   status
                ) ;
        error(errors,buffer,NULL,NULL,0) ;
    } else if ( bSearching && OK(status) ) {
        if ( end ) {
        //  we finished OK without finding headers, flush the buffer
            flushBuffer(buffer,0,end,file) ;
        } else {
            return error(errors,"error - no response from server = %s port = %s wsa_error = %d",servername,port,WSAGetLastError());
        }
    }

    ////////////////////////////////////
    // close sockets
    closesocket(server) ;
    closesocket(sockfd) ;
    response["body"]=file;
    return result;
}

// http://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform
Uri Uri::Parse(const std::string &uri)
{
	Uri result;

	typedef std::string::const_iterator iterator_t;

	if ( !uri.length() )  return result;

	iterator_t uriEnd = uri.end();

	// get query start
	iterator_t queryStart = std::find(uri.begin(), uriEnd, '?');

	// protocol
	iterator_t protocolStart = uri.begin();
	iterator_t protocolEnd   = std::find(protocolStart, uriEnd, ':');            //"://");

	if (protocolEnd != uriEnd)
	{
		std::string prot = &*(protocolEnd);
		if ((prot.length() > 3) && (prot.substr(0, 3) == "://"))
		{
			result.Protocol = std::string(protocolStart, protocolEnd);
			protocolEnd += 3;   //      ://
		}
		else
			protocolEnd = uri.begin();  // no protocol
	}
	else
		protocolEnd = uri.begin();  // no protocol

	// host
	iterator_t hostStart = protocolEnd;
	iterator_t pathStart = std::find(hostStart, uriEnd, '/');  // get pathStart

	iterator_t hostEnd = std::find(protocolEnd, 
		(pathStart != uriEnd) ? pathStart : queryStart,
		':');  // check for port

	result.Host = std::string(hostStart, hostEnd);

	// port
	if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':'))  // we have a port
	{
		hostEnd++;
		iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
		result.Port = std::string(hostEnd, portEnd);
	}
	if ( !result.Port.length() && result.Protocol == "http" ) result.Port = "80";

	// path
	if (pathStart != uriEnd)
		result.Path = std::string(pathStart, queryStart);

	// query
	if (queryStart != uriEnd)
		result.QueryString = std::string(queryStart, uri.end());

	return result;
}   // Parse

// That's all Folks
////
