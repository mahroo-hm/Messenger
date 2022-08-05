#include "server.hpp"

Server::Server()
{
    uniqueID = 0;
    MAXLEN = 150;
    userDB = new UserDB();
    clients.clear();
}

void Server::startListening()
{
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(-1);
	}

    struct sockaddr_in server;
    server.sin_family = AF_INET;
	server.sin_port = htons(10000);
	server.sin_addr.s_addr = INADDR_ANY;
    if ((::bind(serverSocket, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) == -1)
	{
		perror("bind");
		exit(-1);
	}

    if ((listen(serverSocket, 5)) == -1)
	{
		perror("listen");
		exit(-1);
	}
    multiPrint("*** Chat Room start listening ***");   
}

void Server::startAccepting()
{
    struct sockaddr_in client;
    int clientSocket;
    unsigned int len = sizeof(sockaddr_in);

    while (true)
    {
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&client, &len)) == -1)
        {
            perror("accept: ");
			exit(-1);
        } 
        uniqueID++;
        UserServer* userServer = new UserServer(uniqueID, clientSocket);
        thread* clientThread = new thread(handleClient, this, userServer);
        userServer->clientThread = clientThread;
        addClient(userServer);
    }
}

void Server::addClient(UserServer* userServer)
{
    lock_guard<mutex> guard(clientsMTX);
	clients[userServer->id] = userServer;
}

void Server::endConnection(int id)
{
    lock_guard<mutex> guard(clientsMTX);
    if (clients[id])
        delete clients[id];
}

void Server::closeConnection()
{
    close(serverSocket);
}

void Server::multiPrint(string str)
{	
	lock_guard<mutex> guard(printMTX);
	cout << str << endl;
}

void Server::sendMessage(int clientSocket, string message)
{
    send(clientSocket, &message[0], MAXLEN, 0);
}

void Server::addNewUser(UserServer* userServer, string username, string password, string name)
{
    users[username] = new User(username, password, name, userServer);
    userDB->addUser(users[username]);

}

int Server::getUser(UserServer* userServer, string username, string password)
{
    if (users.find(username) != users.end())
    {
        if (users[username]->password == password){
            users[username]->userServer = userServer;
            return 1;
        }
        return 2;
    }
    return 0;
}

bool Server::loginClient(UserServer* userServer)
{
    while(true)
    {
        char name[MAXLEN], password[MAXLEN], password2[MAXLEN], username[MAXLEN], signMode[MAXLEN];
        
        int bytes_received = recv(userServer->clientSocket, signMode, sizeof(signMode), 0);
        if(bytes_received <= 0)
            return false;

        if (signMode[0] == '2')
        {
            bytes_received = recv(userServer->clientSocket, name, sizeof(name), 0);
            if(bytes_received <= 0)
                return false;
        }

        bytes_received = recv(userServer->clientSocket, username, sizeof(username), 0);
        if(bytes_received <= 0)
            return false;

        bytes_received = recv(userServer->clientSocket, password, sizeof(password), 0);
        if(bytes_received <= 0)
            return false;
        
        if (signMode[0] == '2'){
            bytes_received = recv(userServer->clientSocket, password2, sizeof(password2), 0);
            if(bytes_received <= 0)
                return false;
        }

        string welcomeMsg;
        if (signMode[0] == '1')
        {
            if (getUser(userServer, username, password) != 1)
            {
                string errorMsg = "> Invalid username or password, try again.";
                sendMessage(userServer->clientSocket, errorMsg);
                multiPrint(errorMsg);
                continue;
            }
            else
                welcomeMsg = "> Welcome Back " + string(username) + "!";
        }
        else
            if (string(password) != string(password2))
            {
                string errorMsg = "> Passwords do not match, try again.";
                sendMessage(userServer->clientSocket, errorMsg);
                multiPrint(errorMsg);
                continue;
            } 
            else if (getUser(userServer, username, password) != 0)
            {
                string errorMsg = "> This username is already taken, try again.";
                sendMessage(userServer->clientSocket, errorMsg);
                multiPrint(errorMsg);
                continue;
            }
            else{
                addNewUser(userServer, username, password, name);
                welcomeMsg = "> Welcome " + string(username) + "! Thank you for joining us!";
                newUser = true;
            }


        userServer->name = username;
        sendMessage(userServer->clientSocket, welcomeMsg);
        multiPrint(welcomeMsg);
        return true;
    }
}

