#ifndef SERVER_H
#define SERVER_H

#include "iostream"
#include <mutex>
#include "user_server.hpp"
#include <map>
#include "userDB.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include "helper.hpp"
#include "user.hpp"

using namespace std;

class Server
{
public:
    int uniqueID;
    int MAXLEN;
    int serverSocket;
    bool newUser = false;
    string location = "Home";
    UserDB* userDB;
    mutex printMTX, clientsMTX;
    map<string, User*> users;
    map<int, UserServer*> clients;
    
    Server();
    void startListening();
    void startAccepting();
    void closeConnection();
    void endConnection(int id);
    static void handleClient(Server* server, UserServer* user_server);
    void multiPrint(string str);
    void addClient(UserServer* client);
    void sendMessage(int socket, string message);
    void handleUserReq(User* user, string message);
    int getUser(UserServer* user_server, string username, string password);
    void addNewUser(UserServer* user_server, string username, string password, string name);
    void changeName(User* user);
    void sendPv(User* sender, User* receiver, string msg);
    void sendGp(User* sender, string groupName, string msg);
    void openBlocklist(User* user);
    void showFriendsList(User* user);
    void showGroupsList(User* user);
    void showChatHistory(User* user, string userOrGp, bool pv);
    void showBlockList(User* user);
    void showOfflineHistory(string username, UserServer* userServer);
    void showGroupMembers(User* sender, string groupName, bool pv);
    bool loginClient(UserServer* user_server);
    void checkUser(string name);
    void checkGp(string user, string groupName);
    void checkIfIsBlocked(User* sender, string recvUsername);
    void getUsersFromDB();
    void openMainMenu(User* user);
    void openPrivateChatSession(User* user1, User* user2);
    void openGroupChatSession(User* sender, string groupName);
    int getOnlineClientsCount();
    void deleteUsers();
    ~Server();
};

#endif