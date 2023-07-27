#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "user.h"
#include <vector>
using namespace std;


//维护好友数据库信息的操作接口方法
class FriendModel
{
public:
    //添加好友关系
    void insert(int userid,int friendid);
    //返回用户的好友列表信息  两个表的联合查询
    vector<User> query(int userid);
private:
};

#endif//FRIENDMODEL_H