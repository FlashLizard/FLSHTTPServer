#ifndef FLS_HTTP_SERVER
#define FLS_HTTP_SERVER
#include <iostream>
#include <cstring>
#include <functional>
#include <sstream>
#include <regex>
#include <ctime>
#include <map>
#include <set>
#include <vector>
#include "TcpServer.hpp"

namespace http
{

    class HttpRequest;
    class HttpResponse;
    typedef std::function<bool(int sid, const HttpRequest &request, HttpResponse &outResponse)> HttpCallback;

    const std::set<string> HTTP_REQUEST_FIELDS{"Host",
                                               "Connection",
                                               "Cache-control",
                                               "Upgrade-insecure-requests",
                                               "User-Agent",
                                               "Accept",
                                               "Accept-Encoding",
                                               "Accept-Language",
                                               "Cookie"};

    std::string httpGmtime();

    class HttpRequest
    {
    public:
        string method;
        string url;
        string version;
        std::vector<char> body;
        std::map<string, string> fields;

    public:
        HttpRequest();
        HttpRequest(string method, string url, string version, const std::map<string, string> &fields,const std::vector<char> &body);
        string getField(string key);
        std::vector<char> toCharArr();
    };

    class HttpResponse
    {
    public:
        string version;
        string statusCode;
        string statusMessage;
        std::vector<char> body;
        std::map<string, string> fields;

    public:
        HttpResponse();
        HttpResponse(string version, string statusCode, string statusMessage,const std::map<string, string> &fields,const std::vector<char> &body);
        std::vector<char> toCharArr();
        string getField(string key);
        void setField(string key, string value);
    };

    class HttpServer
    {
    private:
        int port;
        int clientCnt = 0;
        string ipAddr;
        HttpCallback httpCallback;
    private:
        bool callback(ConnectedTcpSocket &socket, char *recvData, int len);
        static bool parseRequest(char *str, int len, HttpRequest &outRequest);
    public:
        HttpServer(string ipAddr, int port, HttpCallback httpCallback);
        void run();
    };
}
#endif