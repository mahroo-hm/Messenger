#include "user_server.hpp"

UserServer::UserServer(int _id, int _clientSocket)
{
    this->id = _id;
    this->clientSocket = _clientSocket;
    this->name = "Anonymous";
}

UserServer::~UserServer()
{
    if (clientThread)
    {
        if (clientThread->joinable())
        {
            clientThread->detach();
            delete clientThread;
        }
        clientThread = 0;
    }
    if (clientSocket)
        close(clientSocket);
}