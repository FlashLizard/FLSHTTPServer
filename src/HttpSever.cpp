#include <iostream>
#include <cstring>
#include "TcpSever.hpp"
#include "Log.hpp"

bool callback(TcpSocket &socket, char *recvData, int len)
{
	string str;
	do
	{
		std::cin >> str;
		if (str == "exit")
		{
			socket.close();
			return false;
		}
	} while (str.length() == 0 || str == "\n");
	socket.setSendData((char *)str.c_str(), str.length()+1);
	return true;
}

int main()
{
	TcpSocket socket = TcpSocket("0.0.0.0", 5050, [](){return callback;});
	socket.run();
	return 0;
}