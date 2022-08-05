#include "userDB.hpp"

UserDB::UserDB()
{
    this->createTables();
}

void UserDB::createTables()
{
    sqlite3_open("database.db", &db);

    //Users table
    sql = "CREATE TABLE IF NOT EXISTS USERS("
        "Username       TEXT     NOT NULL,"
        "Name           TEXT     ,"
        "Password       TEXT     NOT NULL,"
        "JoinDT       TEXT     NOT NULL);";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    //Relation table 
    sql = "CREATE TABLE IF NOT EXISTS RELATION("
        "FirstUsername          TEXT        NOT NULL,"
        "SecondUsername         TEXT        NOT NULL,"
        "Status          TEXT        NOT NULL);";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    //Group table
    sql = "CREATE TABLE IF NOT EXISTS GROUPS("
        "Username       TEXT     NOT NULL,"
        "GP       TEXT     NOT NULL);";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    //History table
    sql = "CREATE TABLE IF NOT EXISTS HISTORY("
        "Type         TEXT        NOT NULL,"
        "Col1         TEXT        NOT NULL,"
        "History      TEXT);";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    //Offline hisotry table
    sql = "CREATE TABLE IF NOT EXISTS OFFHISTORY("
        "Type         TEXT        NOT NULL,"
        "OffUser         TEXT        NOT NULL,"
        "FromWho         TEXT        NOT NULL,"
        "History      TEXT);";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_close(db);
}

void UserDB::addUser(User* user)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);

    time_t t = time(nullptr);
    tm* now = std::localtime(&t);
    string time = to_string(now->tm_hour) + ':' + to_string(now->tm_min) + ':' +  to_string(now->tm_sec);
    time += " - " + to_string(now->tm_year + 1900) + '/' + to_string(now->tm_mon + 1) + '/' +  to_string(now->tm_mday) + "\n";
    
    sql = "INSERT INTO USERS (Username, Name, Password, JoinDT) VALUES (?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user->name.c_str(), user->name.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user->password.c_str(), user->password.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, time.c_str(), time.length(), SQLITE_STATIC);
    sqlite3_step(stmt);

    sqlite3_close(db);
}

void UserDB::getSelectedCells(sqlite3_stmt* stmt, int colCount)
{
    string row;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        row = "";
        for (int i = 0; i < colCount ; i++){
            row += reinterpret_cast< char const* >(sqlite3_column_text(stmt, i));
            row += " #";
        }
        if (row != ""){
            row.pop_back();
            row.pop_back();
        }
        dbVectOutput.push_back(row);
    }
}

vector<User*>* UserDB::getUsers()
{
    vector<User*>* usersVect = new vector<User*>();
    dbVectOutput.clear();
    sqlite3_open("database.db", &db);
    
    sql = "SELECT (Username, Name, Password) FROM USERS";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    getSelectedCells(stmt, 3);
    
    vector<string>* messages;
    for (string usr : dbVectOutput){
        messages = splitMessage(usr, wrapper, 0);
        usersVect->push_back(new User(messages->at(0), messages->at(2), messages->at(1)));
    }

    sqlite3_close(db);
    return usersVect;
}

void UserDB::editUserName(User* user, string newName)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);

    sql = "UPDATE USERS SET Name = ? WHERE Username = ?";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, newName.c_str(), newName.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_close(db);
}

void UserDB::addFriend(User* user, User* userNewFriend)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);

    //Delete duplicates
    sql = "DELETE FROM RELATION WHERE FirstUsername = ? AND SecondUsername = ? AND Status = 'FRIEND'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userNewFriend->username.c_str(), userNewFriend->username.length(), SQLITE_STATIC);
    sqlite3_step(stmt);
    
    sql = "INSERT INTO RELATION (FirstUsername, SecondUsername, Status) VALUES (?, ?, 'FRIEND');";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userNewFriend->username.c_str(), userNewFriend->username.length(), SQLITE_STATIC);
    sqlite3_step(stmt);

    sqlite3_close(db);
}

vector<string> UserDB::getUserFriends(User* user)
{
    vector<string> friendsVect;
    dbVectOutput.clear();
    sqlite3_open("database.db", &db);

    sql = "SELECT SecondUsername FROM RELATION WHERE FirstUsername = '" + user->username + "' AND STATUS = 'FRIEND'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    getSelectedCells(stmt, 1);
    
    for (int i = 0; i < dbVectOutput.size(); i++)
        friendsVect.push_back(dbVectOutput.at(i));

    sqlite3_close(db);
    return friendsVect;
}

