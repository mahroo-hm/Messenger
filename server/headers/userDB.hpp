#ifndef USERDB_H
#define USERDB_H

#include <iostream>
#include <user.hpp>
#include <mutex>
#include <helper.hpp>
#include <vector>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 

using namespace std;

class UserDB
{
public:
    vector<string> dbVectOutput;
    mutex write_mtx;
    string wrapper = " #";
    sqlite3 *db;
    char *zErrMsg = 0;
    sqlite3_stmt* stmt;
    string sql;
    
    UserDB();
    void createTables();
    void getSelectedCells(sqlite3_stmt* stmt, int colCount);
    void addUser(User* user);
    vector<User*>* getUsers();
    void editUserName(User* user, string newName);
    void addFriend(User* user, User* userNewFriend);
    vector<string> getUserFriends(User* user);
    void addToBlocklist(User* user, User* userBlock);
    void removeFromBlocklist(User* user, User* userBlock);
    vector<string> getUserBlocklist(User* user);
    vector<string> getBlockers(User* user);
    void addUserToGroup(User* user, string groupName);
    vector<string> getUserGroups(string username);
    vector<string> getGroups();
    void removeUserFromGP(User* user, string groupName);
    static bool lexCompare(string a, string b);
    void updateOnlineHistory(string user1orGp, string sender, string msg, bool pv, string user2orNULL = "");
    void updateOfflineHistory(string offlineUser, string sender, string msg, bool pv, string senderOrGp);
    string getOnlineHistory(string user1orGp, bool pv, string user2orNULL = "");
    string getOfflineHistory(string user1orGp);
    vector<string> getGroupMembers(string groupName);
    void handleNewMessages(string user, bool pv);
    string addTimeToMsg(string sender, string msg);

};

#endif