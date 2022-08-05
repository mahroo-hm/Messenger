#ifndef USER_SERVER_H
#define USER_SERVER_H

#include <thread>
#include <unistd.h>
using namespace std;

class UserServer
{
public:
    int id;
    thread* clientThread;
    int clientSocket;
    string name;
    
    UserServer(int _id, int _clientSocket);
    ~UserServer();
};

#endif