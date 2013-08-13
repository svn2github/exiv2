// farmer.cpp : Defines the entry point for the application.
//
// This code is based on some cross-platform code available from clanmills.com
// clanmills.com does not reserve any ownership or rights to this code
//
// To build on   Linux/Mac/Cygwin:
//               make farmer
// To build for  Edge:
// (bash users)  export C=/opt/crosstool/gcc-3.4.3-glibc-2.3.5/arm-xscale_soft-linux-gnu/arm-xscale_soft-linux-gnu/bin/gcc
// (csh users)   setenv C /opt/..../gcc
//               env CC=$C LD=$C PATH=`dirname $C`:$PATH make -B farmer
// To build with MSVC:
//               use the project files in DevStudio (2005 and later)
// To build on DOS DevStudio 2005 and later:
//               run vcvars32.bat for your compiler!
//               cl farmer.cpp /D WIN32 /D NDEBUG /D _WINDOWS /D UNICODE /D _CONSOLE /EHsc /link WSock32.lib
// To build on MinGW:
//               g++ farmer.cpp -D_CONSOLE -lwsock32 -o farmer.exe
//

#define SLEEP       1000
#define SNOOZE         0
#define MULTITHREADED  0

//
// syntax:  farmer server                  file         output (or -)
// example: farmer clanmills.com          /robin.shtml  robin.txt
//          farmer 192.168.119.98  8080   /something.js something.js
//
// farmer.exe is a windows application (no dos box)
// farmerc.exe is a console application (run in a dos box)
//
// Environment
// --- RELEASE build ---     (enable logging by default)
// FARMER_LOG             1   1 = report to log.  0 = report in MessageBox
// FARMER_LOG_SUCCESS     1   0 = don't report success 1 = report success
// --- Debug build ---       (reports to user by default)
// FARMER_LOG             0
// FARMER_LOG_SUCCESS     0
//
// --- All builds ---
// FARMER_LOG_LIMIT 1000000
// FARMER_SNOOZE        100   milliseconds for little sleep
// FARMER_SLEEP        5000   milliseconds to timeout
//
// Log is written to farmer.log
//
// Set environment strings in DOS with
// set FARMER_LOG=1
//
// Delete environment string in DOS with
// set FARMER_LOG=
//
// List environment strings in DOS with
// set
//

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
#define   _CONSOLE
//        MULTITHREADED enable Windows Threads (don't build for UNIX)
#ifdef    MULTITHREADED
#undef    MULTITHREADED
#define   MULTITHREADED 0
#endif

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

int MessageBoxA(void*,const char* msg,const char* title,int)
{
    return printf("%s:%s\n",title,msg) ;
}

#endif

////////////////////////////////////////
// code
const char* httpTemplate =
"GET %s%s HTTP/1.0\r\n"              // SLASH $page
"User-Agent: farmer/1.0.0\r\n"
"If-Modified-Since: Sat, 1 Jan 2000 00:00:00 GMT\r\n"
"Accept: */*\r\n"
"Host: %s:%s\r\n"                   // $servername,$port
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

char*           title     =   NULL    ;
int             snooze    = SNOOZE    ;
int             sleep_    =  SLEEP    ;
int             nLogLimit = 1024*1024 ;

// thread communication stuff
char*           argv[10]            ; // argument vector
size_t          argc = 0            ; // parameter counter
volatile bool   bDone     = false   ;

#ifdef NDEBUG
volatile int    bLog        = 1 ;
volatile int    bLogSuccess = 1 ;
#else
volatile int    bLogSuccess = 0 ;
volatile int    bLog        = 0 ;
#endif

void report(const char* msg)
{
    if ( !bLog )
    {
#ifndef _CONSOLE
        MessageBoxA(NULL,msg,title ? title : "farmer",0) ;
#endif
    } else {
        bool bDone = false ;
        int  tries   = 10 ;
        while ( tries-- && !bDone ) {
            struct stat buf   ;
            buf.st_size   = 0 ;
            const char* logname = "farmer.log" ;
            bool  bDelete = (stat(logname, &buf ) == 0) && buf.st_size > nLogLimit ;

            FILE* log ;
            fopen_S(log,logname,bDelete?"w":"a") ;
            if ( log ) {
                bDone = true ;
                time_t now   ;
                time (&now)  ;
                char*  t  = strdup(ctime(&now)) ;
                for ( size_t i = 0 ; i < strlen(t) ; i++ )
                    if ( t[i] < ' ' )
                        t[i] = ' ' ;
                fprintf(log,"%s %s %s\n",t,title,msg) ;
                free(t) ;
                fclose(log) ;
            } else {
                Sleep(snooze) ;
            }
        }
    }
}

