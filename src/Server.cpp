#include "Server.hpp"
#include "Log.hpp"

#include <vector>

bool Server::callback(int sid, const http::HttpRequest &request, http::HttpResponse &outResponse)
{
    string url = request.url;
    if (url == "/")
    {
        url = "/index.html";
    }
    string path = this->rootPath + url;
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        outResponse.statusCode = "404";
        outResponse.statusMessage = "Not Found";
        outResponse.setField("Connection", "close");
        Log::error("Client%d can't open file %s", sid, path.c_str());
        return false;
    }
    else
    {
        try
        {
            outResponse.setField("Connection", "close");
            outResponse.statusCode = "200";
            outResponse.statusMessage = "OK";
            file.seekg(0, file.end);
            int length = file.tellg();
            file.seekg(0, file.beg);
            char *buffer = new char[length + 1];
            file.read(buffer, length);
            file.close();
            Log::info("Client%d open file %s success", sid, path.c_str());
            outResponse.body = std::vector<char>(buffer, buffer + length);
            std::regex reg(".*\\.(.*)");
            std::smatch match;
            if (std::regex_match(url, match, reg))
            {
                string ext = match[1];
                if (ext == "html")
                {
                    outResponse.setField("Content-Type", "text/html; charset=UTF-8");
                }
                else if (ext == "css")
                {
                    outResponse.setField("Content-Type", "text/css; charset=UTF-8");
                }
                else if (ext == "js")
                {
                    outResponse.setField("Content-Type", "application/javascript; charset=UTF-8");
                }
                else if (ext == "png")
                {
                    outResponse.setField("Content-Type", "image/png");
                }
                else if (ext == "jpg")
                {
                    outResponse.setField("Content-Type", "image/jpeg");
                }
                else if (ext == "gif")
                {
                    outResponse.setField("Content-Type", "image/gif");
                }
                else if (ext == "ico")
                {
                    outResponse.setField("Content-Type", "image/x-icon");
                }
            }
            return true;
        }
        catch (std::exception &e)
        {
            Log::error("Error %s occurred When Client%d open file %s",e.what(), sid, path.c_str());
            outResponse.statusCode = "500";
            outResponse.statusMessage = "Internal Server Error";
            outResponse.setField("Connection", "close");
            return false;
        }
    }
}
Server::Server(string ipAddr, int port, string rootPath)
{
    this->ipAddr = ipAddr;
    this->port = port;
    this->rootPath = rootPath;
}
void Server::run()
{
    http::HttpServer httpServer(this->ipAddr, this->port, std::bind(&Server::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    httpServer.run();
}