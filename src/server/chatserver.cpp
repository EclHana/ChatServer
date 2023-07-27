#include "chatserver.h"
#include "chatservice.h"
#include <string>
#include "json.h"
#include <iostream>



using json = nlohmann::json;
using namespace std::placeholders;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg)
                    :_server(loop,listenAddr,nameArg),_loop(loop)
{
	_server.setConnectionCallback(std::bind(&ChatServer::onConnect,this,_1));
	_server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
	_server.setThreadNum(4);
}

void ChatServer::start()
{
	std::cout<<"<<=========聊天服务器启动=========>>"<<std::endl;
	_server.start();
}


//连接回调函数
void ChatServer::onConnect(const TcpConnectionPtr&conn)
{
	//客户端断开连接
	if(!conn->connected())
	{
		ChatService::instance()->clientCloseException(conn);
		conn->shutdown();
	}
}
//消息读写回调函数
void ChatServer::onMessage(const TcpConnectionPtr&conn, Buffer*buffer, Timestamp time)
{
	std::cout<<"On Message call"<<std::endl;
	string buf = buffer->retrieveAllAsString();
	json js = json::parse(buf);//反序列化
	//通过给js["msgid"]绑定一个回调操作，获取一个业务handle（conn, buffer, time） 
	//从而完全解耦网络模块代码和业务模块代码
	auto handler = ChatService::instance()->getHandle(js["msgid"].get<int>());
	//回调消息绑定好的事件处理器，来执行相应的业务处理
	handler(conn, js, time);
}