int forgive(int n,int& err)
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

const char* getEnvString(const char* s,const char* def)
{
    const char* v = getenv(s) ;
    return v ?  v : def  ;
}

int getEnvInt(const char* s,int def)
{
    const char* v = getenv(s) ;
    return v ? atoi(v) : def  ;
}

int error(const char* msg,const char* x=NULL,const char* y=NULL,int z=NULL);
int error(const char* msg,const char* x,const char* y,int z)
{
    char buffer[512] ;
    snprintf(buffer,sizeof buffer,msg,x,y,z) ;
    if ( errno ) {
        perror(buffer) ;
    } else {
        fprintf(stderr,"%s\n",buffer);
    }
    report(buffer) ;
    bDone = true ;
    return -1 ;
}

void flushBuffer(const char* buffer,size_t start,int& end,FILE* file)
{
    if ( file ) fwrite(buffer+start,1,end-start,file) ;
    if ( file == stdout ) {
        bLog        = false ;
        bLogSuccess = false ;
        report(buffer) ;
    }
    end = 0 ;
}

int makeNonBlocking(int sockfd)
{
#ifdef WIN32
  ULONG ioctl_opt = 1;
  return ioctlsocket(sockfd, FIONBIO, &ioctl_opt);
#else
  int    result  = fcntl(sockfd, F_SETFL, O_NONBLOCK);
  return result >= 0 ? result : SOCKET_ERROR ;
#endif
}

DWORD WINAPI mainThread(void* /*pParam*/)
{
    ////////////////////////////////////
    // Windows specific code
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    ////////////////////////////////////
    // Sniff environment
    snooze      = getEnvInt("FARMER_SNOOZE"     ,snooze) ;
    sleep_      = getEnvInt("FARMER_SLEEP"      ,sleep_ ) ;
    bLog        = getEnvInt("FARMER_LOG"        ,bLog  ) ;
    bLogSuccess = getEnvInt("FARMER_LOG_SUCCESS",bLogSuccess) ;
    nLogLimit   = getEnvInt("FARMER_LOG_LIMIT"  ,nLogLimit  ) ;

    ////////////////////////////////////
    // sniff command line
    if ( argc < 4 ) {
        bLog = false ;
        char buffer[1000] ;
        snprintf(buffer,sizeof buffer,
            "syntax: %s host [port] page filename\n"
            "Built: %s %s "
#ifdef NDEBUG
            "release build\n\n"
#else
            "debug build\n\n"
#endif
            "Environment strings:\n"
            "FARMER_LOG\n"
            "FARMER_LOG_SUCCESS\n"
            "FARMER_LOG_LIMIT\n"
            "FARMER_SLEEP\n"
            "FARMER_SNOOZE\n"
            ,argv[0], __DATE__ , __TIME__
            ) ;
        return error(buffer,NULL,NULL,0) ;
    }
    int   arg = 1 ;
    const char* servername = argv[arg++] ;
    const char* port       = argc > 4 ? argv[arg++] : "80" ;
    const char* page       = argv[arg++] ;
    const char* filename   = argv[arg++] ;

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

    serv_addr.sin_addr.s_addr   = inet_addr(servername);
    serv_addr.sin_family        = AF_INET    ;
    serv_addr.sin_port          = htons(atoi(port));

    // convert unknown servername into IP address
    // http://publib.boulder.ibm.com/infocenter/iseries/v5r3/index.jsp?topic=/rzab6/rzab6uafinet.htm
    if (serv_addr.sin_addr.s_addr == (unsigned long)INADDR_NONE)
    {
        struct hostent* host = gethostbyname(servername);
        if ( !host )  return error("no such host",servername,NULL,0);
        memcpy(&serv_addr.sin_addr,host->h_addr,sizeof(serv_addr.sin_addr));
    }

    char   buffer[32*1024+1];
    size_t buff_l= sizeof buffer - 1 ;

    ////////////////////////////////////
    // format the request
    const char* slash =  page[0]=='/' ? "" : "/";
    int    n  = snprintf(buffer,buff_l,httpTemplate,slash,page,servername,port) ;
    buffer[n] = 0 ;

    makeNonBlocking(sockfd) ;

    ////////////////////////////////////
    // and connect
    server = connect(sockfd, (const struct sockaddr *) &serv_addr, serv_len) ;
    if ( server == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK )
        return error("error - unable to connect to server = %s port = %s wsa_error = %d",servername,port,WSAGetLastError());

    ////////////////////////////////////
    // send the header (we'll have to wait for the connection by the non-blocking socket)
    while ( sleep_ >= 0 && send(sockfd,buffer,n,0) == SOCKET_ERROR /* && WSAGetLastError() == WSAENOTCONN */ ) {
        Sleep(snooze) ;
        sleep_ -= snooze ;
    }

    if ( sleep_ < 0 )
        return error("error - timeout connecting to server = %s port = %s wsa_error = %d",servername,port,WSAGetLastError());

    ////////////////////////////////////
    // get ready for the reply
    FILE*    file  = strcmp(filename,"-")==0 ? stdout : NULL ;
    if ( !file ) fopen_S(file,(const char*)filename,"wb") ;

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
                for ( int b = 0 ; bSearching && b < lengthof(blankLines) ; b++ ) {
                    if ( strstr(buffer,blankLines[b]) ) {
                        bSearching = false ;
                        body   = (int) ( strstr(buffer,blankLines[b]) - buffer ) + strlen(blankLines[b]) ;
                        status = atoi(strchr(buffer,' ')) ;
                    }
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
        error(buffer,NULL,NULL,0) ;
    } else if ( bSearching && OK(status) ) {
        if ( end ) {
        //  we finished OK without finding headers, flush the buffer
            flushBuffer(buffer,0,end,file) ;
        } else {
            return error("error - no response from server = %s port = %s wsa_error = %d",servername,port,WSAGetLastError());
        }
    }

    ////////////////////////////////////
    // close sockets
    if ( file ) fclose(file) ;
    closesocket(server) ;
    closesocket(sockfd) ;
    if ( bLogSuccess && OK(status) ) report("SUCCESS")   ;
    bDone = true ;
    return 0;
}

