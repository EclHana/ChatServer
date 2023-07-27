#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;


//聊天服务器主类
class ChatServer
{
public:
	//初始化聊天服务器对象
	ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);
	
	//启动服务
	void start();

private:
	//连接回调函数
	void onConnect(const TcpConnectionPtr&conn);
	//消息读写回调函数
	void onMessage(const TcpConnectionPtr&conn, Buffer*buffer, Timestamp time);
	
	//组合的muduo库，实现服务器功能的对象
	TcpServer _server;
	//指向事件循环epoll的指针
	EventLoop *_loop;
};

#endif//CHATSERVER_H