void UserDB::addToBlocklist(User* user, User* userBlock)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);

    //Delete duplicates
    sql = "DELETE FROM RELATION WHERE FirstUsername = ? AND SecondUsername = ? AND Status = 'BLOCK'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userBlock->username.c_str(), userBlock->username.length(), SQLITE_STATIC);
    sqlite3_step(stmt);
    
    sql = "INSERT INTO RELATION (FirstUsername, SecondUsername, Status) VALUES (?, ?, 'BLOCK');";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userBlock->username.c_str(), userBlock->username.length(), SQLITE_STATIC);
    sqlite3_step(stmt);

    sqlite3_close(db);
}

vector<string> UserDB::getUserBlocklist(User* user)
{
    vector<string> blockVect;
    sqlite3_open("database.db", &db);

    sql = "SELECT SecondUsername FROM RELATION WHERE FirstUsername = '" + user->username + "' AND STATUS = 'BLOCK'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        blockVect.push_back(reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0)));
    }

    sqlite3_close(db);
    return blockVect;
}

vector<string> UserDB::getBlockers(User* user)
{
    vector<string> blockVect;
    sqlite3_open("database.db", &db);

    sql = "SELECT FirstUsername FROM RELATION WHERE SecondUsername = '" + user->username + "' AND STATUS = 'BLOCK'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        blockVect.push_back(reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0)));
    }

    sqlite3_close(db);
    return blockVect;
}

void UserDB::removeFromBlocklist(User* user, User* userBlock)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);

    sql = "DELETE FROM RELATION WHERE FirstUsername = ? AND SecondUsername = ? AND Status = 'BLOCK'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userBlock->username.c_str(), userBlock->username.length(), SQLITE_STATIC);
    sqlite3_step(stmt);

    sqlite3_close(db);
}

void UserDB::addUserToGroup(User* user, string groupName)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);

    //Delete duplicates
    sql = "DELETE FROM GROUPS WHERE Username = ? AND GP = ?";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, groupName.c_str(), groupName.length(), SQLITE_STATIC);
    sqlite3_step(stmt);
    
    sql = "INSERT INTO GROUPS (Username, GP) VALUES (?, ?);";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, groupName.c_str(), groupName.length(), SQLITE_STATIC);
    sqlite3_step(stmt);

    sqlite3_close(db);
}

vector<string> UserDB::getUserGroups(string username)
{
    vector<string> groupVect;
    sqlite3_open("database.db", &db);

    sql = "SELECT Gp FROM GROUPS WHERE Username = '" + username + "'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        groupVect.push_back(reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0)));
    }

    sqlite3_close(db);
    return groupVect;
}

vector<string> UserDB::getGroupMembers(string groupName)
{
    vector<string> userVect;
    sqlite3_open("database.db", &db);

    sql = "SELECT Username FROM GROUPS WHERE Gp = '" + groupName + "'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        userVect.push_back(reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0)));
    }

    sqlite3_close(db);
    return userVect;
}

vector<string> UserDB::getGroups()
{  
    vector<string> groupVect;
    sqlite3_open("database.db", &db);

    sql = "SELECT DISTINCT Gp FROM GROUPS";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        groupVect.push_back(reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0)));
    }
    
    sqlite3_close(db);
    return groupVect;
}

void UserDB::removeUserFromGP(User* user, string groupName)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);

    sql = "DELETE FROM GROUPS WHERE Username = ? AND GP = ?";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user->username.c_str(), user->username.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, groupName.c_str(), groupName.length(), SQLITE_STATIC);
    sqlite3_step(stmt);

    sqlite3_close(db);
}

bool UserDB::lexCompare(string a, string b)
{
    if (a.compare(0, b.size(), b) == 0 || b.compare(0, a.size(), a) == 0)
        return a.size() > b.size();
    else
        return a < b;
}

string UserDB::addTimeToMsg(string sender, string msg)
{
    string newMsg = sender + " : " + msg + "     ";
    time_t t = time(nullptr);
    tm* now = std::localtime(&t);
    string time = to_string(now->tm_hour) + ':' + to_string(now->tm_min);
    time += " (" + to_string(now->tm_year + 1900) + '/' + to_string(now->tm_mon + 1) + '/' +  to_string(now->tm_mday) + ')' + "\n";
    newMsg += time;
    return newMsg;
}

