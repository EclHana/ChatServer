#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.h"
#include <string>
#include <vector>
using namespace std;

class GroupModel
{
public:
    //创建群组
    bool createGroup(Group& group);
    //加入群聊
    void addGroup(int userid,int groupid,string role);
    //查询用户群组信息
    vector<Group> queryGroups(int userid);
    //根据指定群id来查询除userid以外的所有群成员，用于给他人群发消息
    vector<int> queryGroupUsers(int userid,int groupid);
private:
};



#endif//GROUPMODEL_H