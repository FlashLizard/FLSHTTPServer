#include "Server.hpp"
#include "Log.hpp"

#include <json/json.h>
#include <iostream>
#include <fstream>
#include <future>

using namespace std;

int main() {
    Log::globalLevel = Log::Info;
    ifstream ifs;
    ifs.open("./config.json", std::ios::binary);
    if(ifs.is_open()) {
        Json::Reader reader;
        Json::Value root;
        if(reader.parse(ifs, root)) {
            cout<<"Test"<<endl;
            string ipAddr = root["ip"].asString();
            int port = root["port"].asInt();
            string rootPath = root["rootPath"].asString();
            Server server(ipAddr, port, rootPath);
            auto serverThread = std::async(std::launch::async, std::bind(&Server::run, &server));
            while(true) {
                string cmd;
                cin>>cmd;
                if(cmd == "exit") {
                    return 0;
                } else {
                    cout<<"Unknown command"<<endl;
                }
            }
        }
    } else {
        Log::error("can't open config.json");
    }
    return 0;
}