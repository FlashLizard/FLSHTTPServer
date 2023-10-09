#ifndef FLS_TCPSERVER
#define FLS_TCPSERVER
#include "winsock2.h"
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <functional>
#include <queue>
#define MAX_BUFFER_SIZE 4096

#pragma comment(lib,"ws2_32.lib")

using std::string;
using std::queue;
using std::pair;

class TcpSocket;
class ConnectedTcpSocket;

typedef std::function<bool(ConnectedTcpSocket &socket, char *recvData, int len)> TcpCallback;
class TcpSocket {
private:
	int port;
	int clientCnt = 0;
	string ipAddr;
	TcpCallback callback;

public:
	TcpSocket(string ipAddr, int port, TcpCallback callback);
	int multiRun();

};

class ConnectedTcpSocket {
private:
	SOCKET socket;
	int sid;
public:
	ConnectedTcpSocket(SOCKET socket, int sid);
	int send(char const *data, int len);
	int getSid();
};

#endif