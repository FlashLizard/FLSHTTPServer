# FLSHTTPServer

## Introduction

一个简陋的HTTP服务器(支持多连接)

## Usage

- TcpServer.hpp

```cpp
#include "TcpServer.hpp"
bool callback(ConnectedTcpSocket &socket, char *recvData, int len) {
    // do something
    string str = "hello world";
    socket.send(str.c_str(), str.length());
    return true;
}

int main()
{
	auto server = TcpServer(this->ipAddr, this->port, callback);
	server.multiRun();
    return 0;
}
```

callback函数原型如上, 该函数会在每次接收到数据时被调用, 该函数需要返回一个bool值, 用于判断是否继续接收数据, 如果返回false, 则会关闭该连接.
callback的第一个参数是一个ConnectedTcpSocket对象, 可以调用send传递响应报文.

- HttpServer.hpp

```cpp
#include "HttpServer.hpp"
#include <vector>
using namespace http;
using namespace std;
bool callback(int sid, const http::HttpRequest &request, http::HttpResponse &outResponse) {
    // do something
    outResponse.setField("Connection", "close");
    outResponse.statusCode = "200";
    outResponse.statusMessage = "OK";
    string str = "hello world";
    auto body = vector<char>(str.begin(), str.end());
    outResponse.body = body;
    return true;
}

int main()
{
	auto server = HttpServer(this->ipAddr, this->port, callback);
	server.run();
    return 0;
}

- Server.hpp

```cpp
#include "Server.hpp"

int main()
{
    string ipAddr = "0.0.0.0";
    int port = 5050;
    string rootPath = "."
    Server server(ipAddr, port, rootPath);
    server.run();
}

## 简单测试

下载安装xmake, 并在项目根目录下执行
```sh
xmake build httpServer
xmake run httpServer
```

成功运行后, 可用浏览器访问 http://localhost:5050/ 查看具体效果.