#ifndef FLS_SERVER
#define FLS_SERVER
#include "HttpServer.hpp"
#include <iostream>
#include <fstream>
#include <regex>

class Server
{
private:
    string ipAddr;
    int port;
    string rootPath;
private:
    bool callback(int sid, const http::HttpRequest &request, http::HttpResponse &outResponse);
public:
    Server(string ipAddr, int port, string rootPath);
    void run();
};

#endif