void Server::handleClient(Server* server, UserServer* userServer)
{
    if (server->loginClient(userServer))
    {   
        if (server->newUser == false)
            server->showOfflineHistory(userServer->name, userServer);
        server->newUser = false;
        char message[server->MAXLEN];
        int bytes_received;
        while(true)
        {
            server->sendMessage(userServer->clientSocket, "#loc> Home| "); 
            server->sendMessage(userServer->clientSocket, "");
            bytes_received = recv(userServer->clientSocket, message, sizeof(message), 0);
            if(bytes_received <= 0)
                break; 
            server->multiPrint(string(userServer->name) + " : " + string(message));
            server->handleUserReq(server->users[userServer->name], message);
        }
        server->users[userServer->name]->userServer = 0;
    }
    server->endConnection(userServer->id);
}

void Server::sendPv(User* sender, User* receiver, string msg)
{   
    if (!receiver->userServer)
        userDB->updateOfflineHistory(receiver->username, sender->username, msg, true, sender->username);
    else
        sendMessage(receiver->userServer->clientSocket, "- " + sender->name + " : " + msg);
    userDB->addFriend(sender, receiver);
    userDB->addFriend(receiver, sender);
    userDB->updateOnlineHistory(sender->username, sender->username, msg, true, receiver->username);

    
}

void Server::sendGp(User* sender, string groupName, string msg)
{
    vector<string> members = userDB->getGroupMembers(groupName);
    for (string mem : members)
    {
        if (!users[mem]->userServer)
            userDB->updateOfflineHistory(users[mem]->username, sender->username, msg, false, groupName);
        else if (users[mem]->username != sender->username)
            sendMessage(users[mem]->userServer->clientSocket, groupName + "-" + sender->name + " : " + msg);
    }
    userDB->updateOnlineHistory(groupName, sender->username, msg, false);
}

void Server::checkUser(string username)
{
    if (users.find(username) == users.end())
        throw "This user doesn't exist.";
}

void Server::checkIfIsBlocked(User* sender, string recvUsername)
{
    vector<string> blocks = userDB->getUserBlocklist(sender);
    for (auto b : blocks)
        if (b == recvUsername)
            throw "You have blocked this user, unblock them to chat.";
    blocks = userDB->getBlockers(sender);
    for (auto b : blocks)
        if (b == recvUsername)
            throw "This user has blocked you :)";
}

void Server::changeName(User* user)
{
    char newName[this->MAXLEN];
    sendMessage(user->userServer->clientSocket, "#loc> Menu|ChangeName| ");
    sendMessage(user->userServer->clientSocket, "> New name: ");
    int bytes_received = recv(user->userServer->clientSocket, newName, sizeof(newName), 0);
    userDB->editUserName(user, string(newName));
    user->setName(newName);
    string result = "> Your name was change to [" + string(newName) + "].";
    sendMessage(user->userServer->clientSocket, result);
    return;
}

