#include "server.hpp"
#include "signal.h"

Server* server = 0;

void exitChat(int sigNum);

int main()
{
    server = new Server();
    signal(SIGINT, exitChat);
    server->getUsersFromDB();
    server->startListening();
    server->startAccepting();
    exitChat(0);
}

void exitChat(int sigNum)
{
    if (server)
        delete server;
    cout<<"***BYE***"<<endl;
    exit(sigNum);
}