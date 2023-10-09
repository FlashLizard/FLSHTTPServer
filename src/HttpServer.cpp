#include "HttpServer.hpp"
#include "Log.hpp"
namespace http
{
	//获取系统时间
	std::string httpGmtime()
	{
		time_t now = time(0);
		tm *gmt = gmtime(&now);

		// http://en.cppreference.com/w/c/chrono/strftime
		// e.g.: Sat, 22 Aug 2015 11:48:50 GMT
		//       5+   3+4+   5+   9+       3   = 29
		const char *fmt = "%a, %d %b %Y %H:%M:%S GMT";
		char tStr[30];
		strftime(tStr, sizeof(tStr), fmt, gmt);
		return tStr;
	}

	HttpRequest::HttpRequest() {}
	HttpRequest::HttpRequest(string method, string url, string version, const std::map<string, string> &fields, const std::vector<char> &body)
	{
		this->method = method;
		this->url = url;
		this->version = version;
		this->fields = std::map<string, string>(fields);
		this->body = std::vector<char>(body);
	}
	string HttpRequest::getField(string key)
	{
		if (this->fields.find(key) != this->fields.end())
			return this->fields[key];
		return "";
	}
	std::vector<char> HttpRequest::toCharArr()
	{
		std::vector<char> ret;
		string tmp;
		tmp += this->method + " " + this->url + " HTTP/" + this->version + "\r\n";
		for (auto &field : this->fields)
		{
			tmp += field.first + ": " + field.second + "\r\n";
		}
		tmp += "\r\n";
		ret = std::vector<char>(tmp.begin(), tmp.end());
		ret.insert(ret.end(), this->body.begin(), this->body.end());
		return ret;
	}

	HttpResponse::HttpResponse()
	{
		this->version = "1.1";
		this->statusCode = "200";
		this->statusMessage = "OK";
		this->fields = std::map<string, string>{
			{"Content-Type", "text/html; charset=UTF-8"},
			{"Connection", "keep-alive"},
			{"Server", "FLSHttpServer"},
			{"Date", httpGmtime()}};
	}
	HttpResponse::HttpResponse(string version, string statusCode, string statusMessage, const std::map<string, string> &fields, const std::vector<char> &body)
	{
		this->version = version;
		this->statusCode = statusCode;
		this->statusMessage = statusMessage;
		this->fields = std::map<string, string>(fields);
		this->body = std::vector<char>(body);
	}
	std::vector<char> HttpResponse::toCharArr()
	{
		std::vector<char> ret;
		string tmp;
		tmp += "HTTP/" + this->version + " " + this->statusCode + " " + this->statusMessage + "\r\n";
		for (auto &field : this->fields)
		{
			tmp += field.first + ": " + field.second + "\r\n";
		}
		tmp += "\r\n";
		ret = std::vector<char>(tmp.begin(), tmp.end());
		ret.insert(ret.end(), this->body.begin(), this->body.end());
		return ret;
	}
	string HttpResponse::getField(string key)
	{
		if (this->fields.find(key) != this->fields.end())
			return this->fields[key];
		return "";
	}
	void HttpResponse::setField(string key, string value)
	{
		this->fields[key] = value;
	}

	HttpServer::HttpServer(string ipAddr, int port, HttpCallback httpCallback)
	{
		this->ipAddr = ipAddr;
		this->port = port;
		this->httpCallback = httpCallback;
	}

	//给TCPServer调用的回调函数
	bool HttpServer::callback(ConnectedTcpSocket &socket, char *recvData, int len)
	{
		HttpRequest request;
		HttpResponse response;
		bool status = true;
		if (parseRequest(recvData, len, request))
		{
			status = this->httpCallback(socket.getSid(), request, response);
		}
		else
		{
			response.setField("Connection", "close");
			response.statusCode = "400";
			response.statusMessage = "Bad Request";
		}
		std::vector<char> tmp = response.toCharArr();
		char *buffer = new char[tmp.size()];
		if (!tmp.empty())
		{
			memcpy(buffer, &tmp[0], tmp.size() * sizeof(char));
		}
		socket.send(buffer, tmp.size());
		delete[] buffer;
		if (request.getField("Connection") == "close" || response.getField("Connection") == "close")
		{
			return false;
		}
		return true;
	}
	//解析HTTP请求为HttpRequest对象
	bool HttpServer::parseRequest(char *str, int len, HttpRequest &outRequest)
	{
		string method;
		string url;
		string version;
		std::vector<char> body;
		string data = string(str, 0, len);
		std::stringstream ss(data);
		string line;
		int pos = 0;

		std::getline(ss, line);
		std::map<string, string> fields;
		std::cmatch match;
		auto topLineRegex = std::regex("([A-Z]+) ([^ ]+) HTTP\\/([0-9.]+)\r");
		auto ret = std::regex_match(line.c_str(), match, topLineRegex);
		if (!ret)
		{
			return false;
		}
		method = match[1];
		url = match[2];
		version = match[3];
		pos+=line.length()+1;

		while (std::getline(ss, line))
		{
			pos+=line.length()+1;
			if (line == "\r")
			{
				break;
			}
			auto pos = line.find(": ");
			if (pos == -1)
			{
				return false;
			}
			auto key = line.substr(0, pos);
			auto value = line.substr(pos + 2, line.length() - pos - 2 - 1);
			if (HTTP_REQUEST_FIELDS.find(key) != HTTP_REQUEST_FIELDS.end())
			{
				fields[key] = value;
			}
		}

		if(pos<len)
			body = std::vector<char>(str+pos, str+len);
		outRequest = HttpRequest(method, url, version, fields, body);
		return true;
	}
	void HttpServer::run()
	{
		TcpSocket socket = TcpSocket(this->ipAddr, this->port, [&](ConnectedTcpSocket &socket, char *recvData, int len)
									 { return callback(socket, recvData, len); });
		socket.multiRun();
	}
}