void Server::openMainMenu(User* user)
{
    char message[this->MAXLEN];
    int bytes_received;
    while (true)
    {
        sendMessage(user->userServer->clientSocket, "#loc> Menu| ");
        string menu = "> Main Menu\n  [1] Change name\n  [2] Friend list\n"
        "  [3] Block list\n  [4] Groups\n";
        sendMessage(user->userServer->clientSocket, menu);
        bytes_received = recv(user->userServer->clientSocket, message, sizeof(message), 0);
        if(bytes_received <= 0)
            break;
        if (message[0] == '1')
        {
            multiPrint("change name");
            changeName(user);
            continue;
        }
        else if (message[0] == '2')
        {
            multiPrint("show friends list");
            showFriendsList(user);
            continue;
        }
        else if (message[0] == '3')
        {
            openBlocklist(user);
            continue;
        }
        else if (message[0] == '4')
        {
            showGroupsList(user);
            continue;
        }
        else if (string(message) == "#back"){
            break;
        }
        else
            sendMessage(user->userServer->clientSocket, "> Error: None of the above were chosen.\n");
        
    }
}

void Server::openBlocklist(User* user)
{
    sendMessage(user->userServer->clientSocket, "> Block list");
    showBlockList(user);
    multiPrint("show block list");
    char msg[this->MAXLEN];
    int bytes_received;
    while (true)
    {
        sendMessage(user->userServer->clientSocket, "#loc> Menu|Blocklist| ");
        sendMessage(user->userServer->clientSocket, "  [1] Block\n  [2] Unblock\n  [3] Exit\n");
        bytes_received = recv(user->userServer->clientSocket, msg, sizeof(msg), 0);
        if(bytes_received <= 0)
            break;
        if (msg[0] == '1')
        {
            sendMessage(user->userServer->clientSocket, "> Enter the username you want to block:");
            bytes_received = recv(user->userServer->clientSocket, msg, sizeof(msg), 0);
            userDB->addToBlocklist(user, users[msg]);
            string result = "> [" + string(msg) + "] has been blocked!";
            sendMessage(user->userServer->clientSocket, result);
            continue;
        }
        else if (msg[0] == '2')
        {
            sendMessage(user->userServer->clientSocket, "> Enter the username you want to unblock:");
            bytes_received = recv(user->userServer->clientSocket, msg, sizeof(msg), 0);
            userDB->removeFromBlocklist(user, users[msg]);
            string result = "> [" + string(msg) + "] has been unblocked!";
            sendMessage(user->userServer->clientSocket, result);
            showBlockList(user);
            continue;
        }
        else if (msg[0] == '3')
            return;
        else
            sendMessage(user->userServer->clientSocket, "> Error: None of the above were chosen.\n");

    }
}

void Server::openPrivateChatSession(User* sender, User* receiver)
{
    char msg[this->MAXLEN];
    int bytes_received;
    while (true)
    {
        sendMessage(sender->userServer->clientSocket, "#loc> PV|"+ receiver->username + "| ");
        sendMessage(sender->userServer->clientSocket, "");
        bytes_received = recv(sender->userServer->clientSocket, msg, sizeof(msg), 0);
        if(bytes_received <= 0)
            break;
        if (string(msg) == "#back"){
            return;
        }
        if (string(msg) == "#history"){
            showChatHistory(sender, receiver->username, true);
            continue;
        }
        else
            sendPv(sender, receiver, msg);
    }
}

void Server::openGroupChatSession(User* sender, string groupName)
{
    char msg[this->MAXLEN];
    int bytes_received;
    while (true)
    {
        sendMessage(sender->userServer->clientSocket, "#loc> GP|"+ groupName + "| ");
        sendMessage(sender->userServer->clientSocket, "");
        bytes_received = recv(sender->userServer->clientSocket, msg, sizeof(msg), 0);
        if(bytes_received <= 0)
            break;
        if (string(msg) == "#back")
            return;
        else if (string(msg) == "#history"){
            showChatHistory(sender, groupName, false);
            continue;
        }
        else if (string(msg) == "#members"){
            showGroupMembers(sender, groupName, false);
            continue;
        }
        else if (string(msg) == "#leave"){
            userDB->removeUserFromGP(sender, groupName);
            sendMessage(sender->userServer->clientSocket, "> You've left group [" + groupName + "]");
            return;
        }
        else
            sendGp(sender, groupName, msg);
    }
}

