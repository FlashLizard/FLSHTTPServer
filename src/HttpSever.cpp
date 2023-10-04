#include <iostream>
#include <cstring>

#include "TcpSever.hpp"
#include "Log.hpp"

bool callback(ConnectedTcpSocket &socket, char *recvData, int len)
{
	string str;
	str = "test";
	do
	{
		//std::cin >> str;
		if (str == "exit")
		{
			socket.close();
			return false;
		}
	} while (str.length() == 0 || str == "\n");
	socket.send((char *)str.c_str(), str.length()+1);
	return true;
}


int main()
{

	TcpSocket socket = TcpSocket("0.0.0.0", 5050, [](){return callback;});
	socket.multiRun();
	return 0;
}