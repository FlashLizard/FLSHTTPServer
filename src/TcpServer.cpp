#pragma once
#include <future>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "TcpServer.hpp"
#include "Log.hpp"

ConnectedTcpSocket::ConnectedTcpSocket(SOCKET socket, int sid) : socket(socket), sid(sid)
{
}

int ConnectedTcpSocket::getSid()
{
	return this->sid;
}

int ConnectedTcpSocket::send(char const *data, int len)
{
	return ::send(this->socket, data, len, 0);
}

TcpSocket::TcpSocket(string ipAddr, int port, TcpCallback callback) : ipAddr(ipAddr), port(port), callback(callback)
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

	auto genSession = [](SOCKET &sessionSocket, int sid, const TcpCallback &callback)
	{
		std::shared_ptr<bool> hadMovePtr = std::make_shared<bool>(false);
		
		// 异步处理sessionSocket
		auto sessionTmp = std::async(std::launch::async, [&sessionSocket, hadMovePtr, sid, &callback]()
									 {
			auto socket = std::move(sessionSocket);
			auto connection = ConnectedTcpSocket(socket, sid);
			*hadMovePtr = true;
			// 设置接收缓冲区
			char recvBuf[MAX_BUFFER_SIZE];
			while (true)
			{
				memset(recvBuf, '\0', sizeof(recvBuf));
				int rtn = recv(socket, recvBuf, sizeof(recvBuf), 0);
				bool status = true;
				if (rtn > 0)
				{
					Log::info("Received %d bytes from Client%d: \n%s\n", rtn, sid, recvBuf);
					Log::debug("callback client%d", sid);
					status = callback(connection, recvBuf, rtn);
				}
				if (!status||rtn == 0)
				{ // 否则是收到了客户端断开连接的请求，也算可读事件。但rtn = 0
					Log::info("Client%d leaving ...\n", sid);
					int status = closesocket(socket); // 既然client离开了，就关闭sessionSocket
					if (status == SOCKET_ERROR)
					{
						Log::error("Client%d close fail", sid);
						return 2;
					}
					Log::debug("Client%d Close Success", sid);
					return 0;
				}
			} });

		// 等待sessionSocket被move, 否则可能在sessionSocket被move之前sessionSocket被析构
		while (!(*hadMovePtr))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		return sessionTmp;
	};
	SOCKET sessionSocket;
	std::vector<std::future<int>> sessionList;
	while (true)
	{
		Log::debug("wait for new accept");
		sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
		if (sessionSocket != INVALID_SOCKET)
		{
			this->clientCnt++;
			auto sid = this->clientCnt;
			Log::info("Client%d connected (From %u.%u.%u.%u:%u)\n", sid,
					  clientAddr.sin_addr.S_un.S_un_b.s_b1,
					  clientAddr.sin_addr.S_un.S_un_b.s_b2,
					  clientAddr.sin_addr.S_un.S_un_b.s_b3,
					  clientAddr.sin_addr.S_un.S_un_b.s_b4,
					  clientAddr.sin_port);
			sessionList.push_back(genSession(sessionSocket, sid, this->callback));
		}
	}
}