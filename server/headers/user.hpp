#ifndef USER_H
#define USER_H

#include <user_server.hpp>
#include <map>
#include <set>
using namespace std;

class User
{
public:
    User(string username, string password, string name, UserServer* _userServer = 0);
    void setName(string name);

    UserServer* userServer;
    string name;
    string username;
    string password;
};

#endif