#pragma once
#include "winsock2.h"
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <queue>
#pragma comment(lib,"ws2_32.lib")

using std::string;
using std::queue;
using std::pair;

class TcpSocket;
class ConnectedTcpSocket;

typedef bool (*TcpCallback)(ConnectedTcpSocket &socket, char *recvData, int len);
class TcpSocket {
private:
	int port;
	int clientCnt = 0;
	queue<pair<char*, int> > sendDataQueue;
	string ipAddr;
	TcpCallback (*genTcpCallback)();
	bool shouldClose = false;

public:
	TcpSocket(string ipAddr, int port, TcpCallback (*genTcpCallback)());
	void setSendData(char* sendData, int sentDataLen);
	void close();
	int multiRun();
	int run();

};

class ConnectedTcpSocket {
private:
	SOCKET socket;
public:
	ConnectedTcpSocket(SOCKET socket);
	int send(char *data, int len);
	int close();
};
