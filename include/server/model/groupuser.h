#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.h"


//groupuser数据的映射对象
class GroupUser:public User
{
public:
    void setRole(string role){_role = role;}
    string getRole(){return _role;}
private:
    string _role;
};


#endif//GROUPUSER_H