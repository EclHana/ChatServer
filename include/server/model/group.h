#ifndef GROUP_H
#define GROUP_H
#include <vector>
#include <string>
#include "groupuser.h"


//Group数据的映射对象
class Group
{
public:
    Group(int id = -1,string name = "",string desc = "")
        :_id(id),_name(name),_desc(desc)
    {}
    void setId(int id){_id = id;}
	void setName(string name){_name = name;}
	void setDesc(string desc){_desc = desc;}

	int getId(){return _id;}
	string getName(){return _name;}
	string getDesc(){return _desc;}
    vector<GroupUser>& getUsers(){return users;}
private: 
    int _id;
    string _name;
    string _desc;
    vector<GroupUser> users;
};



#endif//GROUP_H