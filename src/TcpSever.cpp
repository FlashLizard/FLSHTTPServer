#pragma once
#include <future>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "TcpSever.hpp"
#include "Log.hpp"

ConnectedTcpSocket::ConnectedTcpSocket(SOCKET socket) : socket(socket)
{
}

int ConnectedTcpSocket::send(char *data, int len)
{
	return ::send(this->socket, data, len, 0);
}

int ConnectedTcpSocket::close()
{
	return closesocket(this->socket);
}

TcpSocket::TcpSocket(string ipAddr, int port, TcpCallback (*genTcpCallback)()) : ipAddr(ipAddr), port(port), genTcpCallback(genTcpCallback)
{
}

int TcpSocket::multiRun()
{
	WSADATA wsaData;
	int nRc = WSAStartup(0x0202, &wsaData);
	if (nRc)
	{
		Log::error("Winsock startup failed with error!\n");
	}

	if (wsaData.wVersion != 0x0202)
	{
		Log::error("Winsock version is not correct!\n");
	}

	Log::info("Winsock startup Ok!\n");

	// 监听socket
	SOCKET srvSocket;

	// 服务器地址和客户端地址
	sockaddr_in addr, clientAddr;

	// ip地址长度
	int addrLen;

	// 创建监听socket
	srvSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (srvSocket != INVALID_SOCKET)
		Log::info("Socket create Ok!\n");

	// 设置服务器的端口和地址
	addr.sin_family = AF_INET;
	addr.sin_port = htons(this->port);
	//((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)
	unsigned long ip = inet_addr(ipAddr.c_str());
	if (ip == NULL)
	{
		auto he = gethostbyname(ipAddr.c_str());
		if (he != NULL)
		{
			ip = inet_addr(he->h_name);
		}
	}
	if (ip == NULL && ipAddr != "0.0.0.0")
	{
		Log::error("IP resolve fail");
		return 1;
	}
	addr.sin_addr.S_un.S_addr = ip;

	// binding
	int rtn = bind(srvSocket, (LPSOCKADDR)&addr, sizeof(addr));
	if (rtn != SOCKET_ERROR)
		Log::info("Socket bind Ok!\n");

	// 监听
	rtn = listen(srvSocket, 5);
	if (rtn != SOCKET_ERROR)
		Log::info("Socket listen Ok!\n");

	clientAddr.sin_family = AF_INET;
	addrLen = sizeof(clientAddr);

	auto genSession = [](SOCKET &sessionSocket, int sid, TcpCallback callback)
	{
		Log::info("Client%d listen one client request!\n", sid);
		std::shared_ptr<bool> hadMovePtr = std::make_shared<bool>(false);
		auto sessionTmp = std::async(std::launch::async, [&sessionSocket, hadMovePtr, sid, callback]()
									 {
			auto socket = std::move(sessionSocket);
			auto connection = ConnectedTcpSocket(socket);
			*hadMovePtr = true;
			// 设置接收缓冲区
			char recvBuf[4096];
			while (true)
			{
				memset(recvBuf, '\0', 4096);
				int rtn = recv(socket, recvBuf, 256, 0);
				if (rtn > 0)
				{
					Log::debug("Received %d bytes from client%d: %s\n", rtn, sid, recvBuf);
					callback(connection, recvBuf, rtn);
					Log::debug("callback client%d", sid);
				}
				else
				{ // 否则是收到了客户端断开连接的请求，也算可读事件。但rtn = 0
					Log::info("Client%d leaving ...\n", sid);
					int status = connection.close(); // 既然client离开了，就关闭sessionSocket
					if (status == SOCKET_ERROR)
					{
						Log::error("Client%d close fail", sid);
						return 2;
					}
					Log::info("Client%d Close Success", sid);
					return 0;
				}
			} });
		while (!(*hadMovePtr))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		return sessionTmp;
	};
	SOCKET sessionSocket;
	std::vector<std::future<int> > sessionList;
	while (true)
	{
		Log::debug("wait for new accept");
		sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
		if (sessionSocket != INVALID_SOCKET)
		{
			this->clientCnt++;
			auto sid = this->clientCnt;
			auto callback = this->genTcpCallback();
			sessionList.push_back(genSession(sessionSocket, sid, callback));
		}
	}
}