void go()
{
    ////////////////////////////////////
    // do the business
#if MULTITHREADED
    // spin a thread and shoot him later
    DWORD  dwThreadID ;
    HANDLE hThread = CreateThread(NULL,0,mainThread,NULL,0,&dwThreadID) ;

    int snz = getEnvInt("FARMER_SNOOZE"     ,snooze) ;
    int slp  = getEnvInt("FARMER_SLEEP"     ,sleep_ ) ;
    while ( !bDone && slp > 0 ) {
        Sleep(snz) ;
        slp -= snz ;
    }

    ////////////////////////////////////
    // cleanup
    // MessageBoxA(NULL,"goodnight",title,0) ;
    if ( !bDone && bLog ) {
        TerminateThread(hThread,0) ;
        error("thread timed out") ;
    }
#else
    mainThread(NULL) ;
#endif
}

#ifdef _CONSOLE
int main(int argC, char *argV[])
{
    argc = argC < lengthof(argv) ? argC : lengthof(argv) ;
    for ( size_t i = 0 ; i < argc ; i++ ) argv[i] = argV[i] ;
    title = argV[0] ;
    go() ;
    return 0 ;
}
#else
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    ////////////////////////////////////
    // parse lpCmdLine to argc/argv
    int    x    = 0 ; // output pointer
    int    b    = 1 ; // begin in blanks
    size_t n    = wcslen(lpCmdLine) ; // length of command line
    char*  args = (char*) malloc(n+2) ;
    size_t i ;

    argv[argc++] = "farmer" ; // drop in our name
    for (  i = 0 ; i < n && argc < lengthof(argv) ; i++ ) {
        char c = (char) lpCmdLine[i] ;
        if ( white(c) ) {
            c = 0 ;
            b = 1 ; // we're in the blanks
        } else {
            if ( b ) argv[argc++] = args+x ;
            b = 0 ; // not blank
        }
        args[x++] = c ;
    }
    args[x] = 0 ;

    ////////////////////////////////////
    // write title from command line args
    n = n*3 + argc*3 + 20 ; // allocate enough
    title    = (char*) malloc(n) ;
    size_t l = 0 ;
    for (  i = 0 ; i < argc ; i++ ) {
        l += snprintf(title+l,n-l-1,"%s ",argv[i]) ;
    }
    title[l] = 0 ;

    go() ;

    free(args) ;
    free(title) ;
}
#endif
