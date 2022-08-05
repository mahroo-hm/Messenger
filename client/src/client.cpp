#include "client.hpp"

Client::Client()
{
    MAXLEN = 150;
	loggedIn = false;
    name = new char[MAXLEN];
	username = new char[MAXLEN];
	exited = false;
	password = new char[MAXLEN];
}

void Client::startConnecting()
{
	if ((clientSocket=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket: ");
		exit(-1);
	}

    struct sockaddr_in client;
	client.sin_family=AF_INET;
	client.sin_port=htons(10000);
	client.sin_addr.s_addr=INADDR_ANY;
    if ((connect(clientSocket, (struct sockaddr *)&client, sizeof(struct sockaddr_in))) == -1)
	{
		perror("connect: ");
		exit(-1);
	}
}

void Client::login()
{
	while (true)
	{
		char* signMode = new char[MAXLEN];
		cout << "> Hello!\n  [1] Sign in\n  [2] New? Create an account\n";
		cin.getline(signMode, MAXLEN);
		send(clientSocket, signMode, sizeof(signMode), 0);
		if (signMode[0] == '2')
		{
			cout << "> Name : ";
			cin.getline(name, MAXLEN);
			send(clientSocket, name, sizeof(name), 0);	
		}
		if (signMode[0] == '2' || signMode[0] == '1')
		{
			cout << "> Username : ";
			cin.getline(username, MAXLEN);
			send(clientSocket, username, sizeof(username), 0);

			cout << "> Password : ";
			cin.getline(password, MAXLEN);
			send(clientSocket, password, sizeof(password), 0);
		if (signMode[0] == '2')
		{
			cout << "> Rapeat password : ";
			cin.getline(password, MAXLEN);
			send(clientSocket, password, sizeof(password), 0);
		}

			char answer[MAXLEN];
			int bytes_received = recv(clientSocket, answer, sizeof(answer), 0);
			if(bytes_received <= 0)
				continue;		
			multiPrint(answer, false);
			if (string(answer) == "> Welcome " + string(username) + "! Thank you for joining us!" || string(answer) == "> Welcome Back " + string(username) + "!")
				break;
		}
		else
			cout << "> Error: None of the above were chosen.\n" << endl;
	}
}

void Client::startCommunicating()
{
	login();
    sendThread = new thread(sendHandler, this);
    recvThread = new thread(recvHandler, this);	
	if (sendThread->joinable())
		sendThread->join();
	if (recvThread->joinable())
		recvThread->join();
	
}

void Client::multiPrint(string message, bool you)
{
	lock_guard<mutex> guard(printMTX);
	if (message.length())
		cout << "\33[2K \r" << message<<endl;
	if (you){
		cout << "\33[2K \r" << location;
	}
}

void Client::sendHandler(Client* client)
{
	while(true)
	{
		client->multiPrint("");
		char str[client->MAXLEN];
		cin.getline(str,client->MAXLEN);
		if (string(str) == "#exit")
		{
			client->exited = true;
			client->recvThread->detach();
			close(client->clientSocket);
			return;
		}
		send(client->clientSocket, str, sizeof(str),0);
	}	
}

void Client::recvHandler(Client* client)
{
	while(!client->exited)
	{
		char message[client->MAXLEN];
		int bytes_received = recv(client->clientSocket, message, sizeof(message), 0);
		if(bytes_received <= 0)
			continue;
		if (string(message).substr(0, 4) == "#loc"){
			client->location = string(message).substr(4);
		}
		else{
			client->multiPrint(string(message));
		}
		fflush(stdout);
	}  
}

void Client::closeConnection()
{
	if (sendThread)
	{
		if (sendThread->joinable())
		{
			sendThread->detach();
			delete sendThread;
		}
		sendThread = 0;
	}
	if (recvThread)
	{
		if (recvThread->joinable())
		{
			recvThread->detach();
			delete recvThread;
		}
		recvThread = 0;
	}
    close(clientSocket);
	multiPrint("*** The socket has been turned off ***", false);
}

Client::~Client()
{
	closeConnection();
	delete [] name;
	delete [] username;
	delete [] password;
}