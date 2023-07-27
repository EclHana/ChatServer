#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include "json.h"
#include "usermodel.h"
#include <mutex>
#include "offlinemsgmodel.h"
#include "friendmodel.h"
#include "groupmodel.h"
#include "redis/redis.h"


using json = nlohmann::json;
using namespace std;
using namespace muduo;
using namespace muduo::net;


//处理消息事件回调方法类型
using MsgHandle = function<void(const TcpConnectionPtr&, json&, Timestamp)>;
//chat服务，不是server服务器

//聊天服务器业务类，也就是服务类

class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    //默认handle
    void defaultHandle(const TcpConnectionPtr&, json&, Timestamp);
    //处理登录业务
    void login(const TcpConnectionPtr&, json&, Timestamp);
    //处理注册业务
    void reg(const TcpConnectionPtr&, json&, Timestamp);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr&, json&, Timestamp);
    //添加好友业务
    void addFriend(const TcpConnectionPtr&, json&, Timestamp);
    //创建群聊业务
    void createGroup(const TcpConnectionPtr&, json&, Timestamp);
    //加入群聊业务
    void addGroup(const TcpConnectionPtr&, json&, Timestamp);
    //群聊天业务
    void groupChat(const TcpConnectionPtr&, json&, Timestamp);
    //退出登录业务
    void loginOut(const TcpConnectionPtr&, json&, Timestamp);

    //从redis消息队列接收到订阅的信息之后，服务器的回调函数
    void handleRedisSubscribeMessage(int userid,string msg);

    //获取消息对应的handle
    MsgHandle getHandle(int msgid);

    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr&);

    //服务器异常，业务重置方法
    void reset();


private:
    ChatService();//单例模式，构造私有
    //存储消息id和其对应的业务处理方法
    unordered_map<int,MsgHandle> _msgHandleMap;

    //数据操作类对象
    UserModel _userModel;

    //离线消息处理对象
    OfflineMsgModel _offlineMsgModel;

    //friend操作对象
    FriendModel _friendModel;

    //group操作对象
    GroupModel _groupModel;

    //存储在线用户的通信连接,注意线程安全
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //定义互斥锁保证_userConnMap的线程安全
    mutex _connMtx;

    //Redis对象
    Redis _redis;

};



#endif//CHATSERVICE_H