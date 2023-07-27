#include "chatserver.h"
#include "chatservice.h"
#include <iostream>
#include <signal.h>
using namespace std;

//Ctrl+C （SIGNINT信号）的回调函数
void resetHandle(int)
{
    ChatService::instance()->reset();
    exit(0);
}


int main(int argc,char* argv[])
{
    signal(SIGINT,resetHandle);
    
    EventLoop loop;
    char* ip = nullptr;
    int port = 8080;
    if(argc>=3)
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }
    
    InetAddress addr(ip==nullptr?"192.168.220.130":ip, port);
    ChatServer server(&loop,addr,"ChatServer");

    server.start();
    loop.loop();
    return 0;
}