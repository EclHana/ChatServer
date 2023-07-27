#include "chatservice.h"
#include "public.h"
#include <muduo/base/Logging.h>
#include <string>
#include <iostream>
#include "user.h"

using namespace muduo;
using namespace std;
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}


//注册消息以及对应的回调操作
ChatService::ChatService()
{         
    _msgHandleMap.insert({DEFAULT_MSG,std::bind(&ChatService::defaultHandle,this,_1,_2,_3)});
    _msgHandleMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    _msgHandleMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
    _msgHandleMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msgHandleMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msgHandleMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    _msgHandleMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _msgHandleMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});
    _msgHandleMap.insert({LOGINOUT_MSG,std::bind(&ChatService::loginOut,this,_1,_2,_3)});

    if(_redis.connect())
    {
        //设置上报信息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }

}

//ORM object relation mapping，业务层操作的都是对象
//处理登录业务，业务代码不要直接操作数据库，业务与数据解耦
void ChatService::login(const TcpConnectionPtr&conn, json&js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];
    
    User user = _userModel.query(id);
    if(user.getId()!=-1 && user.getPwd()==pwd)//id返回-1表示查无此人
    {
        if(user.getState()=="online")
        {
            //用户已经在线，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登录，无法重新登录";
            conn->send(response.dump());
        }
        else
        {
            //登录成功,更新用户状态信息 state
            user.setState("online");
            _userModel.updateState(user);

            {
                lock_guard<mutex> lock(_connMtx);//小粒度的锁
                //记录用户连接信息->>要考虑线程安全问题
                _userConnMap.insert({id,conn});
            }
            
            //id用户登录成功后，向redis订阅channel,其他服务器如果需要向id用户发送消息，publish即可
            _redis.subscribe(id);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            //查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                _offlineMsgModel.remove(id);//读取用户的离线之后，把该用户的所有离线消息删除掉
            }
            //查询用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty())
            {
                vector<string> vec2;
                for(User &u:userVec)
                {
                    json js;
                    js["id"] = u.getId();
                    js["name"] = u.getName();
                    js["state"] = u.getState();
                    vec2.emplace_back(js.dump());
                }
                response["friends"] = vec2;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        //用户不存在或者密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或者密码错误";
        conn->send(response.dump());
    }

}

//处理注册业务
void ChatService::reg(const TcpConnectionPtr&conn, json&js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);//操作数据库的地方

    if(state)//回传状态消息
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}
//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr&conn, json&js, Timestamp time)
{
    int toId = js["toid"].get<int>();
    
    {
        lock_guard<mutex> lock(_connMtx);
        if(_userConnMap.count(toId))//保证conn的线程安全操作
        {
            //toId的用户在线，转发消息
            _userConnMap[toId]->send(js.dump());
        }
    }
    //去数据库查一下，看看是不在线还是不在当前服务器上
    User user = _userModel.query(toId);
    if(user.getState()=="online")
    {
        //用户在线，但是在别的服务器上,向redis发布消息
        _redis.publish(toId,js.dump());
    }
    else
    {
        //toId的用户不在线，存储离线消息
        _offlineMsgModel.insert(toId,js.dump());
    }
}
//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr&conn, json&js, Timestamp)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    //存储好友信息
    _friendModel.insert(userid,friendid);
}

//创建群聊业务
void ChatService::createGroup(const TcpConnectionPtr&conn, json&js, Timestamp)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1,name,desc);
    if(_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid,group.getId(),"creator");
    }
}
//加入群聊业务
void ChatService::addGroup(const TcpConnectionPtr&conn, json&js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid,groupid,"normal");
}
//群聊天业务
void ChatService::groupChat(const TcpConnectionPtr&conn, json&js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid,groupid);

    lock_guard<mutex> lock(_connMtx);//保证conn的线程安全操作
    for(int id:useridVec)
    {
        if(_userConnMap.count(id))
        {
            //id用户在线，转发消息
            _userConnMap[id]->send(js.dump());
        }
        else
        {
            User user = _userModel.query(id);
            if(user.getState()=="online")
            {
                //用户在线，但是在别的服务器上,向redis发布消息
                _redis.publish(id,js.dump());
            }
            else
            {
                //toId的用户不在线，存储离线消息
                _offlineMsgModel.insert(id,js.dump());
            }
        }
    }
}

//退出登录业务
void ChatService::loginOut(const TcpConnectionPtr&conn, json&js, Timestamp)
{
   int userid = js["id"].get<int>();

   {
        lock_guard<std::mutex> lock(_connMtx);
        auto iter = _userConnMap.find(userid);
        if(iter!=_userConnMap.end())
        {
            _userConnMap.erase(userid);
        }
   }
    //正常退出需要从redis中取消订阅通道
    _redis.unsubscribe(userid);

   //更新用户状态信息
   User user(userid,"","","offline");
   _userModel.updateState(user);
}

void ChatService::defaultHandle(const TcpConnectionPtr&, json&, Timestamp)
{
    //不做任何事情
}

MsgHandle ChatService::getHandle(int msgid)
{
    if(_msgHandleMap.count(msgid)==0)
    {
        LOG_ERROR << "msgid : "<<msgid<<" can not find handler!";
        //返回一个默认的处理器
        return _msgHandleMap[DEFAULT_MSG];
    }
    else
    {
        return _msgHandleMap[msgid];
    }
          
}

//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr&conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMtx);
        for(auto it = _userConnMap.begin();it!=_userConnMap.end();++it)
        {
            if(it->second==conn)
            {
                //从map表中删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    //异常退出，需要在redis中取消订阅
    _redis.unsubscribe(user.getId());

    //更新用户状态信息
    if(user.getId()!=-1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
    
}

//重置方法
void ChatService::reset()
{
    //把online用户的状态设置成offline
    _userModel.resetState();
}

//从redis消息队列接收到订阅的信息之后，服务器的回调函数要么转发消息，要么储存离线消息
void ChatService::handleRedisSubscribeMessage(int userid,string msg)
{

    {
        std::unique_lock<std::mutex> lock(_connMtx);
        if(_userConnMap.count(userid))
        {
            _userConnMap[userid]->send(msg);
            return;
        }
    }
    
    _offlineMsgModel.insert(userid,msg);
}
