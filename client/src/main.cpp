#include "client.hpp"
#include "signal.h"

void exitChat(int sig_num);

Client* client = 0;

int main()
{
    client = new Client();
    signal(SIGINT, exitChat);
    client->startConnecting();
    client->startCommunicating();
    exitChat(0);
}

void exitChat(int sig_num)
{
    if (client)
        delete client;
    cout<<"You have signed out."<<endl;
    exit(sig_num);
}