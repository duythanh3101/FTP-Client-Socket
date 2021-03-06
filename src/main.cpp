// FTPClient_Test1.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "FTPClient.h"
#include "InputHandler.h"
#pragma comment(lib,"Ws2_32.lib")


int main()
{	
	FTPClient ftpClient;
	string command;
	vector<string> lstCommand;
	cout << "Usage: ftp <host>" << endl;

	while (true)
	{
		cout << "ftp> ";
		getline(cin, command);
		lstCommand = getCommand(command);
		if (lstCommand.size() == 0)
		{
			continue;
		}

		if (lstCommand.size() == 2 
			&& 	toLower(lstCommand[0]) == "ftp" && (toLower(lstCommand[1]) == "localhost" 
			|| toLower(lstCommand[1]) == "127.0.0.1"))
		{		
			ftpClient.ConnectToServer(21, lstCommand[1]);
			ftpClient.Login();
			continue;
		}	

		ftpClient.ExecuteFTPCommand(lstCommand);
	}
	
    return 0;
}