void Server::checkGp(string user, string groupName)
{
    vector<string> groups = userDB->getGroups();
    bool flag = false;
    for (string gp : groups)
        if (groupName == gp){
            flag = true;
            break;
        }
    if (!flag)
        throw "This groups doesn't exist.";
    
    groups.clear();
    groups = userDB->getUserGroups(user);
    flag = false;
    for (string gp : groups)
        if (groupName == gp){
            flag = true;
            break;
        }
    if (!flag)
        throw "You are not a member of this group yet.";

}

void Server::handleUserReq(User* user, string message)
{
    vector<string>* message_splitted = 0;
    try
    {
        message_splitted = splitMessage(message, " #");
        checkMessageSize(message_splitted);
        if (message_splitted->at(0) == "menu")
        {
            checkMessageSize(message_splitted, 1);
            openMainMenu(user);
        }
        else if (message_splitted->at(0) == "pv")
        {
            checkMessageSize(message_splitted, 2);
            checkUser(message_splitted->at(1));
            checkIfIsBlocked(user, users[message_splitted->at(1)]->username);
            openPrivateChatSession(user, users[message_splitted->at(1)]);
            
        }
        else if (message_splitted->at(0) == "join")
        {
            checkMessageSize(message_splitted, 2);
            userDB->addUserToGroup(user, message_splitted->at(1));
        }
        else if (message_splitted->at(0) == "gp")
        {
            checkMessageSize(message_splitted, 2);
            checkGp(user->username, message_splitted->at(1));
            openGroupChatSession(user, message_splitted->at(1));
        }
        else
            throw "The command is not executable";
    }
    catch(const char* msg)
    {   
        sendMessage(user->userServer->clientSocket, "> There is a problem (" + string(msg) + ")");
    }
    if (message_splitted)
    {
        message_splitted->clear();
        delete message_splitted;
    }
}

void Server::showFriendsList(User* user)
{
    vector<string> friends = userDB->getUserFriends(user);
    string clientFriends = "";
    for (string f : friends)
        clientFriends += f + "\n";
    sendMessage(user->userServer->clientSocket, clientFriends);
    return;
}

void Server::showGroupsList(User* user)
{
    vector<string> groups = userDB->getUserGroups(user->username);
    string userGroups = "";
    for (string g : groups)
        userGroups += g + "\n";
    sendMessage(user->userServer->clientSocket, userGroups);
}

void Server::showBlockList(User* user)
{
    vector<string> block = userDB->getUserBlocklist(user);
    string blockedUsers = "";
    for (string b : block)
        blockedUsers += b + "\n";
    sendMessage(user->userServer->clientSocket, blockedUsers);
}

void Server::showChatHistory(User* user, string userOrGp, bool pv)
{
    string his = userDB->getOnlineHistory(userOrGp, pv, user->username);
    sendMessage(user->userServer->clientSocket, his);
}

void Server::showOfflineHistory(string username, UserServer* userServer)
{
    string his = userDB->getOfflineHistory(username);	
    sendMessage(userServer->clientSocket, his);
}

void Server::showGroupMembers(User* user, string groupName, bool pv)
{
    vector<string> gpmem = userDB->getGroupMembers(groupName);
    string members;
    for (string m : gpmem)
        members += m + "\n";
    members += "* " + to_string(gpmem.size()) + " members.";
    sendMessage(user->userServer->clientSocket, members);
}

void Server::getUsersFromDB()
{
    vector<User*>* users_db = userDB->getUsers();
    for (auto &user:*users_db)
        users[user->username] = user;
}

void Server::deleteUsers()
{
    for (auto & u : clients)
    {
        delete u.second;
    }
    clients.clear();
    for (auto & u : users)
    {
        delete u.second;
    }
    users.clear();
}

Server::~Server()
{
    delete userDB;
    deleteUsers();
    closeConnection();
}