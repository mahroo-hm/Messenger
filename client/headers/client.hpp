#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <netinet/in.h>

using namespace std;

class Client
{
public:
    thread* sendThread;
    thread* recvThread;
    int MAXLEN;
    bool loggedIn;
    bool exited;
    char* name;
    char* username;
    char* password;
    mutex printMTX;
    int clientSocket;
    string location = "";

    Client();
    void startConnecting();
    void startCommunicating();
    void closeConnection();
    void login();
    void setLocation();
    static void sendHandler(Client* client);
    static void recvHandler(Client* client);
    void multiPrint(string message, bool you = true);
    ~Client();
};

#endif