#pragma once
#include "stdafx.h"
#include "FTPClient.h"
#include "InputHandler.h"

#define LENGTH 1000

enum MODE
{
	ACTIVE,
	PASSIVE
};

class FTPClient
{
private:
	WSADATA wsaData;
	SOCKADDR_IN serverAddress;			// Dia chi ip cua server
	SOCKET controlSocket;				
	SOCKET dataSocket;

	char *sendBuffer = new char[LENGTH];
	char *recvBuffer = new char[LENGTH];
	string hostIP;
	string userName;
	string password;
	bool isConnected;
	MODE mode;
public:
	FTPClient();
	~FTPClient();

	int sendCommand(string cmd);
	int recvData();

	int getPort();
	int getCode();

	bool ConnectToServer(int portServer, string ipAddress);
	void DisconnectServer();

	bool Login();						// done
	bool user(string userName);			// done
	bool user(vector<string> cmd);		// done

	bool dir();
	bool ls();

	bool cd(vector<string> cmd);		// done
	bool cd(string remote);				// done

	bool lcd(vector<string> cmd);		// done
	bool lcd(string remote);			// done

	bool pasv();						// done
	int actv();

	bool put(vector<string> cmd);		// done
	bool put(string, string);			// done

	bool mput(vector<string> cmd);

	bool get(vector<string> cmd);		// done
	bool get(string remoteFile, string localFile); // done

	bool mget(vector<string> cmd);

	void dele(vector<string> cmd);		// done
	void dele(string remoteFile);		// done

	void mdele(vector<string> cmd);		// done

	void rmdir(vector<string> cmd);		// done
	void rmdir(string folderName);		// done

	void mkdir(string folderName);		// done
	void mkdir(vector<string> cmd);		// done

	void pwd();							// done

	bool quit();						// done

	void ExecuteFTPCommand(vector<string> lstCommand);
	SOCKET ListeningSocket();
	string getCurrentDir();

};

