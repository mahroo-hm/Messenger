#include "user.hpp"
#include "userDB.hpp"

User::User(string _username, string _password, string _name, UserServer* _userServer)
{
    this->userServer = _userServer;
    this->username = _username;
    this->password = _password;
    setName(_name);
}

void User::setName(string name){
    this->name = name;
}