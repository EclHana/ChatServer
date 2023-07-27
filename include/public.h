#ifndef PUBLIC_H
#define PUBLIC_H
//server和client的公共文件

enum EnMsgType
{
    DEFAULT_MSG = 0,    //默认消息
    LOGIN_MSG,      //登录消息
    LOGIN_MSG_ACK,  //登录响应消息
    REG_MSG,        //注册消息
    REG_MSG_ACK,    // 注册响应消息
    ONE_CHAT_MSG,   //一对一聊天消息
    ADD_FRIEND_MSG, //添加好友消息

    CREATE_GROUP_MSG,//创建群
    ADD_GROUP_MSG,  //加群消息
    GROUP_CHAT_MSG, //群聊天
    LOGINOUT_MSG,   //退出
};



#endif//PUBLIC_H