void UserDB::updateOnlineHistory(string userOrGp, string sender, string msg, bool pv, string user2orNULL)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);
    string his;
    string type = (pv) ? "PV" : "GP";
    string newMsg = addTimeToMsg(sender, msg);
    if (user2orNULL != "" && pv){
        vector<string> v = {userOrGp, user2orNULL};
        sort(v.begin(), v.end(), lexCompare);
        userOrGp = v[0] + "//" + v[1];
    }
    

    sql = "SELECT History FROM HISTORY WHERE Type = ? AND Col1 = ?";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, type.c_str(), type.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userOrGp.c_str(), userOrGp.length(), SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        his += reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0));
    
    sql = "DELETE FROM HISTORY WHERE Type = ? AND Col1 = ?";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, type.c_str(), type.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userOrGp.c_str(), userOrGp.length(), SQLITE_STATIC);
    sqlite3_step(stmt);

    his += newMsg;
    sql = "INSERT INTO HISTORY (Type, Col1,History) VALUES (?, ?, ?);";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, type.c_str(), type.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userOrGp.c_str(), userOrGp.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, his.c_str(), his.length(), SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_close(db);
}

string UserDB::getOnlineHistory(string userOrGp, bool pv, string user2orNULL)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);
    string historyStr;
    string type = (pv) ? "PV" : "GP";
    if (user2orNULL != "" && pv){
        vector<string> v = {userOrGp, user2orNULL};
        sort(v.begin(), v.end(), lexCompare);
        userOrGp = v[0] + "//" + v[1];
    }

    sql = "SELECT History FROM HISTORY WHERE Type = ? AND Col1 = ?";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, type.c_str(), type.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, userOrGp.c_str(), userOrGp.length(), SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        historyStr += reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0));
    }

    sqlite3_close(db);
    return historyStr;
}

void UserDB::updateOfflineHistory(string offlineUser, string sender, string msg, bool pv, string senderOrGp)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);
    string his;
    string type = (pv) ? "PV" : "GP";
    string newMsg = addTimeToMsg(sender, msg);


    sql = "SELECT History FROM OFFHISTORY WHERE Type = ? AND OffUser = ?";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, type.c_str(), type.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, offlineUser.c_str(), offlineUser.length(), SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        his += reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0));
    
    sql = "DELETE FROM OFFHISTORY WHERE Type = ? AND OffUser = ?";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, type.c_str(), type.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, offlineUser.c_str(), offlineUser.length(), SQLITE_STATIC);
    sqlite3_step(stmt);

    his += newMsg;
    sql = "INSERT INTO OFFHISTORY (Type,OffUser,FromWho,History) VALUES (?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, type.c_str(), type.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, offlineUser.c_str(), offlineUser.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, senderOrGp.c_str(), senderOrGp.length(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, his.c_str(), his.length(), SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_close(db);
}

string UserDB::getOfflineHistory(string user)
{
    lock_guard<mutex> guard(write_mtx);
    sqlite3_open("database.db", &db);
    string historyStr;

    sql = "SELECT FromWho, History FROM OFFHISTORY WHERE Type = 'PV' AND OffUser = '" + user + "'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        historyStr += "  From ";
        historyStr += reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0));
        historyStr += ":\n";
        historyStr += reinterpret_cast< char const* >(sqlite3_column_text(stmt, 1));
        historyStr += "\n";
    }

    sql = "SELECT FromWho, History FROM OFFHISTORY WHERE Type = 'GP' AND OffUser = '" + user + "'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        historyStr += "  Group ";
        historyStr += reinterpret_cast< char const* >(sqlite3_column_text(stmt, 0));
        historyStr += ":\n";
        historyStr += reinterpret_cast< char const* >(sqlite3_column_text(stmt, 1));
        historyStr += "\n";
    }

    sql = "DELETE FROM OFFHISTORY WHERE OffUser = '" + user + "'";
    sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_step(stmt);

    if (historyStr != "")
        historyStr = "> New messages:\n" + historyStr;
    else
        historyStr = "> No new messages\n";

    sqlite3_close(db);
    return historyStr;
}