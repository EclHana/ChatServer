#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.h"

//只是针对user表数据库的数据操作类，与业务无关
class UserModel
{         
public:
    //user表的增加方法
    bool insert(User &user);
    //根据用户id查询用户信息
    User query(int id);
    //更新用户的状态信息
    bool updateState(User &user);

    //重置用户状态信息
    void resetState();

private:

};




#endif//USERMODEL_H